/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-01 08:56:22 +0200 (Di, 01 Apr 2008) $
Version:   $Revision: 13931 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkLoadFromCHILI.h"

//CHILI
#include <chili/plugin.h>
#include <chili/qclightbox.h>
#include <ipDicom/ipDicom.h>  //read DICOM-Files
#include <ipPic/ipPicTags.h>  //ipPicTags
//MITK
#include "mitkPicDescriptorToNode.h"
#include "mitkImageNumberFilter.h"
#include "mitkSingleSpacingFilter.h"
#include "mitkSpacingSetFilter.h"
#include "mitkStreamReader.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkProgressBar.h"

#ifdef CHILI_PLUGIN_VERSION_CODE

mitk::LoadFromCHILI::LoadFromCHILI()
{
  m_UsedReader = 0;
  m_ParentChild = ParentChild::New();
}

mitk::LoadFromCHILI::~LoadFromCHILI()
{
}

mitk::PACSPlugin::PSRelationInformationList mitk::LoadFromCHILI::GetSeriesRelationInformation( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory )
{
  return m_ParentChild->GetSeriesRelationInformation( instance, seriesOID, tmpDirectory );
}

mitk::PACSPlugin::PSRelationInformationList mitk::LoadFromCHILI::GetStudyRelationInformation( QcPlugin* instance, const std::string& studyOID, const std::string& tmpDirectory )
{
  return m_ParentChild->GetStudyRelationInformation( instance, studyOID, tmpDirectory );
}

void mitk::LoadFromCHILI::SetReaderType( unsigned int readerType )
{
  m_UsedReader = readerType;
}

std::vector<mitk::DataTreeNode::Pointer> mitk::LoadFromCHILI::LoadImagesFromLightbox( QcPlugin* instance, QcLightbox* lightbox, const std::string& tmpDirectory )
{
  m_ImageList.clear();
  m_StreamImageList.clear();
  m_UnknownImageFormatPath.clear();

  //read all frames from Lightbox
  ProgressBar::GetInstance()->AddStepsToDo( lightbox->getFrames() );
  for( unsigned int n = 0; n < lightbox->getFrames(); n++ )
  {
    ipPicDescriptor* pic = lightbox->fetchPic(n);
    if( pic )
    {
      if( pic->dim == 1 )
      {
        StreamImageStruct newElement = LoadStreamImage( pic );
        if( !newElement.imageList.empty() )
          m_StreamImageList.push_back( newElement );
      }
      else
        m_ImageList.push_back( pic );
    }
    ProgressBar::GetInstance()->Progress();
  }

  return CreateNodesFromLists( instance, lightbox->currentSeries()->oid, false, tmpDirectory );
}

