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
#ifndef QMITKUSNAVIGATIONCALIBRATIONUPDATEDEPTHDELEGATE_H
#define QMITKUSNAVIGATIONCALIBRATIONUPDATEDEPTHDELEGATE_H

#include <QStyledItemDelegate>

#include <mitkUSControlInterfaceBMode.h>

/**
 * \brief QStyledItemDelegate that changes the current depth of the ultasound image on double click.
 */
class QmitkUSNavigationCalibrationUpdateDepthDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkUSNavigationCalibrationUpdateDepthDelegate(QObject* parent = 0);
  ~QmitkUSNavigationCalibrationUpdateDepthDelegate();

  void SetControlInterfaceBMode(mitk::USControlInterfaceBMode::Pointer controlInterfaceBMode);

  /**
   * \brief The depth of the b mode ultrasound is changed on double click.
   * A mitk::USControlInterfaceBMode has to be set before by calling
   * SetControlInterfaceBMode(). If no control interface was set (or it is
   * null) the QEvent gets ignored.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
  mitk::USControlInterfaceBMode::Pointer m_ControlInterfaceBMode;
};

#endif // QMITKUSNAVIGATIONCALIBRATIONUPDATEDEPTHDELEGATE_H
