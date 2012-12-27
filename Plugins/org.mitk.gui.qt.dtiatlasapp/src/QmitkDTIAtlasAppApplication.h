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