mitk::LoadFromCHILI::StreamImageStruct mitk::LoadFromCHILI::LoadStreamImage( ipPicDescriptor* pic)
{
  StreamImageStruct newElement;
  newElement.imageList.clear();

#ifndef WIN32
  unsigned int frameNumber;
  ipPicTSV_t *tsv;
  void* data = NULL;
  ipUInt4_t len = 0;
  tsv = ipPicQueryTag( pic, (char*)"SOURCE HEADER" );
  if( tsv )
  {
    if( dicomFindElement( (unsigned char*) tsv->value, 0x0028, 0x0008, &data, &len ) && data != NULL )
    {
      ProgressBar::GetInstance()->AddStepsToDo( 2 );
      sscanf( (char *) data, "%d", &frameNumber );

      ipUInt4_t *offset_table;
      offset_table = (ipUInt4_t *)malloc( sizeof(ipUInt4_t) * frameNumber );
      for( unsigned int i = 0; i < frameNumber; ++i )
        offset_table[i] = 0xffffffff;

      for( unsigned int i = 0; i < frameNumber; i++ )
      {
        ipPicDescriptor* cinePic = ipPicDecompressJPEG( pic, i, frameNumber, NULL, offset_table );
        newElement.imageList.push_back( cinePic );
      }
      free( offset_table );
      ProgressBar::GetInstance()->Progress();

      //interSliceGeometry
      newElement.geometry = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
      if( !pFetchSliceGeometryFromPic( pic, newElement.geometry ) )
      {
        std::cout<<"Not able to load interSliceGeometry!"<<std::endl;
        free( newElement.geometry );
      }

      //SeriesDescription
      ipPicTSV_t* seriesDescriptionTag = ipPicQueryTag( pic, (char*)tagSERIES_DESCRIPTION );
      if( seriesDescriptionTag )
        newElement.seriesDescription = static_cast<char*>( seriesDescriptionTag->value );
      if( newElement.seriesDescription == "" )
      {
        ipPicTSV_t *tsv;
        void* data = NULL;
        ipUInt4_t len = 0;
        tsv = ipPicQueryTag( pic, (char*)"SOURCE HEADER" );
        if( tsv && dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
          newElement.seriesDescription = (char*)data;
      }
      ProgressBar::GetInstance()->Progress();
    }
  }
#endif
  return newElement;
}

std::vector<mitk::DataTreeNode::Pointer> mitk::LoadFromCHILI::CreateNodesFromLists( QcPlugin* instance, const std::string& seriesOID, bool deletePicDescriptor, const std::string& tmpDirectory )
{  //hint: text-files handled seperate
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

  if( !m_ImageList.empty() )
  {
    PicDescriptorToNode::Pointer converterToNode;
    switch ( m_UsedReader )
    {
      case 0:
      {
        converterToNode = ImageNumberFilter::New();
        break;
      }
      case 1:
      {
        converterToNode = SpacingSetFilter::New();
        break;
      }
      case 2:
      {
        converterToNode = SingleSpacingFilter::New();
        break;
      }
      default:
      {
        std::cout << "LoadFromCHILI (CreateNodesFromLists): undefined ReaderType. Abort!" << std::endl;
        return resultNodes;
      }
    }

    converterToNode->SetInput( m_ImageList, seriesOID );
    converterToNode->Update();
    resultNodes = converterToNode->GetOutput();
    //check if volumes known at parent-child-xml
    PACSPlugin::StudyInformation currentStudy = PACSPlugin::GetInstance()->GetStudyInformation( seriesOID );
    PACSPlugin::PatientInformation currentPatient = PACSPlugin::GetInstance()->GetPatientInformation( seriesOID );
    m_ParentChild->InitParentChild( instance, currentStudy.OID, currentStudy.InstanceUID, currentPatient.OID, tmpDirectory );
    std::vector< std::list< std::string > > ImageInstanceUIDs = converterToNode->GetImageInstanceUIDs();
    for( unsigned int x = 0; x < ImageInstanceUIDs.size(); x++ )
    {
      std::string result = m_ParentChild->GetLabel( ImageInstanceUIDs[x] );
      if( result != "" )
        resultNodes[x]->SetProperty( "VolumeLabel", StringProperty::New( result ) );
    }

    if( deletePicDescriptor )  //delete the loaded ipPicDescriptors
    {
      for( std::list<ipPicDescriptor*>::iterator imageIter = m_ImageList.begin(); imageIter != m_ImageList.end(); imageIter++ )
        ipPicFree( (*imageIter) );
      m_ImageList.clear();
    }
  }

  if( !m_StreamImageList.empty() )
  {
    StreamReader::Pointer streamReader = StreamReader::New();
    while( !m_StreamImageList.empty() )
    {
      streamReader->SetInput( m_StreamImageList.front().imageList, seriesOID );
      streamReader->SetSecondInput( m_StreamImageList.front().geometry, m_StreamImageList.front().seriesDescription );
      streamReader->Update();
      resultNodes.push_back( streamReader->GetOutput()[0] );

      //delete the ipPicDescriptors and interSliceGeometry
      for( std::list<ipPicDescriptor*>::iterator imageIter = m_StreamImageList.front().imageList.begin(); imageIter != m_StreamImageList.front().imageList.end(); imageIter++ )
        ipPicFree( (*imageIter) );
      free( m_StreamImageList.front().geometry );
      m_StreamImageList.pop_front();
    }
  }

  if( !m_UnknownImageFormatPath.empty() )
  {
    DataTreeNodeFactory::Pointer factory = DataTreeNodeFactory::New();
    try
    {
      while( !m_UnknownImageFormatPath.empty() )
      {
        factory->SetFileName( m_UnknownImageFormatPath.front().c_str() );
        factory->Update();
        for( unsigned int i = 0 ; i < factory->GetNumberOfOutputs(); ++i )
        {
          DataTreeNode::Pointer node = factory->GetOutput( i );
          if ( ( node.IsNotNull() ) && ( node->GetData() != NULL )  )
          {
            node->SetProperty( "SeriesOID", StringProperty::New( seriesOID ) );
            resultNodes.push_back( node );
          }
        }
        if( factory->GetNumberOfOutputs() > 0 && remove( m_UnknownImageFormatPath.front().c_str() ) != 0)
          std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << m_UnknownImageFormatPath.front().c_str() << std::endl;

        m_UnknownImageFormatPath.pop_front();
      }
    }
    catch ( itk::ExceptionObject & ex )
      itkGenericOutputMacro( << "Exception during file open: " << ex );
  }
  return resultNodes;
}

std::vector<mitk::DataTreeNode::Pointer> mitk::LoadFromCHILI::LoadFromSeries( QcPlugin* instance, const std::string& seriesOID, const std::string& tmpDirectory )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();
  resultNodes = LoadImagesFromSeries( instance, seriesOID, tmpDirectory ); //load the image
  std::vector<DataTreeNode::Pointer> tempResultNodes = LoadTextsFromSeries( instance, seriesOID, tmpDirectory ); //load the text-files
  while( !tempResultNodes.empty() ) //add the text-nodes
  {
    resultNodes.push_back( tempResultNodes.back() );
    tempResultNodes.pop_back();
  }
  return resultNodes;
}

