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

#ifndef QmitkDndFrameWidget_h
#define QmitkDndFrameWidget_h

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QWidget>
#include <org_mitk_gui_qt_common_Export.h>
//drag&drop
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

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


private:
  void dragEnterEvent( QDragEnterEvent *event );
  void dropEvent( QDropEvent * event );


};


#endif
