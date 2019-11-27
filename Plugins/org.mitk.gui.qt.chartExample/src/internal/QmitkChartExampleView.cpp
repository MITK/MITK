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
#include "QmitkChartExampleView.h"
#include <QmitkChartxyData.h>

const std::string ChartExampleView::VIEW_ID = "org.mitk.views.chartexample";

void ChartExampleView::SetFocus()
{
  m_Controls.m_buttonCreateChart->setFocus();
}

void ChartExampleView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  CreateConnectionsForGUIElements();
  connect(m_Controls.m_comboBoxChartType, &QComboBox::currentTextChanged, this, &ChartExampleView::AdaptDataGUI);

  m_Controls.m_lineEditDataXVector->setText("0;1;2;3;4;5;6;7;8;9");
  m_Controls.m_lineEditDataYVector->setText("0;1;2;3;4;5;6;7;8;9");
  m_Controls.m_lineEditDataLabel->setText("Test");
  m_Controls.m_lineEditXAxisLabel->setText("X-Axis");
  m_Controls.m_lineEditYAxisLabel->setText("Y-Axis");
  m_Controls.m_lineEditTitle->setText("Title");

  m_Controls.m_labelPieData->setVisible(false);
  m_Controls.m_lineEditPieDataLabel->setVisible(false);

  m_Controls.m_groupBoxErrors->setVisible(false);
  m_Controls.m_groupBoxXErrors->setVisible(false);
  m_Controls.m_groupBoxYErrors->setVisible(false);

  m_Controls.m_doubleSpinBox_maxZoomX->setValue(10);
  m_Controls.m_doubleSpinBox_maxZoomY->setValue(10);

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

void ChartExampleView::CreateConnectionsForGUIElements()
{
  connect(m_Controls.m_buttonCreateChart, &QPushButton::clicked, this, &ChartExampleView::CreateChart);
  connect(m_Controls.m_buttonUpdateData, &QPushButton::clicked, this, &ChartExampleView::UpdateData);
  connect(m_Controls.m_buttonClearChart, &QPushButton::clicked, this, &ChartExampleView::ClearChart);
  connect(m_Controls.m_buttonAddData, &QPushButton::clicked, this, &ChartExampleView::AddData);
  connect(m_Controls.m_comboBoxExistingData, &QComboBox::currentTextChanged, this, &ChartExampleView::UpdateSelectedData);
  connect(m_Controls.m_checkBoxEnableErrors, &QCheckBox::toggled, this, &ChartExampleView::ShowErrorOptions);
  connect(m_Controls.m_checkBoxEnableXErrors, &QCheckBox::toggled, this, &ChartExampleView::ShowXErrorOptions);
  connect(m_Controls.m_checkBoxEnableYErrors, &QCheckBox::toggled, this, &ChartExampleView::ShowYErrorOptions);
  connect(m_Controls.m_doubleSpinBox_minZoomX, &QSpinBox::editingFinished, this, &ChartExampleView::AdaptZoomX);
  connect(m_Controls.m_doubleSpinBox_maxZoomX, &QSpinBox::editingFinished, this, &ChartExampleView::AdaptZoomX);
  connect(m_Controls.m_doubleSpinBox_minZoomY, &QSpinBox::editingFinished, this, &ChartExampleView::AdaptZoomY);
  connect(m_Controls.m_doubleSpinBox_maxZoomY, &QSpinBox::editingFinished, this, &ChartExampleView::AdaptZoomY);
  connect(m_Controls.m_comboBoxLegendPosition, &QComboBox::currentTextChanged, this, &ChartExampleView::OnLegendPositionChanged);
  connect(m_Controls.m_lineEditTitle, &QLineEdit::editingFinished, this, &ChartExampleView::OnTitleChanged);
  connect(m_Controls.m_lineEditXAxisLabel, &QLineEdit::editingFinished, this, &ChartExampleView::OnXAxisLabelChanged);
  connect(m_Controls.m_lineEditYAxisLabel, &QLineEdit::editingFinished, this, &ChartExampleView::OnYAxisLabelChanged);
  connect(m_Controls.m_comboBoxYAxisScale, &QComboBox::currentTextChanged, this, &ChartExampleView::OnYAxisScaleChanged);
  connect(m_Controls.m_checkBoxShowLegend, &QCheckBox::stateChanged, this, &ChartExampleView::OnShowLegendChanged);
  connect(m_Controls.m_checkBoxStackedData, &QCheckBox::stateChanged, this, &ChartExampleView::OnStackedDataChanged);
  connect(m_Controls.m_checkBoxShowDataPoints, &QCheckBox::stateChanged, this, &ChartExampleView::OnShowDataPointsChanged);
  connect(m_Controls.m_checkBoxShowSubchart, &QCheckBox::stateChanged, this, &ChartExampleView::OnShowSubchartChanged);
}

