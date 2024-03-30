#pragma once

#include <QColor>
#include <QWidget>
#include <optional>
#include <span>

struct PlotCfg final {
  struct XAxisCfg {
    QString label;
    double min;
    double len;
  };
  struct YAxisCfg {
    QString label;
    std::pair<double, double> range;
  };
  struct DataCfg {
    QString name;
    bool yAxis2;
    QColor color;
  };
  std::optional<XAxisCfg> x;
  YAxisCfg y;
  std::optional<YAxisCfg> y2;
  std::vector<DataCfg> data;
};
// a plot that assumes all data shares a x-axis value (often being time)
struct Plot final : QWidget {
  Q_OBJECT
  struct Impl;
  std::unique_ptr<Impl> pImpl;
  PlotCfg cfg;
  Q_DISABLE_COPY_MOVE(Plot)

public:
  explicit Plot(PlotCfg &&cfg, QWidget *parent = nullptr);
  ~Plot();
  void LoadCfg(const PlotCfg &cfg);
  void AddData(double x, std::span<const double> y);
  void Replot();
  bool ImportFromFile(const QString &fileName);
signals:
  void ErrorOccurred(const QString &errMsg);
  void FileImported(const QString &fileName);
};
