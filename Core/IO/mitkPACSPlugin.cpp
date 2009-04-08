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

#include <iostream>

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

void mitk::PACSPlugin::UploadFileAsNewSeries( const std::string& /* filename */,
                                              const std::string& /* mimeType */, 
                                              const std::string& /* studyInstanceUID */, 
                                              int /* seriesNumber */, 
                                              const std::string& /* seriesDescription */ )
{
}

void mitk::PACSPlugin::UploadFileToSeries( const std::string& /* filename */,
                                           const std::string& /* filebasename */, 
                                           const std::string& /* mimeType */, 
                                           const std::string& /* seriesInstanceUID */, 
                                           bool /* overwriteExistingSeries */ )
{
}

std::string mitk::PACSPlugin::GuessMIMEType( const std::string& filename )
{
  std::ifstream file( filename.c_str() );
  if (!file)
  {
    // cannot open file
    return std::string("");
  }

  const unsigned int maxLength = 8;
  char line[ maxLength];
  file.getline( line, maxLength );
  file.close();

  std::string firstLine( line );

  if ( firstLine.substr( 1, 3 ) == "PDF" )
  {
    return std::string("application/pdf");
  }

  if ( firstLine.substr( 0, 5 ) == "{\rtf" )
  {
    return std::string("text/richtext");
  }

  if ( firstLine.substr( 0, 2 ) == "PK" )
  {
    return std::string("application/zip");
  }

  if ( (line[0] == 0xFF) && (line[1] == 0xD8) )
  {
    return std::string("image/jpeg");
  }

  if ( (line[0] == 0x89) && 
       (line[1] == 0x50) &&
       (line[2] == 0x4E) &&
       (line[3] == 0x47) &&
       (line[4] == 0x0D) &&
       (line[5] == 0x0A) &&
       (line[6] == 0x1A) &&
       (line[7] == 0x0A) 
     )
  {
    return std::string("image/png");
  }

  if ( (line[0] == 0x4D) && (line[1] == 0x5A) )
  {
    //return std::string("application/octet-stream");
    return std::string("Windows EXE");
  }

  if ( ( filename.rfind( ".stl" ) == filename.length() - 4 ) ||
       ( filename.rfind( ".STL" ) == filename.length() - 4 ) )
  {
    // this is a guess. don't know a good way to test for STL.
    return std::string("application/sla");
  }

  if ( ( filename.rfind( ".txt" ) == filename.length() - 4 ) ||
       ( filename.rfind( ".TXT" ) == filename.length() - 4 ) )
  {
    // this is a guess. don't know a good way to test for STL.
    return std::string("text/plain");
  }

  return std::string("");
}

    
void mitk::PACSPlugin::DownloadSingleFile( const std::string& /*seriesInstanceUID*/, 
                                           unsigned int /*instanceNumber*/,
                                           const std::string& /*filename*/)
{
}
