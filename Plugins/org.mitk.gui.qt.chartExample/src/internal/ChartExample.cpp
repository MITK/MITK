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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryWorkbenchPlugin.h>
#include <berryIQtStyleManager.h>

// Qmitk
#include "ChartExample.h"

// Qt
#include <QMessageBox>
#include <QRandomGenerator>


const std::string ChartExample::VIEW_ID = "org.mitk.views.chartexample";

void ChartExample::SetFocus()
{
  m_Controls.m_buttonCreateChart->setFocus();
}

void ChartExample::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.m_buttonCreateChart, &QPushButton::clicked, this, &ChartExample::CreateChart);
  connect(m_Controls.m_buttonClearChart, &QPushButton::clicked, this, &ChartExample::ClearChart);
  connect(m_Controls.m_buttonAddData, &QPushButton::clicked, this, &ChartExample::AddData);
  connect(m_Controls.m_checkBoxEnableErrors, &QCheckBox::toggled, this, &ChartExample::ShowErrorOptions);
  connect(m_Controls.m_checkBoxEnableXErrors, &QCheckBox::toggled, this, &ChartExample::ShowXErrorOptions);
  connect(m_Controls.m_checkBoxEnableYErrors, &QCheckBox::toggled, this, &ChartExample::ShowYErrorOptions);

  m_Controls.m_groupBoxErrors->setVisible(false);
  m_Controls.m_groupBoxXErrors->setVisible(false);
  m_Controls.m_groupBoxYErrors->setVisible(false);

  FillRandomDataValues();
  auto chartStyle = GetColorTheme();
  m_Controls.m_Chart->SetTheme(chartStyle);

  m_Controls.m_lineEditXAxisLabel->setText("xLabel");
  m_Controls.m_lineEditYAxisLabel->setText("yLabel");

  m_chartNameToChartType.emplace("bar", QmitkChartWidget::ChartType::bar);
  m_chartNameToChartType.emplace("line", QmitkChartWidget::ChartType::line);
  m_chartNameToChartType.emplace("spline", QmitkChartWidget::ChartType::spline);
  m_chartNameToChartType.emplace("pie", QmitkChartWidget::ChartType::pie);
  m_chartNameToChartType.emplace("area", QmitkChartWidget::ChartType::area);
  m_chartNameToChartType.emplace("area-spline", QmitkChartWidget::ChartType::area_spline);
  m_chartNameToChartType.emplace("scatter", QmitkChartWidget::ChartType::scatter);

  m_LineNameToLineType.emplace("solid", QmitkChartWidget::LineStyle::solid);
  m_LineNameToLineType.emplace("dashed", QmitkChartWidget::LineStyle::dashed);

  m_AxisScaleNameToAxisScaleType.emplace("linear", QmitkChartWidget::AxisScale::linear);
  m_AxisScaleNameToAxisScaleType.emplace("logarithmic", QmitkChartWidget::AxisScale::log);
}

void ChartExample::FillRandomDataValues()
{
  std::vector<double> numbers = generateRandomNumbers(10, 10.0);
  std::string text = convertToText(numbers, ";");
  m_Controls.m_lineEditDataVector->setText(QString::fromStdString(text));

  m_Controls.m_lineEditDataLabel->setText("test" + QString::number(countForUID));

  numbers = generateRandomNumbers(10, 10.0);
  text = convertToText(numbers, ";");
  m_Controls.m_lineEditXErrorPlus->setText(QString::fromStdString(text));
  numbers = generateRandomNumbers(10, 10.0);
  text = convertToText(numbers, ";");
  m_Controls.m_lineEditXErrorMinus->setText(QString::fromStdString(text));
  numbers = generateRandomNumbers(10, 10.0);
  text = convertToText(numbers, ";");
  m_Controls.m_lineEditYErrorPlus->setText(QString::fromStdString(text));
  numbers = generateRandomNumbers(10, 10.0);
  text = convertToText(numbers, ";");
  m_Controls.m_lineEditYErrorMinus->setText(QString::fromStdString(text));

  countForUID++;
}

void ChartExample::CreateChart()
{
  auto dataYAxisScaleType = m_AxisScaleNameToAxisScaleType.at(m_Controls.m_comboBoxYAxisScale->currentText().toStdString());
  auto xAxisLabel = m_Controls.m_lineEditXAxisLabel->text().toStdString();
  auto yAxisLabel = m_Controls.m_lineEditYAxisLabel->text().toStdString();
  auto showLegend = m_Controls.m_checkBoxShowLegend->isChecked();
  auto showDataPoints = m_Controls.m_checkBoxShowDataPoints->isChecked();
  auto stackedData = m_Controls.m_checkBoxStackedData->isChecked();
  auto showSubchart = m_Controls.m_checkBoxShowSubchart->isChecked();
  auto title = m_Controls.title->text().toStdString();

  m_Controls.m_Chart->SetTitle(title);
  m_Controls.m_Chart->SetYAxisScale(dataYAxisScaleType);
  m_Controls.m_Chart->SetXAxisLabel(xAxisLabel);
  m_Controls.m_Chart->SetYAxisLabel(yAxisLabel);
  m_Controls.m_Chart->SetShowLegend(showLegend);
  m_Controls.m_Chart->SetShowErrorBars(true);
  m_Controls.m_Chart->SetShowDataPoints(showDataPoints);
  m_Controls.m_Chart->SetStackedData(stackedData);
  m_Controls.m_Chart->Show(showSubchart);
}


