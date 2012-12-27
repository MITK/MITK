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

#include <Poco/ClassLibrary.h>

#include <mitkIInputDevice.h>
#include "src/internal/mitkSpaceNavigatorActivator.h"


POCO_BEGIN_NAMED_MANIFEST(mitkIInputDevice, mitk::IInputDevice)
  POCO_EXPORT_CLASS(mitk::SpaceNavigatorActivator)
POCO_END_MANIFEST