std::vector<mitk::DataTreeNode::Pointer> mitk::LoadFromCHILI::LoadImagesFromSeries( QcPlugin* instance, const std::string&  seriesOID, const std::string& tmpDirectory )
{
  m_ImageList.clear();
  m_UnknownImageFormatPath.clear();
  m_StreamImageList.clear();

  //iterate over all images from this series, save them to harddisk, load them and add the picDescriptors to m_ImageList, unknown imagefiles get saved to unknownImageFormatPath
  m_tmpDirectory = tmpDirectory;
  pIterateImages( instance, (char*)seriesOID.c_str(), NULL, &LoadFromCHILI::GlobalIterateLoadImages, this );

  return CreateNodesFromLists( instance, seriesOID, true, tmpDirectory );
}

ipBool_t mitk::LoadFromCHILI::GlobalIterateLoadImages( int /*rows*/, int row, image_t* image, void* user_data )
{
  ProgressBar::GetInstance()->AddStepsToDo( 2 );
  LoadFromCHILI* callingObject = static_cast<LoadFromCHILI*>( user_data );

  //get the FileExtension
  std::string imagePath = image->path;
  std::string fileExtension = imagePath.substr( imagePath.find_last_of(".") + 1 );

  //create a new FileName (1.*, 2.*, 3.*, ...) with FileExtension from DB
  std::ostringstream stringHelper;
  stringHelper << row << "." << fileExtension;
  std::string pathAndFile = callingObject->m_tmpDirectory + stringHelper.str();

  //save to harddisk
  ipInt4_t error;
  pFetchDataToFile( image->path, pathAndFile.c_str(), &error );
  ProgressBar::GetInstance()->Progress();
  if( error != 0 )
    std::cout << "LoadFromCHILI (GlobalIterateLoadImage): ChiliError: " << error << ", while reading file (" << image->path << ") from Database." << std::endl;
  else
  {
    if( fileExtension == "pic" )
    {
      ipPicDescriptor *pic = ipPicGet( (char*)pathAndFile.c_str(), NULL );
      if( pic )
      {
        if( pic->dim == 1 )  //load cine-pics
        {
          StreamImageStruct newElement = callingObject->LoadStreamImage( pic );
          if( !newElement.imageList.empty() )
            callingObject->m_StreamImageList.push_back( newElement );
        }
        else  //load ipPicDescriptors
          callingObject->m_ImageList.push_back( pic );

        if( remove(  pathAndFile.c_str() ) != 0 )
          std::cout << "LoadFromCHILI (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
      }
    }
    else
    {
      if( fileExtension == "dcm" )  //load dicom-files
      {
        ipUInt1_t* header = NULL;
        ipUInt4_t  header_size;
        ipUInt1_t* dcimage = NULL;
        ipUInt4_t  image_size;

        dicomGetHeaderAndImage( (char*)pathAndFile.c_str(), &header, &header_size, &dcimage, &image_size );

        if( header && dcimage )
        {
          //ipPicDescriptor *pic = dicomToPic( header, header_size, image, image_size );
          ipPicDescriptor *pic = _dicomToPic( header, header_size, dcimage, image_size, ipFalse, ipTrue, ipTrue);
          if( pic )
          {
            callingObject->m_ImageList.push_back( pic );
            if( remove(  pathAndFile.c_str() ) != 0 )
              std::cout << "LoadFromCHILI (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
          }
        }
        else std::cout<< "LoadFromCHILI (GlobalIterateLoadImage): Could not get header or image." <<std::endl;
      }
      else
        callingObject->m_UnknownImageFormatPath.push_back( pathAndFile );
    }
  }
  ProgressBar::GetInstance()->Progress();

  return ipTrue; // true = next element; false = break iterate
}

