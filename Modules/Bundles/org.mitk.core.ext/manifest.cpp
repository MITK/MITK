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

#include <Poco/ClassLibrary.h>

#include <berryIBundleActivator.h>
#include "src/internal/mitkCoreExtActivator.h"

POCO_BEGIN_MANIFEST(berry::IBundleActivator)
  POCO_EXPORT_CLASS(mitk::CoreExtActivator)
POCO_END_MANIFEST
