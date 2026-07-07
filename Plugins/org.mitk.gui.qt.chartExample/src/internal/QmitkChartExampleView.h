/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkChartExampleView_h
#define QmitkChartExampleView_h

#include <QmitkAbstractView.h>
#include <QmitkChartWidget.h>

#include <memory>

namespace Ui
{
  class QmitkChartExampleViewControls;
}

/**
  \brief Basic example for use of module mitkChart

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkChartExampleView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkChartExampleView();
  ~QmitkChartExampleView() override;

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  void CreateConnectionsForGUIElements();

  virtual void SetFocus() override;

  void AddData();
  void CreateChart();
  void UpdateData();
  void UpdateSelectedData();
  void ClearChart();

private:
  std::map<std::string, QmitkChartWidget::AxisScale> m_AxisScaleNameToAxisScaleType;
  std::map<std::string, QmitkChartWidget::LegendPosition> m_LegendPositionNameToLegendPositionType;

  std::vector<std::string> labelStorage;

  void ShowErrorOptions(bool show);
  void ShowXErrorOptions(bool show);
  void ShowYErrorOptions(bool show);

  void AdaptZoomX();
  void AdaptZoomY();

  void AdaptDataGUI(QString chartType);

  void OnLegendPositionChanged(const QString &newPosition);
  void OnTitleChanged();
  void OnXAxisLabelChanged();
  void OnYAxisLabelChanged();
  void OnYAxisScaleChanged(const QString &newYAxisScale);
  void OnShowLegendChanged(Qt::CheckState newState);
  void OnStackedDataChanged(Qt::CheckState newState);
  void OnShowDataPointsChanged(Qt::CheckState newState);
  void OnShowSubchartChanged(Qt::CheckState newState);

  std::vector< std::pair<double, double> > CreatePairList(std::vector<double> keys, std::vector<double> values) const;
  std::string ConvertToText(std::vector<QVariant> numbers, std::string delimiter = ";") const;
  std::string ConvertToText(std::vector< std::pair<double, double> > numbers, std::string delimiter = ";") const;
  std::string ConvertToText(QVariantList list, std::string delimiter = ";") const;
  std::vector<double> ConvertToDoubleVector(const QString& data, QChar delimiter = ';') const;
  std::vector<std::string> ConvertToStringVector(const QString& data, QChar delimiter = ';') const;

  unsigned int countForUID = 0;
  std::unique_ptr<Ui::QmitkChartExampleViewControls> m_Controls;
};

#endif
