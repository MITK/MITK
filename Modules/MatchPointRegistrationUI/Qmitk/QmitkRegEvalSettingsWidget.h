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

#ifndef QMITK_REG_EVAL_SETTINGS_WIDGET_H
#define QMITK_REG_EVAL_SETTINGS_WIDGET_H

#include <MitkMatchPointRegistrationUIExports.h>

#include "ui_QmitkRegEvalSettingsWidget.h"
#include <QWidget>

#include <mitkDataNode.h>

/**
 * \class QmitkRegEvalSettingsWidget
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkRegEvalSettingsWidget : public QWidget,
                                                                       private Ui::QmitkRegEvalSettingsWidget
{
  Q_OBJECT

public:
  QmitkRegEvalSettingsWidget(QWidget *parent = nullptr);

  /**
   * Configures the passed settings according to the current state of the
   * widget.
   * \param pointer to a instance based on QmitkMappingJobSettings.
   * \pre settings must point to a valid instance..
   */
  void ConfigureControls();

public Q_SLOTS:
  /**
    * \brief Slot that can be used to set the node that should be configured by the widget.*/
  void SetNode(mitk::DataNode *node);

signals:
  void SettingsChanged(mitk::DataNode *node);

protected Q_SLOTS:
  void OnComboStyleChanged(int);
  void OnBlend50Pushed();
  void OnBlendTargetPushed();
  void OnBlendMovingPushed();
  void OnBlendTogglePushed();
  void OnSlideBlendChanged(int);
  void OnSpinBlendChanged(int);
  void OnSpinCheckerChanged(int);
  void OnWipeStyleChanged();
  void OnContourStyleChanged();

private:
  mitk::DataNode::Pointer m_selectedEvalNode;

  bool m_internalBlendUpdate;
  bool m_internalUpdate;
};

#endif // QmitkRegEvalSettingsWidget_H