void ChartExampleView::AddData()
{
    QString lineEditDataX = m_Controls.m_lineEditDataXVector->text();
    QString lineEditDataY = m_Controls.m_lineEditDataYVector->text();
    auto dataX = ConvertToDoubleVector(lineEditDataX);
    auto dataY = ConvertToDoubleVector(lineEditDataY);
    auto dataXandY = CreateMap(dataX, dataY);
    QString data = QString::fromStdString(ConvertToText(dataXandY));

    std::string dataLabel = m_Controls.m_lineEditDataLabel->text().toStdString();
    std::string chartTypeAsString = m_Controls.m_comboBoxChartType->currentText().toLower().toStdString();
    std::string chartColorAsString = m_Controls.m_comboBoxColor->currentText().toLower().toStdString();
    std::string chartStyleAsString = m_Controls.m_comboBoxLineStyle->currentText().toLower().toStdString();
    auto chartStyle = m_LineNameToLineType.at(chartStyleAsString);

    if (std::find(labelStorage.begin(), labelStorage.end(), dataLabel) != labelStorage.end())
    {
        m_Controls.m_labelInfo->setText("This data already exists");
        m_Controls.m_labelInfo->setStyleSheet("color: red;");
        return;
    }

    if (dataX.size() != dataY.size())
    {
        m_Controls.m_labelInfo->setText("Data x and y size have to be equal");
        m_Controls.m_labelInfo->setStyleSheet("color: red;");
        return;
    }

    QString pieLabelsData = m_Controls.m_lineEditPieDataLabel->text();

    if (chartTypeAsString == "Pie")
    {
        if (!pieLabelsData.isEmpty())
        {
            auto pieLabels = ConvertToStringVector(pieLabelsData);
            m_Controls.m_Chart->SetPieLabels(pieLabels, dataLabel);
        }
    }

    labelStorage.push_back(dataLabel);
    m_Controls.m_Chart->AddChartExampleData(dataXandY, dataLabel, chartTypeAsString, chartColorAsString, chartStyleAsString, pieLabelsData.toStdString());
    m_Controls.m_comboBoxExistingData->addItem(m_Controls.m_lineEditDataLabel->text());

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

    m_Controls.m_Chart->SetLineStyle(dataLabel, chartStyle);

    QString dataOverview;
    dataOverview.append(m_Controls.m_lineEditDataLabel->text());
    dataOverview.append("(").append(m_Controls.m_comboBoxChartType->currentText());

    dataOverview.append(", ").append(m_Controls.m_comboBoxLineStyle->currentText());
    dataOverview.append(")");
    dataOverview.append(":").append(data);

    m_Controls.m_plainTextEditDataView->appendPlainText(dataOverview);
}

void ChartExampleView::CreateChart()
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