std::vector<mitk::DataTreeNode::Pointer> mitk::LoadFromCHILI::LoadTextsFromSeries( QcPlugin* instance, const std::string&  seriesOID, const std::string& tmpDirectory )
{
  m_TextList.clear();

  pIterateTexts( instance, (char*)seriesOID.c_str(), NULL, &LoadFromCHILI::GlobalIterateLoadTexts, this );

  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();
  DataTreeNode::Pointer tempNode;

  while( !m_TextList.empty() )
  {
    tempNode = LoadSingleText( instance, seriesOID, m_TextList.front().TextFileOID, m_TextList.front().TextFilePath, tmpDirectory );
    if( tempNode.IsNotNull() )
      resultNodes.push_back( tempNode );
    m_TextList.pop_front();
  }

  return resultNodes;
}

ipBool_t mitk::LoadFromCHILI::GlobalIterateLoadTexts( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  LoadFromCHILI* callingObject = static_cast<LoadFromCHILI*>(user_data);

  //we dont want that to load the parent-child-relation-file
  std::string chiliDatabaseName = text->chiliText;
  std::string fileName = chiliDatabaseName.substr( chiliDatabaseName.find_last_of("-") + 1 );
  if( fileName != "ParentChild.xml" )
  {
    LoadFromCHILI::TextFilePathAndOIDStruct newTextFile;
    newTextFile.TextFilePath = text->chiliText;
    newTextFile.TextFileOID = text->oid;
    callingObject->m_TextList.push_back( newTextFile );
  }
  return ipTrue; // true = next element; false = break iterate
}

mitk::DataTreeNode::Pointer mitk::LoadFromCHILI::LoadSingleText( QcPlugin* instance, const std::string& textOID, const std::string& tmpDirectory )
{
  DataTreeNode::Pointer resultNode = NULL;

  text_t text;
  series_t series;
  initTextStruct( &text );
  initSeriesStruct( &series );
  text.oid = strdup( textOID.c_str() );

  if( pQueryText( instance, &text, &series, NULL, NULL ) )
    resultNode = LoadSingleText( instance, series.oid, text.oid, text.chiliText, tmpDirectory );
  else
    std::cout << "LoadFromCHILI (LoadSingleText): pQueryText() failed. Abort." << std::endl;

  clearTextStruct( &text );
  clearSeriesStruct( &series );
  return resultNode;
}

