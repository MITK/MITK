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

#ifndef QMITKDTIAtlasAppAPPLICATION_H_
#define QMITKDTIAtlasAppAPPLICATION_H_

#include <berryIApplication.h>

class QmitkDTIAtlasAppApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:
  
  QmitkDTIAtlasAppApplication() {}
  QmitkDTIAtlasAppApplication(const QmitkDTIAtlasAppApplication& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  ~QmitkDTIAtlasAppApplication() {}

  int Start();
  void Stop();
};

#endif /*QMITKDTIAtlasAppAPPLICATION_H_*/
