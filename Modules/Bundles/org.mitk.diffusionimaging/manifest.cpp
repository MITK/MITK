/*=========================================================================

Program:   MITK Platform
Language:  C++
Date:      $Date: 2009-05-18 17:57:41 +0200 (Mo, 18 Mai 2009) $
Version:   $Revision: 17020 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <Poco/ClassLibrary.h>

#include <berryIBundleActivator.h>
#include "src/internal/mitkDiffusionImagingActivator.h"

POCO_BEGIN_MANIFEST(berry::IBundleActivator)
POCO_EXPORT_CLASS(mitk::DiffusionImagingActivator)
POCO_END_MANIFEST
