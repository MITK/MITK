/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkUSNavigationCalibrationUpdateDepthDelegate_h
#define QmitkUSNavigationCalibrationUpdateDepthDelegate_h

#include <QStyledItemDelegate>

#include <mitkUSControlInterfaceBMode.h>

/**
 * \brief QStyledItemDelegate that changes the current depth of the ultasound image on double click.
 */
class QmitkUSNavigationCalibrationUpdateDepthDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkUSNavigationCalibrationUpdateDepthDelegate(QObject* parent = nullptr);
  ~QmitkUSNavigationCalibrationUpdateDepthDelegate() override;

  void SetControlInterfaceBMode(mitk::USControlInterfaceBMode::Pointer controlInterfaceBMode);

  /**
   * \brief The depth of the b mode ultrasound is changed on double click.
   * A mitk::USControlInterfaceBMode has to be set before by calling
   * SetControlInterfaceBMode(). If no control interface was set (or it is
   * null) the QEvent gets ignored.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
  mitk::USControlInterfaceBMode::Pointer m_ControlInterfaceBMode;
};

#endif
