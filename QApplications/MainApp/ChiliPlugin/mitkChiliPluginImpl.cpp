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

#include "mitkChiliPluginImpl.h"

QcPlugin* mitk::ChiliPluginImpl::s_PluginInstance = 0;

bool mitk::ChiliPluginImpl::IsPlugin()
{
  return true;
}

QcPlugin* mitk::ChiliPluginImpl::GetPluginInstance()
{
  return s_PluginInstance;
}

void mitk::ChiliPluginImpl::SetPluginInstance(QcPlugin* instance)
{
  s_PluginInstance = instance;
}
mitk::ChiliPluginImpl::~ChiliPluginImpl() 
{ 
}