void ChartExample::ClearChart()
{
  m_Controls.m_Chart->Clear();
  m_Controls.m_plainTextEditDataView->clear();
}

std::vector<double> ChartExample::ConvertToVector(const QString& lineEditData)
{
  std::vector<double> data;
  if (lineEditData.isEmpty())
  {
    return data;
  }

  for(const QString entry : lineEditData.split(';'))
  {
    data.push_back(entry.toDouble());
  }
  return data;
}

void ChartExample::AddData()
{
  QString lineEditData = m_Controls.m_lineEditDataVector->text();
  auto data = ConvertToVector(lineEditData);

  auto chartType = m_chartNameToChartType.at(m_Controls.m_comboBoxChartType->currentText().toStdString());
  std::string dataLabel = m_Controls.m_lineEditDataLabel->text().toStdString();
  std::string dataColor = m_Controls.m_lineEditColor->text().toStdString();
  auto dataLineStyleType = m_LineNameToLineType.at(m_Controls.m_comboBoxLineStyle->currentText().toStdString());
  
  m_Controls.m_Chart->AddData1D(data, dataLabel, chartType);
  if (!dataColor.empty()) {
    m_Controls.m_Chart->SetColor(dataLabel, dataColor);
  }

  if (m_Controls.m_checkBoxEnableErrors->isChecked())
  {
    if (m_Controls.m_checkBoxEnableXErrors->isChecked())
    {
      auto errorsPlus = ConvertToVector(m_Controls.m_lineEditXErrorPlus->text());
      auto errorsMinus = ConvertToVector(m_Controls.m_lineEditXErrorMinus->text());
      m_Controls.m_Chart->SetXErrorBars(m_Controls.m_lineEditDataLabel->text().toStdString(), errorsPlus, errorsMinus);
    }
    if (m_Controls.m_checkBoxEnableYErrors->isChecked())
    {
      auto errorsPlus = ConvertToVector(m_Controls.m_lineEditYErrorPlus->text());
      auto errorsMinus = ConvertToVector(m_Controls.m_lineEditYErrorMinus->text());
      m_Controls.m_Chart->SetYErrorBars(m_Controls.m_lineEditDataLabel->text().toStdString(), errorsPlus, errorsMinus);
    }
  }

  m_Controls.m_Chart->SetLineStyle(dataLabel, dataLineStyleType);

  QString dataOverview;
  dataOverview.append(m_Controls.m_lineEditDataLabel->text());
  dataOverview.append("(").append(m_Controls.m_comboBoxChartType->currentText());
  if (!dataColor.empty()) {
    dataOverview.append(", ").append(dataColor.c_str());
  }
  dataOverview.append(", ").append(m_Controls.m_comboBoxLineStyle->currentText());
  dataOverview.append(")");
  dataOverview.append(":").append(lineEditData);

  m_Controls.m_plainTextEditDataView->appendPlainText(dataOverview);

  FillRandomDataValues();
}

void ChartExample::ShowErrorOptions(bool show)
{
  m_Controls.m_groupBoxErrors->setVisible(show);
}

void ChartExample::ShowXErrorOptions(bool show)
{
  m_Controls.m_groupBoxXErrors->setVisible(show);
}

void ChartExample::ShowYErrorOptions(bool show)
{
  m_Controls.m_groupBoxYErrors->setVisible(show);
}

std::vector<double> ChartExample::generateRandomNumbers(unsigned int amount, double max) const
{
  QRandomGenerator gen;
  gen.seed(time(NULL));

  std::vector<double> data;
  for (unsigned int i = 0; i < amount; i++) {
    data.push_back(gen.bounded(max));
  }

  return data;
}

std::string ChartExample::convertToText(std::vector<double> numbers, std::string delimiter) const
{
  std::ostringstream oss;
  oss.precision(3);

  if (!numbers.empty())
  {
    for (auto number : numbers) {
      oss << number << delimiter;
    }
  }
  auto aString = oss.str();
  aString.pop_back();
  return aString;
}

QmitkChartWidget::ColorTheme ChartExample::GetColorTheme() const
{
  ctkPluginContext* context = berry::WorkbenchPlugin::GetDefault()->GetPluginContext();
  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
    if (styleManager->GetStyle().name == "Dark") {
      return QmitkChartWidget::ColorTheme::darkstyle;
    }
    else {
      return QmitkChartWidget::ColorTheme::lightstyle;
    }
  }
  return QmitkChartWidget::ColorTheme::darkstyle;
}
