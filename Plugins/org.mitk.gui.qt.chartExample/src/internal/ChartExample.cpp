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
#include <berryIQtStyleManager.h>
#include <berryWorkbenchPlugin.h>

// Qmitk
#include "ChartExample.h"

// Qt
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
  CreateConnectionsForGUIElements();
  connect(m_Controls.m_comboBoxChartType, &QComboBox::currentTextChanged, this, &ChartExample::AdaptDataGUI);

  m_Controls.m_groupBoxErrors->setVisible(false);
  m_Controls.m_groupBoxXErrors->setVisible(false);
  m_Controls.m_groupBoxYErrors->setVisible(false);
  m_Controls.m_lineEditDataXVector->setVisible(false);
  m_Controls.m_lineEditDataXVector->setText("0;1;2;3;4;5;6;7;8;9");
  ResetDataGUI();

  m_Controls.m_doubleSpinBox_maxZoomX->setValue(10);
  m_Controls.m_doubleSpinBox_maxZoomY->setValue(10);

  FillRandomDataValues();
  auto chartStyle = GetColorTheme();
  m_Controls.m_Chart->SetTheme(chartStyle);

  m_Controls.m_lineEditXAxisLabel->setText("xLabel");
  m_Controls.m_lineEditYAxisLabel->setText("yLabel");

  m_ChartNameToChartType.emplace("bar", QmitkChartWidget::ChartType::bar);
  m_ChartNameToChartType.emplace("line", QmitkChartWidget::ChartType::line);
  m_ChartNameToChartType.emplace("spline", QmitkChartWidget::ChartType::spline);
  m_ChartNameToChartType.emplace("pie", QmitkChartWidget::ChartType::pie);
  m_ChartNameToChartType.emplace("area", QmitkChartWidget::ChartType::area);
  m_ChartNameToChartType.emplace("area-spline", QmitkChartWidget::ChartType::area_spline);
  m_ChartNameToChartType.emplace("scatter", QmitkChartWidget::ChartType::scatter);

  m_LineNameToLineType.emplace("solid", QmitkChartWidget::LineStyle::solid);
  m_LineNameToLineType.emplace("dashed", QmitkChartWidget::LineStyle::dashed);

  m_AxisScaleNameToAxisScaleType.emplace("linear", QmitkChartWidget::AxisScale::linear);
  m_AxisScaleNameToAxisScaleType.emplace("logarithmic", QmitkChartWidget::AxisScale::log);

  m_LegendPositionNameToLegendPositionType.emplace("bottom middle", QmitkChartWidget::LegendPosition::bottomMiddle);
  m_LegendPositionNameToLegendPositionType.emplace("bottom right", QmitkChartWidget::LegendPosition::bottomRight);
  m_LegendPositionNameToLegendPositionType.emplace("top right", QmitkChartWidget::LegendPosition::topRight);
  m_LegendPositionNameToLegendPositionType.emplace("top left", QmitkChartWidget::LegendPosition::topLeft);
  m_LegendPositionNameToLegendPositionType.emplace("middle right", QmitkChartWidget::LegendPosition::middleRight);
}

