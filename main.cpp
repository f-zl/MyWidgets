#include "BoldCheckBox.hpp"
#include "EitherCheckBox.hpp"
#include <QApplication>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFormLayout>

using namespace Qt::StringLiterals;
struct Widget : QWidget {
  QFormLayout lout{this};
  BoldCheckBox bCk{u"txt"_s};
  EitherCheckBox eitherCk{u"true"_s, u"false"_s};
  Widget() {
    lout.addRow(u"BoldCheckBox"_s, &bCk);
    lout.addRow(u"EitherCheckBox"_s, &eitherCk);
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
  const QString styleSheetPath{u"../style.qss"_s}; // assumes run in <root>/build folder
  // using qrc disables the hot reload of style sheet
  QApplication a{argc, argv};
  SetStyleSheet(a, styleSheetPath);
  QFileSystemWatcher fw{{styleSheetPath}};
  QObject::connect(&fw, &QFileSystemWatcher::fileChanged,
                   [](const QString &path) { SetStyleSheet(*qApp, path); });
  Widget w;
  w.show();
  return a.exec();
}
