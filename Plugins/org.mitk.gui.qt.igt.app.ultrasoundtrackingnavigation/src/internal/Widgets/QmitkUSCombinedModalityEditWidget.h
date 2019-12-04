/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QMITKUSCOMBINEDMODALITYEDITWIDGET_H
#define QMITKUSCOMBINEDMODALITYEDITWIDGET_H

#include <QWidget>

#include "mitkUSCombinedModality.h"

namespace Ui {
class QmitkUSCombinedModalityEditWidget;
}

class QmitkUSNavigationCalibrationsDataModel;
class QmitkUSNavigationCalibrationUpdateDepthDelegate;

class QmitkUSCombinedModalityEditWidget : public QWidget
{
  Q_OBJECT

signals:
  void SignalSaved();
  void SignalAborted();

protected slots:
  void OnSaveButtonClicked();
  void OnCancelButtonClicked();
  void OnCalibrationsSaveButtonClicked();
  void OnCalibrationsLoadButtonClicked();

public:
  explicit QmitkUSCombinedModalityEditWidget(QWidget *parent = nullptr);
  ~QmitkUSCombinedModalityEditWidget() override;

  void SetCombinedModality(mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality);

private:
  mitk::AbstractUltrasoundTrackerDevice::Pointer                m_CombinedModality;

  std::string                                      m_LastCalibrations;

  QmitkUSNavigationCalibrationsDataModel*          m_CalibrationsDataModel;
  QmitkUSNavigationCalibrationUpdateDepthDelegate* m_CalibrationUpdateDepthDelegate;

  Ui::QmitkUSCombinedModalityEditWidget*           ui;
};

#endif // QMITKUSCOMBINEDMODALITYEDITWIDGET_H
