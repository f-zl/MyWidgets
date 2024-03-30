#include "Plot.hpp"
#include "PlotSettingDialog.hpp"
#include "qcustomplot.h"
#include <QtConcurrent>

using namespace Qt::StringLiterals;

static void BoxLayoutAddContent(
    QBoxLayout &lout,
    std::initializer_list<std::variant<QLayout *, QWidget *, std::nullptr_t>>
        content) {
  for (auto c : content) {
    switch (c.index()) {
    case 0:
      lout.addLayout(std::get<0>(c));
      break;
    case 1:
      lout.addWidget(std::get<1>(c));
      break;
    case 2:
      lout.addStretch();
      break;
    }
  }
}

static QString GetTitleLine(const QCustomPlot &plot) {
  QString title;
  QTextStream s{&title};
  const auto graphNum = plot.graphCount();
  s << plot.xAxis->label(); // 第一列是x轴，没有设置时返回""
  for (auto i = 0; i < graphNum; i++) {
    s << ',' << plot.graph(i)->name();
  }
  return title;
}
static void ExportToStream(const QCustomPlot &plot, QTextStream &out) {
  const auto graphNum = plot.graphCount();
  out << GetTitleLine(plot) << '\n';

  auto dataSize = plot.graph(0)->data()->size();
  for (auto j = 0; j < dataSize; j++) {
    auto time = plot.graph(0)->data()->at(j)->mainKey();
    out << time;
    for (auto i = 0; i < graphNum; i++) {
      out << ',' << plot.graph(i)->data()->at(j)->mainValue();
    }
    out << '\n';
  }
}

static bool SaveToFile(const QCustomPlot &plot, const QString &fileName) {
  QSaveFile ofile{fileName};
  bool ok = ofile.open(QIODevice::WriteOnly | QIODevice::Text);
  if (ok) {
    QTextStream out{&ofile};
    ExportToStream(plot, out);
    ok = ofile.commit();
  }
  return ok;
}
static bool IsTitleLineOK(const QCustomPlot &plot, const QString &titleLine) {
  return GetTitleLine(plot) == titleLine;
}
static void ClearData(const QCustomPlot &plot) {
  auto count = plot.graphCount();
  for (int i = 0; i < count; ++i) {
    plot.graph(i)->data()->clear();
  }
}
using ErrString = QString;
static std::optional<ErrString> ImportFromFile(const QCustomPlot &plot,
                                               const QString &fileName) {
  QFile f{fileName};
  bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
  if (!ok)
    return QStringLiteral("Fail open file: %1").arg(f.errorString());
  QTextStream in{&f};
  QString line;
  in.readLineInto(&line);
  if (!IsTitleLineOK(plot, line))
    return QStringLiteral("Invalid title");
  ClearData(plot);
  QCPGraphData point;
  const auto graphNum = plot.graphCount();
  in.readLineInto(&line);
  const QChar comma{','};
  while (line.isEmpty() == false) {
    auto list = line.split(comma);
    point.key = list[0].toDouble();       // 第1列为共用的时间
    for (auto i = 0; i < graphNum; ++i) { // 第2~graphNum列为各个曲线的数据
      point.value = list[i + 1].toDouble();
      plot.graph(i)->data()->add(point);
    }
    in.readLineInto(&line);
  }
  return std::nullopt;
}
static QCPGraph *AddGraph(QCustomPlot &plot, const QString &name,
                          QCPAxis *yAxis, const QPen &pen = QPen{Qt::black}) {
  Q_ASSERT(yAxis == plot.yAxis || yAxis == plot.yAxis2);
  auto *g = plot.addGraph(plot.xAxis, yAxis);
  g->setName(name);
  g->setPen(pen);
  return g;
}
static QPen TogglePale(
    QPen pen) { // 条件：设置的QPen的alpha为255 (ToDo series有个opacity可以用)
  constexpr int PaleAlpha = 25;
  QColor clr = pen.color();
  int alpha = clr.alpha();
  if (alpha > PaleAlpha) { // 在PaleApha与255间切换，一般的red初始alpha=255
    alpha = PaleAlpha;
  } else {
    alpha = 255;
  }
  clr.setAlpha(alpha);
  pen.setColor(clr);
  return pen;
}
struct Plot::Impl {
  QCustomPlot plot;
  PlotSettingDialog dialog{plot};
  std::unique_ptr<QTimer> tmrPlay;
  double moveStep = 0.1;
  QVBoxLayout lout;
  QHBoxLayout hLoutCtrl;
  QLabel lbPlotMsg;
  QScrollBar scrollBar{Qt::Horizontal};
  QPushButton btnPlay{u"播放"_s};
  QLineEdit lePlaySpeed;
  QCheckBox ckBoxAutoXRange{u"横轴自动更新"_s};
  QPushButton btnSetting{u"图像设置"_s};
  QPushButton btnReset{u"图像重置"_s};
  QPushButton btnExport{u"数据导出"_s};
  QPushButton btnImport{u"数据导入"_s};
  QString exportImportFileDir;

