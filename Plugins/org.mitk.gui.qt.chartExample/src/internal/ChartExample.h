/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef ChartExample_h
#define ChartExample_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_ChartExampleControls.h"

/**
  \brief ChartExample

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class ChartExample : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  void CreateChart();
  void ClearChart();
  std::vector<double> ConvertToVector(const QString &lineEditData);
  void AddData();

  void ShowErrorOptions(bool show);
  void ShowXErrorOptions(bool show);
  void ShowYErrorOptions(bool show);

  Ui::ChartExampleControls m_Controls;

private:
  void FillRandomDataValues();
  std::vector<double> generateRandomNumbers(unsigned int amount, double max) const;
  std::string convertToText(std::vector<double> numbers, std::string delimiter) const;
  QmitkChartWidget::ColorTheme GetColorTheme() const;

  std::map<std::string, QmitkChartWidget::ChartType> m_chartNameToChartType;
  std::map<std::string, QmitkChartWidget::LineStyle> m_LineNameToLineType;
  std::map<std::string, QmitkChartWidget::AxisScale> m_AxisScaleNameToAxisScaleType;

  unsigned int countForUID = 0;
};

#endif // ChartExample_h