mitk::DataTreeNode::Pointer mitk::LoadFromCHILI::LoadSingleText( QcPlugin* instance, const std::string& seriesOID, const std::string& textOID, const std::string& textPath, const std::string& tmpDirectory )
{
  ProgressBar::GetInstance()->AddStepsToDo( 2 );
  DataTreeNode::Pointer resultNode = NULL;
  //get Filename and path to save to harddisk
  std::string fileName = textPath.substr( textPath.find_last_of("-") + 1 );
  std::string pathAndFile = tmpDirectory + fileName;

  //Load File from DB
  ipInt4_t error;
  pFetchDataToFile( textPath.c_str(), pathAndFile.c_str(), &error );
  ProgressBar::GetInstance()->Progress();
  if( error != 0 )
    std::cout << "LoadFromCHILI (LoadSingleText): ChiliError: " << error << ", while reading file (" << fileName << ") from Database." << std::endl;
  else
  {
    //if there are no fileextension in the filename, the reader not able to load the file and the plugin crashed
    if( fileName.find_last_of(".") == std::string::npos )
      std::cout << "LoadFromCHILI (LoadSingleText): Reader not able to read file without extension.\nIf you dont close CHILI you can find the file here: "<< pathAndFile <<"." << std::endl;
    else
    {
      //try to Read the File
      DataTreeNodeFactory::Pointer factory = DataTreeNodeFactory::New();
      try
      {
        factory->SetFileName( pathAndFile );
        factory->Update();
        for ( unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i )
        {
          resultNode = factory->GetOutput( i );
          if ( ( resultNode.IsNotNull() ) && ( resultNode->GetData() != NULL )  )
          {
            //get the FileExtension and cut them from the filename
            std::string fileNameWithoutExtension = fileName.substr( 0, fileName.find_last_of(".") );
            //set the filename without extension as name-property
            resultNode->SetProperty( "name", StringProperty::New( fileNameWithoutExtension ) );
            resultNode->SetProperty( "TextOID", StringProperty::New( textOID ) );
            resultNode->SetProperty( "SeriesOID", StringProperty::New( seriesOID ) );
            //it should be possible to override all non-image-entries
            resultNode->SetProperty( "CHILI: MANUFACTURER", StringProperty::New( "MITK" ) );
            resultNode->SetProperty( "CHILI: INSTITUTION NAME", StringProperty::New( "DKFZ.MBI" ) );
            //check if volumes known at parent-child-xml
            PACSPlugin::StudyInformation currentStudy = PACSPlugin::GetInstance()->GetStudyInformation( seriesOID );
            PACSPlugin::PatientInformation currentPatient = PACSPlugin::GetInstance()->GetPatientInformation( seriesOID );
            m_ParentChild->InitParentChild( instance, currentStudy.OID, currentStudy.InstanceUID, currentPatient.OID, tmpDirectory );
            std::list< std::string > UID;
            UID.clear();
            UID.push_back( textOID );
            std::string result = m_ParentChild->GetLabel( UID );
            if( result != "" )
              resultNode->SetProperty( "VolumeLabel", StringProperty::New( result ) );
            if( remove(  pathAndFile.c_str() ) != 0 )
              std::cout << "LoadFromCHILI (LoadSingleText): Not able to  delete file: " << pathAndFile << std::endl;
          }
        }
      }
      catch ( itk::ExceptionObject & ex )
        itkGenericOutputMacro( << "Exception during file open: " << ex );
    }
  }
  ProgressBar::GetInstance()->Progress();
  return resultNode;
}

mitk::DataTreeNode::Pointer mitk::LoadFromCHILI::LoadParentChildElement( QcPlugin* instance, const std::string& seriesOID, const std::string& label, const std::string& tmpDirectory )
{
  PACSPlugin::StudyInformation currentStudy = PACSPlugin::GetInstance()->GetStudyInformation( seriesOID );
  PACSPlugin::PatientInformation currentPatient = PACSPlugin::GetInstance()->GetPatientInformation( seriesOID );
  m_ParentChild->InitParentChild( instance, currentStudy.OID, currentStudy.InstanceUID, currentPatient.OID, tmpDirectory );
  std::list<std::string> uids = m_ParentChild->GetSlices( label, seriesOID );
  if( !uids.empty() )
  {
    if( uids.front() == "Text" )
    {
      uids.pop_front();
      return LoadSingleText( instance, uids.front(), tmpDirectory );
    }
    if( uids.front() == "Image" )
    {
      DataTreeNode::Pointer resultNode = NULL;
      uids.pop_front();
      m_VolumeImageInstanceUIDs = uids;
      m_ImageList.clear();
      pIterateImages( instance, (char*)seriesOID.c_str(), NULL, &LoadFromCHILI::GlobalIterateLoadSinglePics, this );

      if( m_ImageList.size() == uids.size() )
      {
        PicDescriptorToNode::Pointer converterToNode;
        converterToNode = SingleSpacingFilter::New();
        converterToNode->SetInput( m_ImageList, seriesOID );
        converterToNode->Update();
        std::vector<DataTreeNode::Pointer> tempResult = converterToNode->GetOutput();
        if( !tempResult.empty() )
        {
          //check if volume alway known in parent-child-xml
          std::vector< std::list< std::string > > ImageInstanceUIDs = converterToNode->GetImageInstanceUIDs();
          std::string result = m_ParentChild->GetLabel( ImageInstanceUIDs[0] );
          if( result != "" )
            tempResult[0]->SetProperty( "VolumeLabel", StringProperty::New( result ) );

          resultNode = tempResult[0];
        }
      }

      for( std::list<ipPicDescriptor*>::iterator imageIter = m_ImageList.begin(); imageIter != m_ImageList.end(); imageIter++ )
        ipPicFree( (*imageIter) );
      m_ImageList.clear();

      return resultNode;
    }
  }
  return NULL;
}

