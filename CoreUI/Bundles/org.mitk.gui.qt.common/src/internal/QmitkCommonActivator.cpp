/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkCommonActivator.h"

#include <mitkGlobalInteraction.h>
#include <QmitkRegisterClasses.h>

#include <QFile>


void
QmitkCommonActivator::Start(berry::IBundleContext::Pointer)
{ 
  QFile file(":/org.mitk.gui.qt.common/StateMachine.xml");
  if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text) )
  {
    QByteArray contents = file.readAll();
    QString string(contents);
    file.close();
    mitk::GlobalInteraction::GetInstance()->Initialize("global", string.toStdString());
  }
  else throw std::exception();

  QmitkRegisterClasses();

}

