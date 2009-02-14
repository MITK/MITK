/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

mitk::PACSPlugin* mitk::PACSPlugin::GetInstance(bool destroyInstance)
{
  static mitk::PACSPlugin::Pointer s_Instance = mitk::PACSPlugin::New();

  if( destroyInstance )
  {
    s_Instance = NULL;
  }

  return s_Instance;
}


mitk::PACSPlugin::PACSPlugin()
: m_ReaderType(0)
{
}


mitk::PACSPlugin::~PACSPlugin()
{
}


mitk::PACSPlugin::PACSPluginCapability mitk::PACSPlugin::GetPluginCapabilities()
{
  PACSPluginCapability result;
  result.IsPACSFunctional = false;
  result.HasLoadCapability = false;
  result.HasSaveCapability = false;
  return result;
}


mitk::PACSPlugin::PatientInformationList mitk::PACSPlugin::GetPatientInformationList()
{
  PatientInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}


mitk::PACSPlugin::StudyInformationList mitk::PACSPlugin::GetStudyInformationList( const PatientInformation& /* patient */ )
{
  StudyInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}


mitk::PACSPlugin::SeriesInformationList mitk::PACSPlugin::GetSeriesInformationList( const std::string& /*studyInstanceUID*/ )
{
  SeriesInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}


mitk::PACSPlugin::DocumentInformationList mitk::PACSPlugin::GetDocumentInformationList( const std::string& /*seriesInstanceUID*/ )
{
  DocumentInformationList emptyResult;
  emptyResult.clear();
  return emptyResult;
}


mitk::PACSPlugin::PatientInformation mitk::PACSPlugin::GetPatientInformation( const std::string& )
{
  PatientInformation emptyResult;
  emptyResult.PatientsSex = "O";
  emptyResult.PatientComments = "No PACS connectivity implemented or configured. Cannot query patient";
  return emptyResult;
}


mitk::PACSPlugin::StudyInformation mitk::PACSPlugin::GetStudyInformation( const std::string& )
{
  StudyInformation emptyResult;
  emptyResult.StudyDescription  = "No PACS connectivity implemented or configured. Cannot query study";
  return emptyResult;
}


mitk::PACSPlugin::SeriesInformation mitk::PACSPlugin::GetSeriesInformation( const std::string& )
{
  SeriesInformation emptyResult;
  emptyResult.SeriesDescription = "No PACS connectivity implemented or configured. Cannot query series.";
  return emptyResult;
}


mitk::PACSPlugin::DocumentInformation mitk::PACSPlugin::GetDocumentInformation( const std::string& itkNotUsed(seriesInstanceUID), 
                                                                                unsigned int itkNotUsed(instanceNumber) )
{
  DocumentInformation emptyResult;
  return emptyResult;
}


unsigned int mitk::PACSPlugin::GetLightboxCount()
{
  return 0;
}


unsigned int mitk::PACSPlugin::GetActiveLightbox()
{
  return (unsigned int)-1; // user should check GetLightboxCount() first, unsensible values might tell him that his request is not sensible
}


void mitk::PACSPlugin::SetReaderType( unsigned int readerType )
{
  m_ReaderType = readerType;
}


void mitk::PACSPlugin::AbortPACSImport()
{
}


std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadImagesFromLightbox( unsigned int itkNotUsed(lightboxIndex) )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}


std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadFromSeries( const std::string& seriesInstanceUID )
{
  std::vector<DataTreeNode::Pointer> resultVector = this->LoadImagesFromSeries( seriesInstanceUID );
  std::vector<DataTreeNode::Pointer> secondResultVector = this->LoadTextsFromSeries( seriesInstanceUID );
  resultVector.insert( resultVector.end(), secondResultVector.begin(), secondResultVector.end() );
  return resultVector;
}


std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadImagesFromSeries( const std::string& /* seriesInstanceUID */ )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}
    
std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadImagesFromSeries( std::vector<std::string> /* seriesInstanceUIDs */ )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}


std::vector<mitk::DataTreeNode::Pointer> mitk::PACSPlugin::LoadTextsFromSeries( const std::string&  /* seriesInstanceUID */ )
{
  std::vector<DataTreeNode::Pointer> emptyVector;
  emptyVector.clear();
  return emptyVector;
}


mitk::DataTreeNode::Pointer mitk::PACSPlugin::LoadSingleText( const std::string& /* seriesInstanceUID */ , unsigned int /* instanceNumber */ )
{
  return NULL;
}


void mitk::PACSPlugin::SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer /* inputNodes */, 
                                        const std::string& /* studyInstanceUID */, 
                                        int /* seriesNumber */, 
                                        const std::string& /*seriesDescription */)
{
}


void mitk::PACSPlugin::SaveToSeries( DataStorage::SetOfObjects::ConstPointer /* inputNodes */, 
                                     const std::string& /* seriesInstanceUID */,
                                     bool /* overwriteExistingSeries */)
{
}

