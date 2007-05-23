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

mitk::ChiliPlugin::ChiliPlugin()
{
}

mitk::ChiliPlugin::~ChiliPlugin()
{
}

bool mitk::ChiliPlugin::IsPlugin()
{
  return false;
}

int mitk::ChiliPlugin::GetConferenceID()
{
  return 0;
}

QcPlugin* mitk::ChiliPlugin::GetPluginInstance()
{
  return 0;
}

mitk::ChiliPlugin::StudyInformation mitk::ChiliPlugin::GetCurrentStudy()
{
  mitk::ChiliPlugin::StudyInformation emptyList;
  return emptyList;
}

mitk::ChiliPlugin::SeriesList mitk::ChiliPlugin::GetCurrentSeries()
{
  mitk::ChiliPlugin::SeriesList emptyList;
  return emptyList;
}

mitk::ChiliPlugin* mitk::ChiliPlugin::GetInstance()
{
  static mitk::ChiliPlugin::Pointer s_Instance = mitk::ChiliPlugin::New();
  return s_Instance;
}

void mitk::ChiliPlugin::SetPluginInstance(QcPlugin* instance)
{
}
