/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDnDFrameWidget_h
#define QmitkDnDFrameWidget_h

#include <QWidget>
#include <org_mitk_gui_qt_common_Export.h>
//drag&drop
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class QmitkDnDFrameWidgetPrivate;

/**
 * \ingroup org_mitk_gui_qt_common
 *
 * \brief Enables loading data into a MITK application via Drag'n Drop.
 *
 * You can use this class inside your MITK View as a container for your widgets
 * if you want the user to be able to load data by dropping files onto your view.
 */
class MITK_QT_COMMON QmitkDnDFrameWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkDnDFrameWidget(QWidget *parent);
  ~QmitkDnDFrameWidget() override;

private:
  void dragEnterEvent( QDragEnterEvent *event ) override;
  void dropEvent( QDropEvent * event ) override;

  const QScopedPointer<QmitkDnDFrameWidgetPrivate> d;

};


#endif