void ChartExample::CreateConnectionsForGUIElements()
{
  connect(m_Controls.m_buttonCreateChart, &QPushButton::clicked, this, &ChartExample::CreateChart);
  connect(m_Controls.m_buttonUpdateChart, &QPushButton::clicked, this, &ChartExample::UpdateChart);
  connect(m_Controls.m_buttonClearChart, &QPushButton::clicked, this, &ChartExample::ClearChart);
  connect(m_Controls.m_buttonAddData, &QPushButton::clicked, this, &ChartExample::AddData);
  connect(m_Controls.m_checkBoxEnableDataX, &QCheckBox::toggled, this, &ChartExample::ShowXData);
  connect(m_Controls.m_checkBoxEnableErrors, &QCheckBox::toggled, this, &ChartExample::ShowErrorOptions);
  connect(m_Controls.m_checkBoxEnableXErrors, &QCheckBox::toggled, this, &ChartExample::ShowXErrorOptions);
  connect(m_Controls.m_checkBoxEnableYErrors, &QCheckBox::toggled, this, &ChartExample::ShowYErrorOptions);
  connect(m_Controls.m_doubleSpinBox_minZoomX, &QSpinBox::editingFinished, this, &ChartExample::AdaptZoomX);
  connect(m_Controls.m_doubleSpinBox_maxZoomX, &QSpinBox::editingFinished, this, &ChartExample::AdaptZoomX);
  connect(m_Controls.m_doubleSpinBox_minZoomY, &QSpinBox::editingFinished, this, &ChartExample::AdaptZoomY);
  connect(m_Controls.m_doubleSpinBox_maxZoomY, &QSpinBox::editingFinished, this, &ChartExample::AdaptZoomY);
  connect(m_Controls.m_comboBoxLegendPosition, &QComboBox::currentTextChanged, this, &ChartExample::OnLegendPositionChanged);
  connect(m_Controls.m_lineEditTitle, &QLineEdit::editingFinished, this, &ChartExample::OnTitleChanged);
  connect(m_Controls.m_lineEditXAxisLabel, &QLineEdit::editingFinished, this, &ChartExample::OnXAxisLabelChanged);
  connect(m_Controls.m_lineEditYAxisLabel, &QLineEdit::editingFinished, this, &ChartExample::OnYAxisLabelChanged);
  connect(
    m_Controls.m_comboBoxYAxisScale, &QComboBox::currentTextChanged, this, &ChartExample::OnYAxisScaleChanged);
  connect(m_Controls.m_checkBoxShowLegend, &QCheckBox::stateChanged, this, &ChartExample::OnShowLegendChanged);
  connect(m_Controls.m_checkBoxStackedData, &QCheckBox::stateChanged, this, &ChartExample::OnStackedDataChanged);
  connect(m_Controls.m_checkBoxShowDataPoints, &QCheckBox::stateChanged, this, &ChartExample::OnShowDataPointsChanged);
  connect(m_Controls.m_checkBoxShowSubchart, &QCheckBox::stateChanged, this, &ChartExample::OnShowSubchartChanged);
}

void ChartExample::FillRandomDataValues()
{
  std::vector<double> numbers = GenerateRandomNumbers(10, 10.0);
  std::string text = ConvertToText(numbers);
  m_Controls.m_lineEditDataYVector->setText(QString::fromStdString(text));

  m_Controls.m_lineEditDataLabel->setText("test" + QString::number(countForUID));

  numbers = GenerateRandomNumbers(10, 10.0);
  text = ConvertToText(numbers);
  m_Controls.m_lineEditXErrorPlus->setText(QString::fromStdString(text));
  numbers = GenerateRandomNumbers(10, 10.0);
  text = ConvertToText(numbers);
  m_Controls.m_lineEditXErrorMinus->setText(QString::fromStdString(text));
  numbers = GenerateRandomNumbers(10, 10.0);
  text = ConvertToText(numbers);
  m_Controls.m_lineEditYErrorPlus->setText(QString::fromStdString(text));
  numbers = GenerateRandomNumbers(10, 10.0);
  text = ConvertToText(numbers);
  m_Controls.m_lineEditYErrorMinus->setText(QString::fromStdString(text));

  countForUID++;
}

