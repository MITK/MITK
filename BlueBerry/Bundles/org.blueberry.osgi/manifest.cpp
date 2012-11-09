/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <Poco/ClassLibrary.h>

#include "src/internal/berrySystemBundleActivator.h"

POCO_BEGIN_MANIFEST(berry::IBundleActivator)
  POCO_EXPORT_CLASS(berry::SystemBundleActivator)
POCO_END_MANIFEST
