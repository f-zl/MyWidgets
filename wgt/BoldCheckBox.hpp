#pragma once
#include <QCheckBox>
// set bold when checked
struct BoldCheckBox : QCheckBox {
  BoldCheckBox(const QString &txt, QWidget *parent = nullptr)
      : QCheckBox{txt, parent} {
    connect(this, &QCheckBox::toggled, [this](bool isChecked) {
      auto ft = font();
      ft.setBold(isChecked);
      setFont(ft);
    });
  }
};