void ChartExample::CreateChart()
{
  auto dataYAxisScaleType =
    m_AxisScaleNameToAxisScaleType.at(m_Controls.m_comboBoxYAxisScale->currentText().toStdString());
  auto xAxisLabel = m_Controls.m_lineEditXAxisLabel->text().toStdString();
  auto yAxisLabel = m_Controls.m_lineEditYAxisLabel->text().toStdString();
  auto showLegend = m_Controls.m_checkBoxShowLegend->isChecked();
  auto legendPosition =
    m_LegendPositionNameToLegendPositionType.at(m_Controls.m_comboBoxLegendPosition->currentText().toStdString());
  auto showDataPoints = m_Controls.m_checkBoxShowDataPoints->isChecked();
  auto stackedData = m_Controls.m_checkBoxStackedData->isChecked();
  auto showSubchart = m_Controls.m_checkBoxShowSubchart->isChecked();
  auto title = m_Controls.m_lineEditTitle->text().toStdString();

  m_Controls.m_Chart->SetTitle(title);
  m_Controls.m_Chart->SetYAxisScale(dataYAxisScaleType);
  m_Controls.m_Chart->SetXAxisLabel(xAxisLabel);
  m_Controls.m_Chart->SetYAxisLabel(yAxisLabel);
  m_Controls.m_Chart->SetShowLegend(showLegend);
  m_Controls.m_Chart->SetLegendPosition(legendPosition);
  m_Controls.m_Chart->SetShowErrorBars(true);
  m_Controls.m_Chart->SetShowDataPoints(showDataPoints);
  m_Controls.m_Chart->SetStackedData(stackedData);
  m_Controls.m_Chart->Show(showSubchart);
}

void ChartExample::UpdateChart() {
  // Test update mechanism
  m_Controls.m_Chart->SetLineStyle("test0", QmitkChartWidget::LineStyle::dashed);
  m_Controls.m_Chart->SetChartType("test0", QmitkChartWidget::ChartType::spline);
  m_Controls.m_Chart->SetColor("test0", "violet");
  m_Controls.m_Chart->UpdateData2D({{0, 1}, {0.1, 2}, {0.2, 3}, {8, -2} }, "test0");
  m_Controls.m_Chart->UpdateLabel("test0", "newLabel");
}

void ChartExample::ClearChart()
{
  m_Controls.m_Chart->Clear();
  m_Controls.m_plainTextEditDataView->clear();
}

std::vector<double> ChartExample::ConvertToDoubleVector(const QString &data, QChar delimiter) const
{
  std::vector<double> output;
  if (data.isEmpty())
  {
    return output;
  }

  for (const QString entry : data.split(delimiter))
  {
    output.push_back(entry.toDouble());
  }
  return output;
}

std::vector<std::string> ChartExample::ConvertToStringVector(const QString &data, QChar delimiter) const
{
  std::vector<std::string> output;
  if (data.isEmpty())
  {
    return output;
  }

  for (const QString entry : data.split(delimiter))
  {
    output.push_back(entry.toStdString());
  }
  return output;
}

