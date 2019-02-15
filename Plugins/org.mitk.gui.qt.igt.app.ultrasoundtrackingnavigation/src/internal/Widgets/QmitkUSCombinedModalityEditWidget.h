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
  explicit QmitkUSCombinedModalityEditWidget(QWidget *parent = 0);
  ~QmitkUSCombinedModalityEditWidget();

  void SetCombinedModality(mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality);

private:
  mitk::AbstractUltrasoundTrackerDevice::Pointer                m_CombinedModality;

  std::string                                      m_LastCalibrations;

  QmitkUSNavigationCalibrationsDataModel*          m_CalibrationsDataModel;
  QmitkUSNavigationCalibrationUpdateDepthDelegate* m_CalibrationUpdateDepthDelegate;

  Ui::QmitkUSCombinedModalityEditWidget*           ui;
};

#endif // QMITKUSCOMBINEDMODALITYEDITWIDGET_H
