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

/** Return a singleton from the current ChiliPlugin. */
mitk::ChiliPlugin* mitk::ChiliPlugin::GetInstance(bool destroyInstance)
{
  static mitk::ChiliPlugin::Pointer s_Instance = mitk::ChiliPlugin::New();

  if( destroyInstance )  // needed because of double inheritance of mitk::ChiliPluginImpl
  {
    s_Instance = NULL;
  }

  return s_Instance;
}

/** DefaultImplementation */
mitk::ChiliPlugin::ChiliPlugin()
{
}

/** DefaultImplementation */
mitk::ChiliPlugin::~ChiliPlugin()
{
}

/** DefaultImplementation */
int mitk::ChiliPlugin::GetConferenceID()
{
  return 0;
}

/** DefaultImplementation */
bool mitk::ChiliPlugin::IsPlugin()
{
  return false;
}

/** DefaultImplementation */
mitk::ChiliPlugin::StudyInformation mitk::ChiliPlugin::GetStudyInformation( const std::string& )
{
  StudyInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::ChiliPlugin::PatientInformation mitk::ChiliPlugin::GetPatientInformation( const std::string& )
{
  PatientInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::ChiliPlugin::SeriesInformation mitk::ChiliPlugin::GetSeriesInformation( const std::string& )
{
  SeriesInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::ChiliPlugin::SeriesInformationList mitk::ChiliPlugin::GetSeriesInformationList( const std::string& )
{
  SeriesInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}

/** DefaultImplementation */
mitk::ChiliPlugin::TextInformation mitk::ChiliPlugin::GetTextInformation( const std::string& )
{
  TextInformation emptyResult;
  return emptyResult;
}

/** DefaultImplementation */
mitk::ChiliPlugin::TextInformationList mitk::ChiliPlugin::GetTextInformationList( const std::string& )
{
  TextInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}

/** DefaultImplementation */
unsigned int mitk::ChiliPlugin::GetLightboxCount()
{
  return 0;
}

/** DefaultImplementation */
QcLightbox* mitk::ChiliPlugin::GetNewLightbox()
{
  return NULL;
}

/** DefaultImplementation */
QcLightbox* mitk::ChiliPlugin::GetCurrentLightbox()
{
  return NULL;
}

/** DefaultImplementation */
void mitk::ChiliPlugin::SetReaderType( unsigned int )
{
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromLightbox( QcLightbox* )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadCompleteSeries( const std::string& )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadAllImagesFromSeries( const std::string& )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadAllTextsFromSeries( const std::string& )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}

/** DefaultImplementation */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadOneText( const std::string& )
{
  return NULL;
}

/** DefaultImplementation */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadOneText( const std::string& , const std::string& , const std::string& )
{
  return NULL;
}

/** DefaultImplementation */
void mitk::ChiliPlugin::SaveToChili( DataStorage::SetOfObjects::ConstPointer )
{
}

/** DefaultImplementation */
void mitk::ChiliPlugin::SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer , std::string , int , std::string, bool )
{
}

/** DefaultImplementation */
void mitk::ChiliPlugin::SaveToSeries( DataStorage::SetOfObjects::ConstPointer , std::string , std::string , bool )
{
}