ipBool_t mitk::LoadFromCHILI::GlobalIterateLoadSinglePics( int /*rows*/, int row, image_t* image, void* user_data )
{
  ProgressBar::GetInstance()->AddStepsToDo( 2 );
  LoadFromCHILI* callingObject = static_cast<LoadFromCHILI*>( user_data );

  //if the current image_instance_UID exist in the m_SavedImageInstanceUIDs-list, this image get saved to the m_ImageList
  if( find( callingObject->m_VolumeImageInstanceUIDs.begin(), callingObject->m_VolumeImageInstanceUIDs.end(), image->instanceUID ) != callingObject->m_VolumeImageInstanceUIDs.end() )
  {
    //get the FileExtension
    std::string imagePath = image->path;
    std::string fileExtension = imagePath.substr( imagePath.find_last_of(".") + 1 );

    //create a new FileName (1.*, 2.*, 3.*, ...) with FileExtension from DB
    std::ostringstream stringHelper;
    stringHelper << row << "." << fileExtension;
    std::string pathAndFile = callingObject->m_tmpDirectory + stringHelper.str();

    //save to harddisk
    ipInt4_t error;
    pFetchDataToFile( image->path, pathAndFile.c_str(), &error );
    ProgressBar::GetInstance()->Progress();
    if( error != 0 )
      std::cout << "LoadFromCHILI (GlobalIterateLoadImage): ChiliError: " << error << ", while reading file (" << image->path << ") from Database." << std::endl;
    else
    {
      if( fileExtension == "pic" )
      {
        ipPicDescriptor *pic = ipPicGet( (char*)pathAndFile.c_str(), NULL );
        if( pic )
        {
          if( pic->dim == 1 )  //load cine-pics
          {
            StreamImageStruct newElement = callingObject->LoadStreamImage( pic );
            if( !newElement.imageList.empty() )
              callingObject->m_StreamImageList.push_back( newElement );
          }
          else  //load ipPicDescriptors
            callingObject->m_ImageList.push_back( pic );

          if( remove(  pathAndFile.c_str() ) != 0 )
            std::cout << "LoadFromCHILI (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
        }
      }
      else
      {
        if( fileExtension == "dcm" )  //load dicom-files
        {
          ipUInt1_t* header = NULL;
          ipUInt4_t  header_size;
          ipUInt1_t* dcimage = NULL;
          ipUInt4_t  image_size;

          dicomGetHeaderAndImage( (char*)pathAndFile.c_str(), &header, &header_size, &dcimage, &image_size );

          if( header && dcimage )
          {
            //ipPicDescriptor *pic = dicomToPic( header, header_size, image, image_size );
            ipPicDescriptor *pic = _dicomToPic( header, header_size, dcimage, image_size, ipFalse, ipTrue, ipTrue);
            if( pic )
            {
              callingObject->m_ImageList.push_back( pic );
              if( remove(  pathAndFile.c_str() ) != 0 )
                std::cout << "LoadFromCHILI (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
            }
          }
          else std::cout<< "LoadFromCHILI (GlobalIterateLoadImage): Could not get header or image." <<std::endl;
        }
        else
          callingObject->m_UnknownImageFormatPath.push_back( pathAndFile );
      }
    }
    ProgressBar::GetInstance()->Progress();
  }
  return ipTrue; // true = next element; false = break iterate
}

#endif
