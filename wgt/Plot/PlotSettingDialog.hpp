#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
class QCustomPlot;
class PlotSettingDialog : public QDialog {
public:
  explicit PlotSettingDialog(QCustomPlot &plot, QWidget *parent = nullptr);
  void show();

private:
  QCustomPlot &ref;
  // 从上往下，从左往右
  QGridLayout layout{this};
  QLabel lb_xRange{"xmin, xlen"};
  QLineEdit le_xMin;
  QLineEdit le_xLen;
  QLabel lb_yLeftRange{"ymin, ymax"};
  QLineEdit le_yLeftMin;
  QLineEdit le_yLeftMax;
  QCheckBox ckBox_enYAxis2{"y2"};
  QLabel lb_yRightRange{"y2min, y2max"};
  QLineEdit le_yRightMin;
  QLineEdit le_yRightMax;
  QCheckBox ckBox_enLegend{"legend"};
  QCheckBox ckBox_enScatter{"scatter"};
  QCheckBox ckBox_interactY1{"interact y1"};
  QCheckBox ckBox_interactY2{"interact y2"};
  QLabel lb_msg;
  void SyncPlotSetting();

protected:
  void keyPressEvent(QKeyEvent *event) override;
};
