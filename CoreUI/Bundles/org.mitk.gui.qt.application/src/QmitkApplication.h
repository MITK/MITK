/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QMITKAPPLICATION_H_
#define QMITKAPPLICATION_H_

#include <berryIApplication.h>

#include <QObject>

#include <org_mitk_gui_qt_application_Export.h>

class MITK_QT_APP QmitkApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)
  
public:
  
  QmitkApplication();
  QmitkApplication(const QmitkApplication& other);
  
  int Start();
  void Stop();
};

#endif /*QMITKAPPLICATION_H_*/