  explicit Impl(Plot *parent) : lout{parent} {
    ckBoxAutoXRange.setChecked(true);
    QObject::connect(
        &plot, &QCustomPlot::legendClick,
        [this](QCPLegend *legend, QCPAbstractLegendItem *item,
               QMouseEvent *event) {
          (void)legend;
          if ((event->button() != Qt::RightButton) &&
              (item != nullptr)) { // item=null表示点击空白
            auto *legendItem = static_cast<QCPPlottableLegendItem *>(item);
            auto *g = static_cast<QCPGraph *>(legendItem->plottable());
            g->setVisible(
                !g->visible()); // 只设置图的visible，legendItem的visible不变
            g->setPen(TogglePale(g->pen())); // 图的pen改变后legendItem也会变
            // 如果legendItem设置visible=false，则文字也会消失，无法再选中
            // 可以把color设成Qt::transparent，但是这样就需要存储之前的颜色
            plot.replot();
          }
        });
    QObject::connect(&btnSetting, &QPushButton::clicked, [this] {
      dialog.show(); // TODO 已打开再点击，对话框不会移到前面来
    });
    QObject::connect(&btnReset, &QPushButton::clicked, [this] {
      ClearData(plot);
      plot.replot();
    });
    QObject::connect(&btnExport, &QPushButton::clicked, [this] {
      QString startPath =
          exportImportFileDir.isEmpty() ? u"."_s : exportImportFileDir;
      // file dialog必须在gui thread打开
      QString fileName = QFileDialog::getSaveFileName(
          nullptr, u"Export File"_s, startPath, u"CSV Files (*.csv)"_s);
      if (fileName.isEmpty() == false) { // 取消时fileName=""
        QFuture<bool> result = QtConcurrent::run([this, fileName]() -> bool {
          bool ok = SaveToFile(plot, fileName);
          exportImportFileDir = QDir{fileName}.path(); // 无论是否成功都切换路径
          return ok;
        });
        result.then([this](bool ok) {
          lbPlotMsg.setText(ok ? u"导出成功"_s : u"导出失败"_s);
        });
      }
    });
    QObject::connect(&btnImport, &QPushButton::clicked, [this, parent] {
      QString startPath =
          exportImportFileDir.isEmpty() ? u"."_s : exportImportFileDir;
      QString fileName = QFileDialog::getOpenFileName(
          nullptr, u"Import File"_s, startPath, u"CSV Files (*.csv)"_s);
      if (fileName.isEmpty() == false) {
        exportImportFileDir = QDir{fileName}.path();
        parent->ImportFromFile(fileName);
      }
    });
    scrollBar.setMaximum(10000); // min: 0
    scrollBar.setMinimumWidth(200);
    QObject::connect(&scrollBar, &QScrollBar::valueChanged, [this](int value) {
      auto xAxisRange =
          plot.xAxis->range(); // plot.graph(0)->getKeyRange(found);
      auto xAxisLen = xAxisRange.upper - xAxisRange.lower;
      bool found;
      auto xDataRange = plot.graph(0)->getKeyRange(found);
      // keeps xAxisRange length
      // 0~10000 map to xmin-1 ~ xmax+1
      double start = value / 10000.0 * (xDataRange.size() - xAxisLen + 2) +
                     xDataRange.lower - 1;
      plot.xAxis->setRange(start, start + xAxisLen);
      plot.replot();
    });
    connect(&btnPlay, &QPushButton::clicked, [this] {
      if (btnPlay.text() == u"播放"_s) {
        if (!tmrPlay) {
          tmrPlay = std::make_unique<QTimer>();
          tmrPlay->setInterval(50);
          connect(tmrPlay.get(), &QTimer::timeout, [this] {
            bool found;
            auto rangeMax = plot.graph(0)->getKeyRange(found).upper;
            if (plot.xAxis->range().upper < rangeMax) {
              plot.xAxis->moveRange(moveStep);
              plot.replot();
            } else {
              tmrPlay->stop();
              btnPlay.setText(u"Play"_s);
            }
          });
        }
        bool found;
        auto range = plot.graph(0)->getKeyRange(found);
        auto rangeMin = range.lower;
        auto rangeLen = range.upper - range.lower;
        if (plot.xAxis->range().upper < rangeMin) {
          plot.xAxis->setRange(rangeMin - rangeLen * 0.5,
                               rangeMin + rangeLen * 2);
        }
        tmrPlay->start();
        btnPlay.setText(u"暂停"_s);
      } else {
        tmrPlay->stop();
        btnPlay.setText(u"播放"_s);
      }
      // TODO: 状态机管理播放、拖动、自动更新
    });
    lePlaySpeed.setPlaceholderText(u"速度(默认0.1)"_s);
    lePlaySpeed.setMaximumWidth(100);
    connect(&lePlaySpeed, &QLineEdit::returnPressed,
            [this] { moveStep = lePlaySpeed.text().toDouble(); });
    QList<QCPAxis *> axes{plot.xAxis, plot.yAxis, plot.yAxis2};
    plot.axisRect()->setRangeZoomAxes(axes);
    plot.axisRect()->setRangeDragAxes(axes);
    plot.setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot.axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft |
                                                             Qt::AlignTop);
    BoxLayoutAddContent(hLoutCtrl, {&lbPlotMsg, nullptr, &scrollBar, &btnPlay,
                                    &lePlaySpeed, &ckBoxAutoXRange, &btnSetting,
                                    &btnReset, &btnExport, &btnImport});
    BoxLayoutAddContent(lout, {&plot, &hLoutCtrl});
  }
};
Plot::Plot(PlotCfg &&cfg, QWidget *parent)
    : QWidget{parent}, pImpl{std::make_unique<Impl>(this)},
      cfg{std::move(cfg)} {
  LoadCfg(this->cfg);
}
Plot::~Plot() {}
void Plot::AddData(double x, std::span<const double> y) {
  assert(y.size() == cfg.data.size());
  for (size_t i = 0; i < y.size(); ++i) {
    pImpl->plot.graph(i)->data()->add({x, y[i]});
  }
}
void Plot::Replot() {
  if (pImpl->ckBoxAutoXRange.isChecked()) {
    auto data = pImpl->plot.graph(0)->data();
    if (data->size() > 0) {
      const double size = pImpl->plot.xAxis->range().size();
      const double lastX = data->at(data->size() - 1)->key;
      pImpl->plot.xAxis->setRange(lastX - size * 0.9, lastX + size * 0.1);
    }
  }
  pImpl->plot.replot();
}
void Plot::LoadCfg(const PlotCfg &cfg) {
  QCustomPlot &plot = pImpl->plot;
  plot.clearGraphs();
  plot.legend->setVisible(true);
  if (cfg.x) {
    auto &x = cfg.x.value();
    plot.xAxis->setRange(x.min, x.min + x.len);
    plot.xAxis->setLabel(x.label);
  } else {
    plot.xAxis->setRange(0.0, 30.0);
    plot.xAxis->setLabel(u"time/sec"_s);
  }
  plot.yAxis->setRange(cfg.y.range.first, cfg.y.range.second);
  plot.yAxis->setLabel(cfg.y.label);
  if (cfg.y2) {
    auto &y2Cfg = cfg.y2.value();
    plot.yAxis2->setVisible(true);
    plot.yAxis2->setRange(y2Cfg.range.first, y2Cfg.range.second);
    plot.yAxis2->setLabel(y2Cfg.label);
  }
  for (auto &c : cfg.data) {
    AddGraph(plot, c.name, c.yAxis2 ? plot.yAxis2 : plot.yAxis, QPen{c.color});
  }
}
bool Plot::ImportFromFile(const QString &fileName) {
  auto errString = ::ImportFromFile(pImpl->plot, fileName);
  if (errString) {
    emit ErrorOccurred(errString.value());
  } else {
    Replot();
    emit FileImported(fileName);
  }
  return !errString.has_value();
}
