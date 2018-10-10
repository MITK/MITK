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


#ifndef QMITK_FIT_PLOT_DATA_WIDGET_H
#define QMITK_FIT_PLOT_DATA_WIDGET_H

#include "mitkModelFitPlotDataHelper.h"

#include "MitkModelFitUIExports.h"

#include "ui_QmitkFitPlotDataWidget.h"
#include <QWidget>
#include <memory>

class QmitkFitPlotDataModel;

/**
* \class QmitkFitPlotDataWidget
* Widget that displays the content of a ModelFitPlotData instance.
* In addition it allows to transfer this information as CSV into the clipboard or a file.
*/
class MITKMODELFITUI_EXPORT QmitkFitPlotDataWidget : public QWidget
{
  Q_OBJECT

public:
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  QmitkFitPlotDataWidget(QWidget* parent = 0);
  ~QmitkFitPlotDataWidget();

  const mitk::ModelFitPlotData* GetPlotData() const;
  const std::string& GetXName() const;

public Q_SLOTS:

  void SetPlotData(const mitk::ModelFitPlotData* data);
  void SetXName(const std::string& xName);

protected Q_SLOTS:
  void OnExportClicked() const;

  /** @brief Saves the results table to clipboard */
  void OnClipboardResultsButtonClicked() const;

protected:
  std::string StreamModelToString() const;

  QmitkFitPlotDataModel * m_InternalModel;

  Ui::QmitkFitPlotDataWidget m_Controls;

};

#endif // QmitkFitPlotDataWidget_H
