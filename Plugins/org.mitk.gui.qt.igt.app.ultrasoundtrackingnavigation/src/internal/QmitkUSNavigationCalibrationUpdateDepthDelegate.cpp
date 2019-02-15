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
#include "QmitkUSNavigationCalibrationUpdateDepthDelegate.h"

#include <QEvent>

QmitkUSNavigationCalibrationUpdateDepthDelegate::QmitkUSNavigationCalibrationUpdateDepthDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QmitkUSNavigationCalibrationUpdateDepthDelegate::~QmitkUSNavigationCalibrationUpdateDepthDelegate()
{
}

void QmitkUSNavigationCalibrationUpdateDepthDelegate::SetControlInterfaceBMode(mitk::USControlInterfaceBMode::Pointer controlInterfaceBMode)
{
  m_ControlInterfaceBMode = controlInterfaceBMode;
}

bool QmitkUSNavigationCalibrationUpdateDepthDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem& option, const QModelIndex &index)
{
  if( event->type() == QEvent::MouseButtonDblClick && m_ControlInterfaceBMode.IsNotNull() )
  {
    // change the b mode ultrasound image to the selected depth of the data model
    m_ControlInterfaceBMode->SetScanningDepth(model->data(index).toDouble());
    return true;
  }
  else
  {
    return QStyledItemDelegate::editorEvent(event, model, option, index);
  }
}
