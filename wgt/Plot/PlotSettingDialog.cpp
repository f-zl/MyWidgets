#include "PlotSettingDialog.hpp"
#include "qcustomplot.h"

PlotSettingDialog::PlotSettingDialog(QCustomPlot &plot, QWidget *parent)
    : QDialog{parent}, ref{plot} {
  layout.addWidget(&lb_xRange, 0, 1);
  layout.addWidget(&le_xMin, 0, 2);
  layout.addWidget(&le_xLen, 0, 3);
  layout.addWidget(&lb_yLeftRange, 1, 1);
  layout.addWidget(&le_yLeftMin, 1, 2);
  layout.addWidget(&le_yLeftMax, 1, 3);
  layout.addWidget(&ckBox_enYAxis2, 2, 0);
  layout.addWidget(&lb_yRightRange, 2, 1);
  layout.addWidget(&le_yRightMin, 2, 2);
  layout.addWidget(&le_yRightMax, 2, 3);
  layout.addWidget(&ckBox_enLegend, 3, 0);
  layout.addWidget(&ckBox_enScatter, 3, 1);
  layout.addWidget(&lb_msg, 4, 0, 1, 3);
  layout.addWidget(&ckBox_interactY1, 5, 0);
  layout.addWidget(&ckBox_interactY2, 5, 1);
  lb_msg.setStyleSheet("color:red");
  connect(&le_xMin, &QLineEdit::returnPressed, [this] {
    bool ok;
    double v = le_xMin.text().toDouble(&ok);
    if (ok) {
      auto xLen = ref.xAxis->range().size();
      ref.xAxis->setRange(v, v + xLen);
      ref.replot();
    } else {
      lb_msg.setText("Cannot convert data");
    }
  });
  connect(&le_xLen, &QLineEdit::returnPressed, [this] {
    bool ok;
    double v = le_xLen.text().toDouble(&ok);
    if (ok) {
      double upper = ref.xAxis->range().lower + v;
      ref.xAxis->setRangeUpper(upper);
      ref.replot();
    } else {
      lb_msg.setText("Cannot convert data");
    }
  });
  connect(&le_yLeftMin, &QLineEdit::returnPressed, [this] {
    bool ok;
    double v = le_yLeftMin.text().toDouble(&ok);
    if (ok) {
      ref.yAxis->setRangeLower(v);
      ref.replot();
    } else {
      lb_msg.setText("Cannot convert data");
    }
  });
  connect(&le_yLeftMax, &QLineEdit::returnPressed, [this] {
    bool ok;
    double v = le_yLeftMax.text().toDouble(&ok);
    if (ok) {
      ref.yAxis->setRangeUpper(v);
      ref.replot();
    } else {
      lb_msg.setText("Cannot convert data");
    }
  });
  connect(&le_yRightMin, &QLineEdit::returnPressed, [this] {
    bool ok;
    double v = le_yRightMin.text().toDouble(&ok);
    if (ok) {
      ref.yAxis2->setRangeLower(v);
      ref.replot();
    } else {
      lb_msg.setText("Cannot convert data");
    }
  });
  connect(&le_yRightMax, &QLineEdit::returnPressed, [this] {
    bool ok;
    double v = le_yRightMax.text().toDouble(&ok);
    if (ok) {
      ref.yAxis2->setRangeUpper(v);
      ref.replot();
    } else {
      lb_msg.setText("Cannot convert data");
    }
  });
  connect(&ckBox_enYAxis2, &QCheckBox::toggled, [this](bool checked) {
    ref.yAxis2->setVisible(checked);
    ref.replot();
  });
  connect(&ckBox_enLegend, &QCheckBox::toggled, [this](bool checked) {
    ref.legend->setVisible(checked);
    ref.replot();
  });
  connect(&ckBox_enScatter, &QCheckBox::toggled, [this](bool checked) {
    QCPScatterStyle style;
    if (checked)
      style.setShape(QCPScatterStyle::ssDisc);
    for (auto i = 0; i < ref.graphCount(); ++i) {
      ref.graph(i)->setScatterStyle(style);
    }
    ref.replot();
  });
  //   connect(&ck) TODO 根据ckBox_interactiveY1, Y2修改
  setWindowTitle("plot");
}

void PlotSettingDialog::show() {
  SyncPlotSetting();
  QDialog::show();
}

void PlotSettingDialog::SyncPlotSetting() {
  auto xrange = ref.xAxis->range();
  auto yrange = ref.yAxis->range();
  auto y2range = ref.yAxis2->range();
  le_xMin.setPlaceholderText(QString::number(xrange.lower));
  le_xLen.setPlaceholderText(QString::number(ref.xAxis->range().size()));
  le_yLeftMin.setPlaceholderText(QString::number(yrange.lower));
  le_yLeftMax.setPlaceholderText(QString::number(yrange.upper));
  ckBox_enYAxis2.setChecked(ref.yAxis2->visible());
  le_yRightMin.setPlaceholderText(QString::number(y2range.lower));
  le_yRightMax.setPlaceholderText(QString::number(y2range.upper));
  ckBox_enLegend.setChecked(ref.legend->visible());
  // TODO 根据plot的设置，设置ckBox_interactY1和Y2
}
// don't trigger ok button when return pressed on line edit
void PlotSettingDialog::keyPressEvent(QKeyEvent *event) {
  int key = event->key();
  if (key == Qt::Key_Return || key == Qt::Key_Enter)
    event->ignore();
  else
    QDialog::keyPressEvent(event);
}
