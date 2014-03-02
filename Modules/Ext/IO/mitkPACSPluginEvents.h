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

#ifndef mitkPACSPluginEventshincluded
#define mitkPACSPluginEventshincluded

#include <itkObject.h>
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

namespace mitk
{
  #pragma GCC visibility push(default)
  itkEventMacro( PluginEvent, itk::AnyEvent );
  itkEventMacro( PluginStudySelected, PluginEvent );
  itkEventMacro( PluginLightBoxCountChanged, PluginEvent );
  itkEventMacro( PluginAbortPACSImport, PluginEvent );
  #pragma GCC visibility pop
}

#endif

