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

#ifndef QMITKDiffusionImagingAppAPPLICATION_H_
#define QMITKDiffusionImagingAppAPPLICATION_H_

#include <berryIApplication.h>

class QmitkDiffusionImagingAppApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:
  
  QmitkDiffusionImagingAppApplication() {}
  QmitkDiffusionImagingAppApplication(const QmitkDiffusionImagingAppApplication& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  ~QmitkDiffusionImagingAppApplication() {}

  int Start();
  void Stop();
};

#endif /*QMITKDiffusionImagingAppAPPLICATION_H_*/
