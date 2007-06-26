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

unsigned int mitk::ChiliPlugin::GetLightBoxCount()
{
  return 0;
}

mitk::ChiliPlugin::StudyInformation mitk::ChiliPlugin::GetCurrentSelectedStudy()
{
  mitk::ChiliPlugin::StudyInformation emptyList;
  return emptyList;
}

mitk::ChiliPlugin::SeriesList mitk::ChiliPlugin::GetCurrentSelectedSeries()
{
  mitk::ChiliPlugin::SeriesList emptyList;
  return emptyList;
}

void mitk::ChiliPlugin::AddPropertyListToNode( const mitk::PropertyList::Pointer, mitk::DataTreeNode* )
{
}

mitk::ChiliPlugin* mitk::ChiliPlugin::GetInstance(bool destroyInstance)
{
  static mitk::ChiliPlugin::Pointer s_Instance = mitk::ChiliPlugin::New();

  if (destroyInstance)  // needed because of double inheritance of mitk::ChiliPluginImpl
    s_Instance = NULL;

  return s_Instance;
}

unsigned int mitk::ChiliPlugin::GetChiliVersion()
{
  return CHILI_VERSION;
}

void mitk::ChiliPlugin::UploadViaFile( DataTreeNode*, std::string studyInstanceUID, std::string patientOID, std::string studyOID, std::string seriesOID )
{
}

void mitk::ChiliPlugin::UploadViaBuffer( DataTreeNode* )
{
}

void mitk::ChiliPlugin::DownloadViaFile( std::string chiliText, std::string MimeType, mitk::DataTreeIteratorBase* parentIterator )
{
}

mitk::DataTreeNode* mitk::ChiliPlugin::DownloadViaBuffer()
{
  return NULL;
}

mitk::ChiliPlugin::PatientInformation mitk::ChiliPlugin::GetCurrentSelectedPatient()
{
  mitk::ChiliPlugin::PatientInformation emptyList;
  return emptyList;
}

mitk::ChiliPlugin::TextFileList mitk::ChiliPlugin::GetTextFileInformation( std::string seriesOID )
{
  mitk::ChiliPlugin::TextFileList emptyList;
  return emptyList;
}