void ChartExample::AddData()
{
  QString data = m_Controls.m_lineEditDataYVector->text();
  auto dataY = ConvertToDoubleVector(data);

  auto chartType = m_ChartNameToChartType.at(m_Controls.m_comboBoxChartType->currentText().toStdString());
  std::string dataLabel = m_Controls.m_lineEditDataLabel->text().toStdString();
  std::string dataColor = m_Controls.m_lineEditColor->text().toStdString();
  auto dataLineStyleType = m_LineNameToLineType.at(m_Controls.m_comboBoxLineStyle->currentText().toStdString());

  if (m_Controls.m_checkBoxEnableDataX->isChecked())
  {
    QString lineEditDataX = m_Controls.m_lineEditDataXVector->text();
    auto dataX = ConvertToDoubleVector(lineEditDataX);
    if (dataX.size() != dataY.size())
    {
      mitkThrow() << "data x and y size have to be equal";
    }
    auto dataXandY = CreateMap(dataX, dataY);
    data = QString::fromStdString(ConvertToText(dataXandY));

    m_Controls.m_Chart->AddData2D(dataXandY, dataLabel, chartType);
  }
  else
  {
    m_Controls.m_Chart->AddData1D(dataY, dataLabel, chartType);
  }
  if (!dataColor.empty())
  {
    m_Controls.m_Chart->SetColor(dataLabel, dataColor);
  }

  if (chartType == QmitkChartWidget::ChartType::pie)
  {
    QString pieLabelsData = m_Controls.m_lineEditPieDataLabel->text();
    if (!pieLabelsData.isEmpty())
    {
      auto pieLabels = ConvertToStringVector(pieLabelsData);
      m_Controls.m_Chart->SetPieLabels(pieLabels, dataLabel);
    }
  }

  if (m_Controls.m_checkBoxEnableErrors->isChecked())
  {
    if (m_Controls.m_checkBoxEnableXErrors->isChecked())
    {
      auto errorsPlus = ConvertToDoubleVector(m_Controls.m_lineEditXErrorPlus->text());
      auto errorsMinus = ConvertToDoubleVector(m_Controls.m_lineEditXErrorMinus->text());
      m_Controls.m_Chart->SetXErrorBars(m_Controls.m_lineEditDataLabel->text().toStdString(), errorsPlus, errorsMinus);
    }
    if (m_Controls.m_checkBoxEnableYErrors->isChecked())
    {
      auto errorsPlus = ConvertToDoubleVector(m_Controls.m_lineEditYErrorPlus->text());
      auto errorsMinus = ConvertToDoubleVector(m_Controls.m_lineEditYErrorMinus->text());
      m_Controls.m_Chart->SetYErrorBars(m_Controls.m_lineEditDataLabel->text().toStdString(), errorsPlus, errorsMinus);
    }
  }

  m_Controls.m_Chart->SetLineStyle(dataLabel, dataLineStyleType);

  QString dataOverview;
  dataOverview.append(m_Controls.m_lineEditDataLabel->text());
  dataOverview.append("(").append(m_Controls.m_comboBoxChartType->currentText());
  if (!dataColor.empty())
  {
    dataOverview.append(", ").append(dataColor.c_str());
  }
  dataOverview.append(", ").append(m_Controls.m_comboBoxLineStyle->currentText());
  dataOverview.append(")");
  dataOverview.append(":").append(data);

  m_Controls.m_plainTextEditDataView->appendPlainText(dataOverview);

  FillRandomDataValues();
}

