/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <Poco/ClassLibrary.h>

#include <mitkIInputDevice.h>
#include "src/internal/mitkSpaceNavigatorActivator.h"


POCO_BEGIN_NAMED_MANIFEST(mitkIInputDevice, mitk::IInputDevice)
  POCO_EXPORT_CLASS(mitk::SpaceNavigatorActivator)
POCO_END_MANIFEST
