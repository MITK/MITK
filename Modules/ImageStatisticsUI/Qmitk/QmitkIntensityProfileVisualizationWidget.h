/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkIntensityProfileVisualizationWidget_H__INCLUDED
#define QmitkIntensityProfileVisualizationWidget_H__INCLUDED

//Qt
#include <QWidget>

#include <MitkImageStatisticsUIExports.h>
#include <ui_QmitkIntensityProfileVisualizationWidget.h>

//mitk
#include <mitkIntensityProfile.h>

/**
* \brief Widget for displaying intensity profiles.
*/

class MITKIMAGESTATISTICSUI_EXPORT QmitkIntensityProfileVisualizationWidget : public QWidget
{
	Q_OBJECT

public:
	QmitkIntensityProfileVisualizationWidget(QWidget* parent = nullptr);

	void SetIntensityProfile(mitk::IntensityProfile::ConstPointer intensityProfile, const std::string& dataLabel);
	/** \brief Clears the intensity profile and disables all GUI elements. */
	void Reset();

  void SetTheme(QmitkChartWidget::ColorTheme style);

private:

	void CreateConnections();

	void SetGUIElementsEnabled(bool enabled);

	std::vector<double> ConvertIntensityProfileToVector(mitk::IntensityProfile::ConstPointer intensityProfile) const;

	/** \brief  Saves the intensity profile to the clipboard. */
	void OnClipboardButtonClicked();
	/** \brief Shows / Hides the subchart. */
	void OnShowSubchartCheckBoxChanged();

  void OnPageSuccessfullyLoaded();

private:

	Ui::QmitkIntensityProfileControls m_Controls;
  QmitkChartWidget::ColorTheme m_ChartStyle = QmitkChartWidget::ColorTheme::darkstyle;

	std::vector<double> m_IntensityProfileList;
};

#endif //QmitkIntensityProfileVisualizationWidget_H__INCLUDED