void ChartExample::ShowXData(bool show)
{
  m_Controls.m_lineEditDataXVector->setVisible(show);
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

void ChartExample::AdaptZoomX()
{
  m_Controls.m_Chart->SetMinMaxValueXView(m_Controls.m_doubleSpinBox_minZoomX->value(),
                                             m_Controls.m_doubleSpinBox_maxZoomX->value());
  m_Controls.m_Chart->Show();
}

void ChartExample::AdaptZoomY()
{
  m_Controls.m_Chart->SetMinMaxValueYView(m_Controls.m_doubleSpinBox_minZoomY->value(),
                                             m_Controls.m_doubleSpinBox_maxZoomY->value());
  m_Controls.m_Chart->Show();
}

void ChartExample::AdaptDataGUI(const QString &chartType)
{
  ResetDataGUI();
  auto chartTypeEnum = m_ChartNameToChartType.at(chartType.toStdString());
  if (chartTypeEnum == QmitkChartWidget::ChartType::pie)
  {
    m_Controls.m_labelPieData->setVisible(true);
    m_Controls.m_lineEditPieDataLabel->setVisible(true);
    m_Controls.m_labelColor->setVisible(false);
    m_Controls.m_lineEditColor->setVisible(false);
  }
  else if (chartTypeEnum == QmitkChartWidget::ChartType::line || chartTypeEnum == QmitkChartWidget::ChartType::area ||
           chartTypeEnum == QmitkChartWidget::ChartType::area_spline ||
           chartTypeEnum == QmitkChartWidget::ChartType::spline)
  {
    m_Controls.m_labelLineStyle->setVisible(true);
    m_Controls.m_comboBoxLineStyle->setVisible(true);
  }
}

void ChartExample::ResetDataGUI() {
  m_Controls.m_labelPieData->setVisible(false);
  m_Controls.m_lineEditPieDataLabel->setVisible(false);
  m_Controls.m_labelColor->setVisible(true);
  m_Controls.m_lineEditColor->setVisible(true);
  m_Controls.m_labelLineStyle->setVisible(false);
  m_Controls.m_comboBoxLineStyle->setVisible(false);
}

std::vector<double> ChartExample::GenerateRandomNumbers(unsigned int amount, double max) const
{
  QRandomGenerator gen;
  gen.seed(time(NULL));

  std::vector<double> data;
  for (unsigned int i = 0; i < amount; i++)
  {
    data.push_back(gen.bounded(max));
  }

  return data;
}

std::map<double, double> ChartExample::CreateMap(std::vector<double> keys, std::vector<double> values) const
{
  std::map<double, double> aMap;
  std::transform(keys.begin(), keys.end(), values.begin(), std::inserter(aMap, aMap.end()), [](double a, double b) {
    return std::make_pair(a, b);
  });
  return aMap;
}

std::string ChartExample::ConvertToText(std::vector<double> numbers, std::string delimiter) const
{
  std::ostringstream oss;
  oss.precision(3);

  if (!numbers.empty())
  {
    for (auto number : numbers)
    {
      oss << number << delimiter;
    }
  }
  auto aString = oss.str();
  aString.pop_back();
  return aString;
}

std::string ChartExample::ConvertToText(std::map<double, double> numbers, std::string delimiter) const
{
  std::ostringstream oss;
  oss.precision(3);

  if (!numbers.empty())
  {
    for (const auto keyValue : numbers)
    {
      oss << keyValue.first << ":" << keyValue.second << delimiter;
    }
  }
  auto aString = oss.str();
  aString.pop_back();
  return aString;
}

QmitkChartWidget::ColorTheme ChartExample::GetColorTheme() const
{
  ctkPluginContext *context = berry::WorkbenchPlugin::GetDefault()->GetPluginContext();
  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
    if (styleManager->GetStyle().name == "Dark")
    {
      return QmitkChartWidget::ColorTheme::darkstyle;
    }
    else
    {
      return QmitkChartWidget::ColorTheme::lightstyle;
    }
  }
  return QmitkChartWidget::ColorTheme::darkstyle;
}

void ChartExample::OnLegendPositionChanged(const QString &newText)
{
  auto legendPosition = m_LegendPositionNameToLegendPositionType.at(newText.toStdString());
  m_Controls.m_Chart->SetLegendPosition(legendPosition);
}

void ChartExample::OnTitleChanged() {
  auto newTitle = m_Controls.m_lineEditTitle->text();
  m_Controls.m_Chart->SetTitle(newTitle.toStdString());
}

void ChartExample::OnXAxisLabelChanged() {
  auto newXAxisLabel = m_Controls.m_lineEditXAxisLabel->text();
  m_Controls.m_Chart->SetXAxisLabel(newXAxisLabel.toStdString());
}

void ChartExample::OnYAxisLabelChanged() {
  auto newYAxisLabel = m_Controls.m_lineEditYAxisLabel->text();
  m_Controls.m_Chart->SetYAxisLabel(newYAxisLabel.toStdString());
}

void ChartExample::OnYAxisScaleChanged(const QString &newYAxisScale) {
  auto yAxisScale = m_AxisScaleNameToAxisScaleType.at(newYAxisScale.toStdString());
  m_Controls.m_Chart->SetYAxisScale(yAxisScale);
}

void ChartExample::OnShowLegendChanged(int newState) {
  m_Controls.m_Chart->SetShowLegend(newState == Qt::Checked);
}

void ChartExample::OnStackedDataChanged(int newState) {
  m_Controls.m_Chart->SetStackedData(newState == Qt::Checked);
}

void ChartExample::OnShowDataPointsChanged(int newState) {
  m_Controls.m_Chart->SetShowDataPoints(newState == Qt::Checked);
}

void ChartExample::OnShowSubchartChanged(int newState) {
  m_Controls.m_Chart->SetShowSubchart(newState == Qt::Checked);
}