void ChartExampleView::UpdateData()
{
    if (m_Controls.m_comboBoxExistingData->currentText().isEmpty())
    {
        m_Controls.m_labelInfo->setText("Please enter a valid Datalabel");
        m_Controls.m_labelInfo->setStyleSheet("color: red;");
        return;
    }

    if (m_Controls.m_lineEditDataLabel->text() != m_Controls.m_comboBoxExistingData->currentText())
    {
        return;
    }

    auto dataLabel = m_Controls.m_lineEditDataLabel->text().toStdString();
    m_Controls.m_Chart->RemoveData(dataLabel);

    auto it = std::find(labelStorage.begin(), labelStorage.end(), dataLabel);
    labelStorage.erase(it);

    auto index = m_Controls.m_comboBoxExistingData->findText(QString::fromStdString(dataLabel));
    m_Controls.m_comboBoxExistingData->removeItem(index);

    AddData();
}

void ChartExampleView::UpdateSelectedData()
{
    std::string label = m_Controls.m_comboBoxExistingData->currentText().toStdString();
    auto data = m_Controls.m_Chart->GetDataElementByLabel(label);

    if (data == nullptr)
    {
        return;
    }

    auto x = data->GetXData();
    auto y = data->GetYData();

    auto xVector = x.toVector().toStdVector();
    auto yVector = y.toVector().toStdVector();

    QString xString = QString::fromStdString(ConvertToText(xVector));
    QString yString = QString::fromStdString(ConvertToText(yVector));

    auto color = data->GetColor();
    auto type = data->GetChartType();
    auto style = data->GetLineStyle();

    int colorIndex = m_Controls.m_Chart->GetIndexByString(color.toString().toStdString());
    int typeIndex = m_Controls.m_Chart->GetIndexByString(type.toString().toStdString());
    int styleIndex = m_Controls.m_Chart->GetIndexByString(style.toString().toStdString());

    if (type.toString() == "Pie")
    {
        m_Controls.m_comboBoxLineStyle->setVisible(false);
        m_Controls.m_labelLineStyle->setVisible(false);
        m_Controls.m_lineEditPieDataLabel->setVisible(true);
        m_Controls.m_labelPieData->setVisible(true);

        auto pieLabels = data->GetPieLabels();

        auto pieLabelsVector = pieLabels.toVector().toStdVector();

        QString pieLabelsString = QString::fromStdString(ConvertToText(pieLabelsVector));

        m_Controls.m_lineEditPieDataLabel->setText(pieLabelsString);
    }

    else
    {
        m_Controls.m_lineEditPieDataLabel->setVisible(false);
        m_Controls.m_labelPieData->setVisible(false);
        m_Controls.m_comboBoxLineStyle->setVisible(true);
        m_Controls.m_labelLineStyle->setVisible(true);
        m_Controls.m_comboBoxLineStyle->setCurrentIndex(styleIndex);
    }

    m_Controls.m_lineEditDataXVector->setText(xString);
    m_Controls.m_lineEditDataYVector->setText(yString);
    m_Controls.m_lineEditDataLabel->setText(QString::fromStdString(label));
    m_Controls.m_comboBoxColor->setCurrentIndex(colorIndex);
    m_Controls.m_comboBoxChartType->setCurrentIndex(typeIndex);
}

void ChartExampleView::ClearChart()
{
  m_Controls.m_Chart->Clear();

  m_Controls.m_plainTextEditDataView->clear();

  m_Controls.m_comboBoxExistingData->clear();

  labelStorage.clear();
}

void ChartExampleView::ShowErrorOptions(bool show)
{
  m_Controls.m_groupBoxErrors->setVisible(show);
}

void ChartExampleView::ShowXErrorOptions(bool show)
{
  m_Controls.m_groupBoxXErrors->setVisible(show);
}

void ChartExampleView::ShowYErrorOptions(bool show)
{
  m_Controls.m_groupBoxYErrors->setVisible(show);
}

void ChartExampleView::AdaptZoomX()
{
  m_Controls.m_Chart->SetMinMaxValueXView(m_Controls.m_doubleSpinBox_minZoomX->value(),
                                             m_Controls.m_doubleSpinBox_maxZoomX->value());
  m_Controls.m_Chart->Show();
}

