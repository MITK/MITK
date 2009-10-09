/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-02-10 14:14:32 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16224 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkDndFrameWidget_h
#define QmitkDndFrameWidget_h

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QWidget>
//drag&drop
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class QmitkDnDFrameWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkDnDFrameWidget(QWidget *parent);
  

private:
  void dragEnterEvent( QDragEnterEvent *event );
  void dropEvent( QDropEvent * event );

 
};


#endif 
