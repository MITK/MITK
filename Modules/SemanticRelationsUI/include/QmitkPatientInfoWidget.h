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

#ifndef QMITKPATIENTINFOWIDGET_H
#define QMITKPATIENTINFOWIDGET_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <ui_QmitkPatientInfoWidgetControls.h>

// qt
#include <QWidget>

// mitk core
#include <mitkDataNode.h>

/*
* @brief The QmitkPatientInfoWidget is a widget that shows some DICOM information about a specific patient (data node).
*
* CURRENTLY NOT USED
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkPatientInfoWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkPatientInfoWidget(QWidget* parent = nullptr);
  ~QmitkPatientInfoWidget();

  void SetPatientInfo(const mitk::DataNode* dataNode);

private:

  void QmitkPatientInfoWidget::Init();

  Ui::QmitkPatientInfoWidgetControls m_Controls;

};

#endif // QMITKPATIENTINFOWIDGET_H