void ChartExampleView::AdaptZoomY()
{
  m_Controls.m_Chart->SetMinMaxValueYView(m_Controls.m_doubleSpinBox_minZoomY->value(),
                                             m_Controls.m_doubleSpinBox_maxZoomY->value());
  m_Controls.m_Chart->Show();
}

void ChartExampleView::AdaptDataGUI(QString chartType)
{
  if (chartType == "Pie")
  {
    m_Controls.m_labelPieData->setVisible(true);
    m_Controls.m_lineEditPieDataLabel->setVisible(true);
    m_Controls.m_labelLineStyle->setVisible(false);
    m_Controls.m_comboBoxLineStyle->setVisible(false);
  }

  else
  {
    m_Controls.m_labelLineStyle->setVisible(true);
    m_Controls.m_comboBoxLineStyle->setVisible(true);
    m_Controls.m_labelPieData->setVisible(false);
    m_Controls.m_lineEditPieDataLabel->setVisible(false);
  }
}

void ChartExampleView::OnLegendPositionChanged(const QString &newText)
{
  auto legendPosition = m_LegendPositionNameToLegendPositionType.at(newText.toStdString());
  m_Controls.m_Chart->SetLegendPosition(legendPosition);
}

void ChartExampleView::OnTitleChanged() {
  auto newTitle = m_Controls.m_lineEditTitle->text();
  m_Controls.m_Chart->SetTitle(newTitle.toStdString());
}

void ChartExampleView::OnXAxisLabelChanged() {
  auto newXAxisLabel = m_Controls.m_lineEditXAxisLabel->text();
  m_Controls.m_Chart->SetXAxisLabel(newXAxisLabel.toStdString());
}

void ChartExampleView::OnYAxisLabelChanged() {
  auto newYAxisLabel = m_Controls.m_lineEditYAxisLabel->text();
  m_Controls.m_Chart->SetYAxisLabel(newYAxisLabel.toStdString());
}

void ChartExampleView::OnYAxisScaleChanged(const QString &newYAxisScale) {
  auto yAxisScale = m_AxisScaleNameToAxisScaleType.at(newYAxisScale.toStdString());
  m_Controls.m_Chart->SetYAxisScale(yAxisScale);
}

void ChartExampleView::OnShowLegendChanged(int newState) {
  m_Controls.m_Chart->SetShowLegend(newState == Qt::Checked);
}

void ChartExampleView::OnStackedDataChanged(int newState) {
  m_Controls.m_Chart->SetStackedData(newState == Qt::Checked);
}

void ChartExampleView::OnShowDataPointsChanged(int newState) {
  m_Controls.m_Chart->SetShowDataPoints(newState == Qt::Checked);
}

void ChartExampleView::OnShowSubchartChanged(int newState) {
  m_Controls.m_Chart->SetShowSubchart(newState == Qt::Checked);
}

std::map<double, double> ChartExampleView::CreateMap(std::vector<double> keys, std::vector<double> values) const
{
    std::map<double, double> aMap;
    std::transform(keys.begin(), keys.end(), values.begin(), std::inserter(aMap, aMap.end()), [](double a, double b) {
        return std::make_pair(a, b);
        });
    return aMap;
}

std::string ChartExampleView::ConvertToText(std::map<double, double> numbers, std::string delimiter) const
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

std::string ChartExampleView::ConvertToText(std::vector<QVariant> numbers, std::string delimiter) const
{
    std::ostringstream oss;
    oss.precision(3);

    if (!numbers.empty())
    {
        for (auto number : numbers)
        {
            oss << number.toDouble() << delimiter;
        }
    }
    auto aString = oss.str();
    aString.pop_back();
    return aString;
}

std::vector<double> ChartExampleView::ConvertToDoubleVector(const QString& data, QChar delimiter) const
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

std::vector<std::string> ChartExampleView::ConvertToStringVector(const QString& data, QChar delimiter) const
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
