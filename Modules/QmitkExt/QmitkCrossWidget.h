/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-05-15 18:09:46 +0200 (Fri, 15 May 2009) $
 Version:   $Revision: 17280 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef QMITKTCROSSWIDGET_H_INCLUDED
#define QMITKTCROSSWIDGET_H_INCLUDED

#include <QLabel>
#include "QmitkExtExports.h"

#include <mitkCommon.h>

class QmitkExt_EXPORT QmitkCrossWidget : public QLabel
{

  Q_OBJECT

public:

  QmitkCrossWidget( QWidget * parent=0, Qt::WindowFlags f = 0 );

  void mousePressEvent( QMouseEvent* mouseEvent );
  void mouseMoveEvent( QMouseEvent* mouseEvent );
  void mouseReleaseEvent( QMouseEvent* mouseEvent );

 signals:
 
  void SignalDeltaMove( int,int );

protected:

  int lastX,lastY;

};
#endif

