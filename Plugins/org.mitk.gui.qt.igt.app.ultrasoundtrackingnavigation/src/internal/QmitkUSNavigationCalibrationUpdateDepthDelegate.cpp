/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
