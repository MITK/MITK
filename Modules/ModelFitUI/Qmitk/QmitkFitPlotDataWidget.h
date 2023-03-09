/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFitPlotDataWidget_h
#define QmitkFitPlotDataWidget_h

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

  QmitkFitPlotDataWidget(QWidget* parent = nullptr);
  ~QmitkFitPlotDataWidget() override;

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

#endif
