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

#include "QmitkIntensityProfileVisualizationWidget.h"

#include <QClipboard>

QmitkIntensityProfileVisualizationWidget::QmitkIntensityProfileVisualizationWidget(QWidget* parent) : QWidget(parent)
{
	m_Controls.setupUi(this);
  m_Controls.checkBoxShowSubchart->setChecked(false);
	CreateConnections();
}



void QmitkIntensityProfileVisualizationWidget::SetIntensityProfile(mitk::IntensityProfile::ConstPointer intensityProfile, const std::string& dataLabel)
{
	if (intensityProfile == nullptr)
		return;

	m_IntensityProfileList = mitk::CreateVectorFromIntensityProfile(intensityProfile);
	if (m_IntensityProfileList.empty())
		return;

	m_Controls.chartWidget->AddData1D(m_IntensityProfileList, dataLabel);
	m_Controls.chartWidget->SetChartType(dataLabel, QmitkChartWidget::ChartType::line);
	m_Controls.chartWidget->SetXAxisLabel("Distance");
	m_Controls.chartWidget->SetYAxisLabel("Intensity");
  m_Controls.chartWidget->SetShowLegend(false);
  m_Controls.chartWidget->Show(m_Controls.checkBoxShowSubchart->isChecked());
	SetGUIElementsEnabled(true);
}

void QmitkIntensityProfileVisualizationWidget::Reset()
{
  m_Controls.chartWidget->Clear();
	SetGUIElementsEnabled(false);
	m_IntensityProfileList.clear();
}

void QmitkIntensityProfileVisualizationWidget::SetTheme(QmitkChartWidget::ColorTheme style)
{
  m_ChartStyle = style;
}

void QmitkIntensityProfileVisualizationWidget::CreateConnections()
{
	connect(m_Controls.checkBoxShowSubchart, &QCheckBox::clicked, this, &QmitkIntensityProfileVisualizationWidget::OnShowSubchartCheckBoxChanged);
	connect(m_Controls.buttonCopyToClipboard, &QPushButton::clicked, this, &QmitkIntensityProfileVisualizationWidget::OnClipboardButtonClicked);
  connect(m_Controls.chartWidget, &QmitkChartWidget::PageSuccessfullyLoaded, this, &QmitkIntensityProfileVisualizationWidget::OnPageSuccessfullyLoaded);
}

void QmitkIntensityProfileVisualizationWidget::SetGUIElementsEnabled(bool enabled)
{
	this->setEnabled(enabled);
	m_Controls.groupBoxIntensityProfile->setEnabled(enabled);
	m_Controls.groupBoxPlot->setEnabled(enabled);
	m_Controls.buttonCopyToClipboard->setEnabled(enabled);
	m_Controls.checkBoxShowSubchart->setEnabled(enabled);
	m_Controls.chartWidget->setEnabled(enabled);
}

std::vector<double> QmitkIntensityProfileVisualizationWidget::ConvertIntensityProfileToVector(mitk::IntensityProfile::ConstPointer intensityProfile) const
{
	std::vector<double> intensityProfileList;
	if (intensityProfile != nullptr)
	{
		auto end = intensityProfile->End();

		for (auto it = intensityProfile->Begin(); it != end; ++it)
		{
			intensityProfileList.push_back(it.GetMeasurementVector()[0]);
		}
	}
	return intensityProfileList;
}

void QmitkIntensityProfileVisualizationWidget::OnClipboardButtonClicked()
{
	if (m_IntensityProfileList.empty())
		return;

	QApplication::clipboard()->clear();

	QString clipboard("Pixel \t Intensity\n");
	for (unsigned int i = 0; i < m_IntensityProfileList.size(); i++)
	{
		clipboard = clipboard.append("%L1 \t %L2\n")
			.arg(QString::number(i))
			.arg(QString::number(m_IntensityProfileList.at(i)));
	}
	QApplication::clipboard()->setText(clipboard, QClipboard::Clipboard);
}

void QmitkIntensityProfileVisualizationWidget::OnShowSubchartCheckBoxChanged()
{
	m_Controls.chartWidget->Show(m_Controls.checkBoxShowSubchart->isChecked());
}

void QmitkIntensityProfileVisualizationWidget::OnPageSuccessfullyLoaded()
{
  m_Controls.chartWidget->SetTheme(m_ChartStyle);
}
