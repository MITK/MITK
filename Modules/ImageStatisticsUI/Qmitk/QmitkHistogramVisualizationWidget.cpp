/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkHistogramVisualizationWidget.h"

#include <QClipboard>

QmitkHistogramVisualizationWidget::QmitkHistogramVisualizationWidget(QWidget* parent) : QWidget(parent)
{
  m_Controls.setupUi(this);
  m_Controls.checkBoxShowSubchart->setChecked(false);
  m_Controls.spinBoxNBins->setValue(m_DefaultNBins);
  m_Controls.spinBoxNBins->setMinimum(m_MinNBins);
  m_Controls.spinBoxNBins->setMaximum(m_MaxNBins);
  m_Controls.chartWidget->SetXAxisLabel("Gray value");
  m_Controls.chartWidget->SetYAxisLabel("Frequency");
  m_Controls.chartWidget->SetShowLegend(false);

  SetGUIElementsEnabled(false);
  CreateConnections();
}

void QmitkHistogramVisualizationWidget::SetHistogram(itk::Statistics::Histogram<double>::ConstPointer histogram, const std::string& dataLabel)
{
  if (histogram == nullptr)
    return;

  bool histogramWasEmpty = m_Histograms.empty();

  if (m_Histograms.find(dataLabel) == m_Histograms.end())
  {
    m_Histograms.insert(std::make_pair(dataLabel, histogram));
    m_Controls.chartWidget->AddData2D(ConvertHistogramToPairList(histogram), dataLabel);
    m_Controls.chartWidget->SetChartType(dataLabel, QmitkChartWidget::ChartType::bar);
  }
  else
  {
    m_Histograms[dataLabel] = histogram;
    m_Controls.chartWidget->UpdateData2D(ConvertHistogramToPairList(histogram), dataLabel);
  }

  if (m_Histograms.empty() != histogramWasEmpty)
  {
    m_Controls.chartWidget->Show(m_Controls.checkBoxShowSubchart->isChecked());
    SetGUIElementsEnabled(!m_Histograms.empty());
  }
}

void QmitkHistogramVisualizationWidget::Reset()
{
  m_Controls.chartWidget->Clear();
  SetGUIElementsEnabled(false);
}

int QmitkHistogramVisualizationWidget::GetBins() {
  return m_Controls.spinBoxNBins->value();
}

void QmitkHistogramVisualizationWidget::ResetDefault()
{
  m_Controls.checkBoxUseDefaultNBins->setChecked(true);
  m_Controls.spinBoxNBins->setEnabled(false);
  m_Controls.spinBoxNBins->setValue(100);
  m_Controls.checkBoxShowSubchart->setChecked(false);
}

void QmitkHistogramVisualizationWidget::SetTheme(QmitkChartWidget::ColorTheme style)
{
  m_Controls.chartWidget->SetTheme(style);
}

void QmitkHistogramVisualizationWidget::CreateConnections()
{
  connect(m_Controls.buttonCopyHistogramToClipboard, &QPushButton::clicked, this, &QmitkHistogramVisualizationWidget::OnClipboardButtonClicked);
  connect(m_Controls.checkBoxUseDefaultNBins, &QCheckBox::clicked, this, &QmitkHistogramVisualizationWidget::OnDefaultNBinsCheckBoxChanged);
  connect(m_Controls.spinBoxNBins, &QSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnNBinsSpinBoxValueChanged);
  connect(m_Controls.checkBoxShowSubchart, &QCheckBox::clicked, this, &QmitkHistogramVisualizationWidget::OnShowSubchartCheckBoxChanged);
  connect(m_Controls.checkBoxViewMinMax, &QCheckBox::clicked, this, &QmitkHistogramVisualizationWidget::OnViewMinMaxCheckBoxChanged);
  connect(m_Controls.doubleSpinBoxMaxValue, &QSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnMaxValueSpinBoxValueChanged);
  connect(m_Controls.doubleSpinBoxMinValue, &QSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnMinValueSpinBoxValueChanged);
}

void QmitkHistogramVisualizationWidget::SetGUIElementsEnabled(bool enabled)
{
  this->setEnabled(enabled);
  m_Controls.tabWidgetPlot->setEnabled(enabled);
  m_Controls.checkBoxShowSubchart->setEnabled(enabled);
  m_Controls.checkBoxUseDefaultNBins->setEnabled(enabled);
  m_Controls.spinBoxNBins->setEnabled(!m_Controls.checkBoxUseDefaultNBins->isChecked());
  m_Controls.buttonCopyHistogramToClipboard->setEnabled(enabled);
  m_Controls.checkBoxViewMinMax->setEnabled(enabled);
  m_Controls.doubleSpinBoxMaxValue->setEnabled(m_Controls.checkBoxViewMinMax->isChecked());
  m_Controls.doubleSpinBoxMinValue->setEnabled(m_Controls.checkBoxViewMinMax->isChecked());
}

