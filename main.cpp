#include "BoldCheckBox.hpp"
#include "EitherCheckBox.hpp"
#include "Plot/Plot.hpp"
#include <QApplication>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFormLayout>
#include <QTabWidget>
#include <QTimer>
#include <random>

using namespace Qt::StringLiterals;
struct Tab1 : QWidget {
  QFormLayout lout{this};
  BoldCheckBox bCk{u"txt"_s};
  EitherCheckBox eitherCk{u"true"_s, u"false"_s};
  Tab1() {
    lout.addRow(u"BoldCheckBox"_s, &bCk);
    lout.addRow(u"EitherCheckBox"_s, &eitherCk);
  }
};
struct RandomNumGen {
  std::random_device r;
  std::default_random_engine e1{r()};
  std::uniform_int_distribution<int> d;
  RandomNumGen(int min, int max) : d{min, max} {}
  int Next() { return d(e1); }
};
struct TabPlot : QWidget {
  QTimer timer;
  double xValue = 0.0;
  RandomNumGen randumNum{0, 100};

  QHBoxLayout lout{this};
  QVBoxLayout vLout;
  EitherCheckBox ckStartStop{u"Start"_s, u"Stop"_s};
  Plot plot{
      PlotCfg{.x{},
              .y{u"y1 unit"_s, {0.0, 200.0}},
              .y2{{u"%"_s, {-1.0, 101.0}}},
              .data{{u"y1"_s, false, Qt::black}, {u"y2"_s, true, Qt::red}}}};
  TabPlot() {
    vLout.addWidget(&ckStartStop);
    lout.addLayout(&vLout);
    lout.addWidget(&plot);
    timer.setInterval(100);
    connect(&timer, &QTimer::timeout, [this] { UpdatePlotData(); });
    connect(
        &ckStartStop.checkBox(), &QCheckBox::toggled,
        [this](bool isChecked) { isChecked ? timer.start() : timer.stop(); });
  }
  void UpdatePlotData() {
    xValue += 0.5; // should increase by 0.1 (second) to be consistent with
                   // timer's interval, use 0.5 here for better display
    double yValue[] = {randumNum.Next() * 2.0, double(randumNum.Next())};
    // added data should be consistent with PlotCfg
    plot.AddData(xValue, yValue);
    plot.Replot();
  }
};
struct Widget : QTabWidget {
  Tab1 tab1;
  TabPlot tabPlot;
  Widget() {
    for (auto &i : std::initializer_list<std::pair<QWidget *, QString>>{
             {&tab1, u"Tab1"_s}, {&tabPlot, u"Plot"_s}}) {
      addTab(i.first, i.second);
      i.first->setAutoFillBackground(true); // remove white background
    }
  }
};
static void SetStyleSheet(QApplication &a, const QString &path) {
  QFile f{path};
  bool ok = f.open(QIODeviceBase::ReadOnly);
  if (!ok) {
    qWarning("style sheet not found");
  } else {
    a.setStyleSheet(f.readAll());
  }
  // auto close f
}
int main(int argc, char **argv) {
  // edit the style sheet to see immediate effect
  // e.g. set background-color to display the size of a widget
  const QString styleSheetPath{
      u"../style.qss"_s}; // assumes run in <root>/build folder
  // using qrc disables the hot reload of style sheet
  QApplication a{argc, argv};
  SetStyleSheet(a, styleSheetPath);
  QFileSystemWatcher fw{{styleSheetPath}};
  QObject::connect(&fw, &QFileSystemWatcher::fileChanged,
                   [](const QString &path) { SetStyleSheet(*qApp, path); });
  Widget w;
  w.showMaximized();
  return a.exec();
}
