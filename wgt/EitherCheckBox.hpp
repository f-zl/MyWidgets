#pragma once
#include "ClickableLabel.hpp"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>

// Dispaly 2 options, the checked option is bold
class EitherCheckBox : public QWidget {
  // QLabel supports rich text. QCheckBox doesn't
  QHBoxLayout lout{this};
  QCheckBox ckBox;
  ClickableLabel lb;
  QString txtChecked;
  QString txtNotChecked;

public:
  EitherCheckBox(const QString &txtTrue, const QString &txtFalse,
                 bool initialValue = false, QWidget *parent = nullptr);
  bool isChecked() const { return ckBox.isChecked(); }
  QCheckBox &checkBox() { return ckBox; }
  QLabel &label() { return lb; }
};
