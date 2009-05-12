/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2008-04-24 18:31:42 +0200 (Do, 24 Apr 2008) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkToolGUIArea.h"

QmitkToolGUIArea::QmitkToolGUIArea( QWidget* parent, Qt::WFlags f )
:QWidget(parent,f)
{
  QWidget::setContentsMargins(0, 0, 0, 0);
}

QmitkToolGUIArea::~QmitkToolGUIArea()
{
}