std::vector< std::pair<double, double> > QmitkHistogramVisualizationWidget::ConvertHistogramToPairList(itk::Statistics::Histogram<double>::ConstPointer histogram) const
{
  std::map<double, double> histogramMap;
  if (histogram)
  {
    auto endIt = histogram->End();
    auto it = histogram->Begin();

    // generating Lists of measurement and frequencies
    for (; it != endIt; ++it)
    {
      double frequency = it.GetFrequency();
      double measurement = it.GetMeasurementVector()[0];
      histogramMap.emplace(measurement, frequency);
    }

  }
  std::vector< std::pair<double, double> > histogram_list;
  for(auto iter = histogramMap.begin(); iter != histogramMap.end(); ++iter)
    histogram_list.emplace_back( iter->first, iter->second );

  return histogram_list;
}

void QmitkHistogramVisualizationWidget::OnClipboardButtonClicked()
{
  if (!m_Histograms.empty())
  {
    QString clipboard;
    for (const auto& histogram : m_Histograms)
    {
      clipboard.append(QString::fromStdString(histogram.first));
      clipboard.append("Measurement \t Frequency\n");
      auto iter = histogram.second->Begin();
      auto iterEnd = histogram.second->End();
      for (; iter != iterEnd; ++iter)
      {
        clipboard = clipboard.append("%L1 \t %L2\n")
            .arg(iter.GetMeasurementVector()[0], 0, 'f', 2)
            .arg(iter.GetFrequency());
      }
      clipboard.append("\n\n");
    }

    QApplication::clipboard()->clear();
    QApplication::clipboard()->setText(clipboard, QClipboard::Clipboard);
  }
}

void QmitkHistogramVisualizationWidget::OnDefaultNBinsCheckBoxChanged()
{
  if (m_Controls.checkBoxUseDefaultNBins->isChecked())
  {
    m_Controls.spinBoxNBins->setEnabled(false);
    if (m_Controls.spinBoxNBins->value() != static_cast<int>(m_DefaultNBins) ) {
      m_Controls.spinBoxNBins->setValue(m_DefaultNBins);
      OnNBinsSpinBoxValueChanged();
    }
  }
  else
  {
    m_Controls.spinBoxNBins->setEnabled(true);
  }
}

void QmitkHistogramVisualizationWidget::OnNBinsSpinBoxValueChanged()
{
  emit RequestHistogramUpdate(m_Controls.spinBoxNBins->value());
}

void QmitkHistogramVisualizationWidget::OnShowSubchartCheckBoxChanged()
{
  m_Controls.chartWidget->Show(m_Controls.checkBoxShowSubchart->isChecked());
}

void QmitkHistogramVisualizationWidget::OnViewMinMaxCheckBoxChanged()
{
  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::lowest();
  for (const auto& histogram : m_Histograms)
  {
    auto aMin = histogram.second->GetBinMin(0, 0);
    if (min > aMin) min = aMin;

    auto maxVector = histogram.second->GetDimensionMaxs(0);
    if (m_Controls.checkBoxUseDefaultNBins->isChecked())
    {
      max = std::max(max, maxVector[m_DefaultNBins - 1]);
    }
    else
    {
      max = std::max(max, maxVector[m_Controls.spinBoxNBins->value() - 1]);
    }
  }

  if (!m_Controls.checkBoxViewMinMax->isChecked())
  {
    m_Controls.doubleSpinBoxMaxValue->setEnabled(false);
    m_Controls.doubleSpinBoxMinValue->setEnabled(false);
    m_Controls.chartWidget->Reload();
  }
  else
  {
    m_Controls.doubleSpinBoxMinValue->setMinimum(min);
    m_Controls.doubleSpinBoxMinValue->setValue(min);
    m_Controls.doubleSpinBoxMaxValue->setMaximum(max);
    m_Controls.doubleSpinBoxMaxValue->setValue(max);
    m_Controls.doubleSpinBoxMaxValue->setEnabled(true);
    m_Controls.doubleSpinBoxMinValue->setEnabled(true);
  }
}

void QmitkHistogramVisualizationWidget::OnMinValueSpinBoxValueChanged()
{
  m_Controls.doubleSpinBoxMaxValue->setMinimum(m_Controls.doubleSpinBoxMinValue->value()+1);
  m_Controls.chartWidget->SetMinMaxValueXView(m_Controls.doubleSpinBoxMinValue->value(),m_Controls.doubleSpinBoxMaxValue->value());
  m_Controls.chartWidget->Show();
}

void QmitkHistogramVisualizationWidget::OnMaxValueSpinBoxValueChanged()
{
  m_Controls.doubleSpinBoxMinValue->setMaximum(m_Controls.doubleSpinBoxMaxValue->value()-1);
  m_Controls.chartWidget->SetMinMaxValueXView(m_Controls.doubleSpinBoxMinValue->value(),m_Controls.doubleSpinBoxMaxValue->value());
  m_Controls.chartWidget->Show();
}
