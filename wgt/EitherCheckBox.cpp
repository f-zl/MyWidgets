#include "EitherCheckBox.hpp"
static QString TxtChecked(const QString &txtTrue, const QString &txtFalse) {
  return QStringLiteral(R"(<span style="font:bold">%1</span>/<span>%2</span>)")
      .arg(txtTrue)
      .arg(txtFalse);
}
static QString TxtNotChecked(const QString &txtTrue, const QString &txtFalse) {
  return QStringLiteral(R"(<span>%1</span>/<span style="font:bold">%2</span>)")
      .arg(txtTrue)
      .arg(txtFalse);
}
EitherCheckBox::EitherCheckBox(const QString &txtTrue, const QString &txtFalse,
                               bool initialValue, QWidget *parent)
    : QWidget{parent}, txtChecked{TxtChecked(txtTrue, txtFalse)},
      txtNotChecked{TxtNotChecked(txtTrue, txtFalse)} {
  lout.addWidget(&ckBox);
  lout.addWidget(&lb);
  // default non 0 margin makes this CheckBox not aligned with other QCheckBox
  // in a QVBoxLayout
  lout.setContentsMargins(0, 0, 0, 0);
  // set horizontal sizePolicy to be Fixed so that QLabel can be closely after
  // QCheckbox
  ckBox.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  ckBox.setChecked(initialValue);
  lb.setText(initialValue ? txtChecked : txtNotChecked);
  connect(&ckBox, &QCheckBox::toggled, [this](bool isChecked) {
    lb.setText(isChecked ? txtChecked : txtNotChecked);
  });
  connect(&lb, &ClickableLabel::clicked, &ckBox, &QCheckBox::toggle);
  // lb.setBuddy(&ckBox); doesn't work. click on label doesn't click ckBox
}
