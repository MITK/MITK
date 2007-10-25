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

#include "mitkPACSPlugin.h"

/** Return a singleton from the current ChiliPlugin. */
mitk::PACSPlugin* mitk::PACSPlugin::GetInstance(bool destroyInstance)
{
  static mitk::PACSPlugin::Pointer s_Instance = mitk::PACSPlugin::New();

  if( destroyInstance )  // needed because of double inheritance of mitk::ChiliPlugin
  {
    s_Instance = NULL;
  }

  return s_Instance;
}

/** DefaultImplementation */
mitk::PACSPlugin::PACSPlugin()
{
}

/** DefaultImplementation */
mitk::PACSPlugin::~PACSPlugin()
{
}

/** DefaultImplementation */
int mitk::PACSPlugin::GetConferenceID()
{
  return 0;
}

/** DefaultImplementation */
bool mitk::PACSPlugin::IsPlugin()
{
  return false;
}

/** DefaultImplementation */
mitk::PACSPlugin::StudyInformation mitk::PACSPlugin::GetStudyInformation( const std::string& )
{
  StudyInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::PACSPlugin::PatientInformation mitk::PACSPlugin::GetPatientInformation( const std::string& )
{
  PatientInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::PACSPlugin::SeriesInformation mitk::PACSPlugin::GetSeriesInformation( const std::string& )
{
  SeriesInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::PACSPlugin::SeriesInformationList mitk::PACSPlugin::GetSeriesInformationList( const std::string& )
{
  SeriesInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}

/** DefaultImplementation */
mitk::PACSPlugin::TextInformation mitk::PACSPlugin::GetTextInformation( const std::string& )
{
  TextInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::PACSPlugin::TextInformationList mitk::PACSPlugin::GetTextInformationList( const std::string& )
{
  TextInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}

/** DefaultImplementation */
unsigned int mitk::PACSPlugin::GetLightboxCount()
{
  return 0;
}

/** DefaultImplementation */
QcLightbox* mitk::PACSPlugin::GetNewLightbox()
{
  return NULL;
}

/** DefaultImplementation */
QcLightbox* mitk::PACSPlugin::GetCurrentLightbox()
{
  return NULL;
}

/** DefaultImplementation */
void mitk::PACSPlugin::SetReaderType( unsigned int )
{
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadImagesFromLightbox( QcLightbox* )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadCompleteSeries( const std::string& )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadAllImagesFromSeries( const std::string& )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadAllTextsFromSeries( const std::string& )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
mitk::DataTreeNode::Pointer mitk::PACSPlugin::LoadOneText( const std::string& )
{
  return NULL;
}

/** DefaultImplementation */
mitk::DataTreeNode::Pointer mitk::PACSPlugin::LoadOneText( const std::string& , const std::string& , const std::string& )
{
  return NULL;
}

/** DefaultImplementation */
void mitk::PACSPlugin::LoadParentChildRelation( const std::string& )
{
  return;
}

/** DefaultImplementation */
void mitk::PACSPlugin::SaveToChili( DataStorage::SetOfObjects::ConstPointer )
{
}

/** DefaultImplementation */
void mitk::PACSPlugin::SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer , std::string , int , std::string, bool )
{
}

/** DefaultImplementation */
void mitk::PACSPlugin::SaveToSeries( DataStorage::SetOfObjects::ConstPointer , std::string , std::string , bool )
{
}

