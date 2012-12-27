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
#ifndef MITKOBJECTOBSERVER_H
#define MITKOBJECTOBSERVER_H

#include <itkObject.h>
#include "mitkSetObserver.h"

namespace mitk
{

  ///
  /// Abstract interface for classes
  /// that want to be informed when an
  /// itk object was modified or deleted
  ///
  struct ObjectObserver: public SetObserver<itk::Object*>
  {
  };
}

#endif // MITKOBJECTOBSERVER_H
