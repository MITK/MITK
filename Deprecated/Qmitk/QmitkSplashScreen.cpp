/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkSplashScreen.h"

#include <qapplication.h>
#include <qpixmap.h>

QmitkSplashScreen::QmitkSplashScreen(const QPixmap& pixmap)
:QWidget(NULL,NULL, Qt::WStyle_StaysOnTop | Qt::WStyle_NoBorder | Qt::WStyle_Customize )
{
  setErasePixmap( pixmap );
  resize( pixmap.size() );
  QRect screenSize = QApplication::desktop()->screenGeometry();
  move( screenSize.center() - QWidget::rect().center() );
  
  show();
  repaint();
}

QmitkSplashScreen::~QmitkSplashScreen()
{
}

void QmitkSplashScreen::repaint()
{
  QWidget::repaint();
  QApplication::flush();
}

void QmitkSplashScreen::close()
{
  QWidget::close(true); // true = also delete widget
}

