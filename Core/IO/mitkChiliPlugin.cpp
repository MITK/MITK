/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkChiliPlugin.h"

QcPlugin* mitk::ChiliPlugin::s_PluginInstance = 0;

bool mitk::ChiliPlugin::IsPlugin()
{
#ifdef CHILIPLUGIN
  return true;
#else
  return false;
#endif
}

QcPlugin* mitk::ChiliPlugin::GetPluginInstance()
{
  return s_PluginInstance;
}

void mitk::ChiliPlugin::SetPluginInstance(QcPlugin* instance)
{
  s_PluginInstance = instance;
}
