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

//MITK
#include "mitkPicDescriptorToNode.h"
#include "mitkImageNumberFilter.h"
#include "mitkSingleSpacingFilter.h"
#include "mitkSpacingSetFilter.h"
#include "mitkStreamReader.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkIpPic.h"
#include "mitkProgressBar.h"
#include "mitkIpPicUnmangle.h"
#include "mitkChiliPlugin.h"

//CHILI
#include <chili/plugin.h>
#include <chili/qclightbox.h>
#include <ipPic/ipPic.h>  //ipPicDescriptor
#include <ipPic/ipPicTags.h>
#include <ipDicom/ipDicom.h>  //read DICOM-Files

mitk::LoadFromCHILI::LoadFromCHILI()
:m_UsedReader(0),
 m_GroupByAcquisitionNumber(false)
{
}

mitk::LoadFromCHILI::~LoadFromCHILI()
{
}

void mitk::LoadFromCHILI::SetReaderType( unsigned int readerType )
{
  m_UsedReader = readerType;
}

std::vector<mitk::DataTreeNode::Pointer> 
mitk::LoadFromCHILI::LoadImagesFromLightbox( QcPlugin* instance, 
                                             QcLightbox* lightbox, 
                                             const std::string& tmpDirectory )
{
  // first just get images
  
  m_ImageList.clear();
  m_StreamImageList.clear();
  m_UnknownImageFormatPath.clear();

  if ( !lightbox->getFrames() )
  {
    return std::vector<DataTreeNode::Pointer>();
  }

  //read all frames from Lightbox
  ProgressBar::GetInstance()->AddStepsToDo( lightbox->getFrames() );
  for( unsigned int n = 0; n < lightbox->getFrames(); ++n )
  {
    // note difference between ipPicDescriptor and mitkIpPicDescriptor
    if( ipPicDescriptor* pic = lightbox->fetchPic(n) )
    {
      mitkIpPicDescriptor* mitkPic = reinterpret_cast<mitkIpPicDescriptor*>(pic);
      mitkIpPicDescriptor* currentPic = mitkIpPicClone( mitkPic );
      if( currentPic )
      {
        if( currentPic->dim == 1 )
        {
          StreamImageStruct newElement = LoadStreamImage( currentPic );
          if( !newElement.imageList.empty() )
          {
            m_StreamImageList.push_back( newElement );
          }
        }
        else
        {
          m_ImageList.push_back( currentPic );
        }
        // how about deleting currentPic?
        // Answer: this is done in CreateNodesFromLists
      }
    }
    ProgressBar::GetInstance()->Progress();
  }

  std::vector<mitk::DataTreeNode::Pointer> result = 
    CreateNodesFromLists( instance, lightbox->currentSeries()->instanceUID, tmpDirectory );

  // add text objects
  std::vector<mitk::DataTreeNode::Pointer> textResults = 
    LoadTextsFromSeries( instance, lightbox->currentSeries()->instanceUID, tmpDirectory );

  result.insert( result.end(), textResults.begin(), textResults.end() );

  return result;
}


mitk::LoadFromCHILI::StreamImageStruct mitk::LoadFromCHILI::LoadStreamImage( mitkIpPicDescriptor* pic)
{
#ifndef _WIN32
  StreamImageStruct newElement;
  newElement.imageList.clear();

  unsigned int frameNumber;
  mitkIpPicTSV_t *tsv;
  void* data = NULL;
  ipUInt4_t len = 0;
  tsv = mitkIpPicQueryTag( pic, (char*)"SOURCE HEADER" );
  if( tsv )
  {
    if( dicomFindElement( (unsigned char*) tsv->value, 0x0028, 0x0008, &data, &len ) && data )
    {
      ProgressBar::GetInstance()->AddStepsToDo( 2 );
      sscanf( (char *) data, "%d", &frameNumber );

      ipUInt4_t *offset_table;
      offset_table = (ipUInt4_t *)malloc( sizeof(ipUInt4_t) * frameNumber );
      for( unsigned int i = 0; i < frameNumber; ++i )
      {
        offset_table[i] = 0xffffffff;
      }

      for( unsigned int i = 0; i < frameNumber; i++ )
      {
        ipPicDescriptor* chiliPic = ipPicClone(reinterpret_cast<ipPicDescriptor*>(pic));
        ipPicDescriptor* cinePic = pipPicDecompressJPEG( chiliPic, i, frameNumber, NULL, offset_table );
        mitkIpPicDescriptor *mitkCinePic = mitkIpPicClone( reinterpret_cast<mitkIpPicDescriptor*>(cinePic) );
        ipPicFree( chiliPic );
        newElement.imageList.push_back( mitkCinePic );
      }
      free( offset_table );
      ProgressBar::GetInstance()->Progress();

      //interSliceGeometry
      ipPicDescriptor* chiliPic = reinterpret_cast<ipPicDescriptor*>(pic);
      newElement.geometry = ( interSliceGeometry_t* ) malloc ( sizeof( interSliceGeometry_t ) );
      if( !chiliPic || !pFetchSliceGeometryFromPic( chiliPic, newElement.geometry ) )
      {
        std::cout<<"Not able to load interSliceGeometry!"<<std::endl;
        free( newElement.geometry );
      }

      //SeriesDescription
      mitkIpPicTSV_t* seriesDescriptionTag = mitkIpPicQueryTag( pic, (char*)tagSERIES_DESCRIPTION );
      if( seriesDescriptionTag )
      {
        newElement.seriesDescription = static_cast<char*>( seriesDescriptionTag->value );
      }

      if( newElement.seriesDescription == "" )
      {
        mitkIpPicTSV_t *tsv;
        void* data = NULL;
        ipUInt4_t len = 0;
        tsv = mitkIpPicQueryTag( pic, (char*)"SOURCE HEADER" );
        if( tsv && dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
        {
          newElement.seriesDescription = (char*)data;
        }
      }
      ProgressBar::GetInstance()->Progress();
    }
  }

  return newElement;

#else
StreamImageStruct dummy;
return dummy;
#endif
}

std::vector<mitk::DataTreeNode::Pointer> 
mitk::LoadFromCHILI::CreateNodesFromLists( QcPlugin* itkNotUsed(instance), 
                                           const std::string& seriesInstanceUID, 
                                           const std::string& itkNotUsed(tmpDirectory) )
{  //hint: text-files handled separately
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

  // process several kinds of images:
  //   m_ImageList contains "normal" PIC images, will be sorted into images volumes by a class of type PicDescriptorToNode
  //   m_StreamImageList contains JPEG streams. We need CHILI to decode these kind of files, so they are treated specially
  //   m_UnknownImageFormatPath contains other types of images. We try to load these through the MITK DataTreeNodeFactory
  
  if( !m_ImageList.empty() )
  {
    // PicDescriptorToNode is one of those classes
    // that sort 2D images into 3D or 3D+t image volumes
    PicDescriptorToNode::Pointer converterToNode;
    switch ( m_UsedReader )
    {
      case 0:
      {
        ImageNumberFilter::Pointer imageNumberFilter = ImageNumberFilter::New();
        imageNumberFilter->SetGroupByAcquisitionNumber( m_GroupByAcquisitionNumber );
        converterToNode = imageNumberFilter;
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
      }
    }

    if ( converterToNode.IsNotNull() )
    {
      converterToNode->SetInput( m_ImageList, seriesInstanceUID );
      converterToNode->Update();
      resultNodes = converterToNode->GetOutput();
    }

    for( std::list<mitkIpPicDescriptor*>::iterator imageIter = m_ImageList.begin(); 
         imageIter != m_ImageList.end(); 
         ++imageIter )
    {
      mitkIpPicFree( (*imageIter) );
    }

    m_ImageList.clear();
  }

  if( !m_StreamImageList.empty() )
  {
    StreamReader::Pointer streamReader = StreamReader::New();
    for ( std::list<StreamImageStruct>::iterator iter = m_StreamImageList.begin();
          iter != m_StreamImageList.end();
          ++iter )
    {
      streamReader->SetInput( iter->imageList, seriesInstanceUID );
      streamReader->SetSecondInput( iter->geometry, iter->seriesDescription );
      streamReader->Update();
      resultNodes.push_back( streamReader->GetOutput()[0] );

      //delete the mitkIpPicDescriptors and interSliceGeometry
      for( std::list<mitkIpPicDescriptor*>::iterator imageIter = iter->imageList.begin();
           imageIter != iter->imageList.end(); 
           ++imageIter )
      {
        mitkIpPicFree( (*imageIter) );
      }

      free( iter->geometry );
    }
  }

  if( !m_UnknownImageFormatPath.empty() )
  {
    DataTreeNodeFactory::Pointer factory = DataTreeNodeFactory::New();
    try
    {
      for ( std::list<std::string>::iterator iter = m_UnknownImageFormatPath.begin();
            iter != m_UnknownImageFormatPath.end();
            ++iter )
      {
        factory->SetFileName( iter->c_str() );
        factory->Update();
        for( unsigned int i = 0 ; i < factory->GetNumberOfOutputs(); ++i )
        {
          DataTreeNode::Pointer node = factory->GetOutput( i );
          if ( node.IsNotNull() && node->GetData()  )
          {
            node->SetProperty( "SeriesInstanceUID", StringProperty::New( seriesInstanceUID ) );
            resultNodes.push_back( node );
          }
        }

        if( (factory->GetNumberOfOutputs() > 0) && remove( iter->c_str() ) != 0)
        {
          std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << iter->c_str() << std::endl;
        }
      }
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open: " << ex );
    }
  }

  return resultNodes;
}

std::vector<mitk::DataTreeNode::Pointer> mitk::LoadFromCHILI::LoadImagesFromSeries( QcPlugin* instance, 
                                                                                    const std::string& seriesInstanceUID, 
                                                                                    const std::string& tmpDirectory )
{
  std::vector<std::string> uids;
  uids.push_back( seriesInstanceUID );

  return LoadImagesFromSeries( instance, uids, tmpDirectory );
}


std::vector<mitk::DataTreeNode::Pointer> mitk::LoadFromCHILI::LoadImagesFromSeries( QcPlugin* instance, 
                                                                                    std::vector<std::string> seriesInstanceUIDs, 
                                                                                    const std::string& tmpDirectory )
{
  if ( seriesInstanceUIDs.empty() )
  {
    std::vector<mitk::DataTreeNode::Pointer> empty;
    empty.clear();
    return empty;
  }

  m_ImageList.clear();
  m_UnknownImageFormatPath.clear();
  m_StreamImageList.clear();

  //iterate over all images in this series
  //  save to local files
  //  load from local files 
  //  add the 
  //    "normal" picDescriptors to m_ImageList
  //    jpeg compressed streams to m_StreamImageList
  //    unknown imagefiles to unknownImageFormatPath
  m_tmpDirectory = tmpDirectory;
  
  for ( std::vector<std::string>::iterator iter = seriesInstanceUIDs.begin();
        iter != seriesInstanceUIDs.end();
        ++iter )
  {
    std::string seriesOID = ChiliPlugin::GetChiliPluginInstance()->GetSeriesOIDFromInstanceUID( *iter );
    pIterateImages( instance, (char*)seriesOID.c_str(), NULL, &LoadFromCHILI::GlobalIterateLoadImages, this );
  }

  return CreateNodesFromLists( instance, seriesInstanceUIDs.front(), tmpDirectory ); // use first series instance uid for datatreenode property
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

  //save to local file
  ipInt4_t error;
  pFetchDataToFile( image->path, pathAndFile.c_str(), &error );
  ProgressBar::GetInstance()->Progress();

  if( error )
  {
    std::cout << "LoadFromCHILI (GlobalIterateLoadImage): "
                 "ChiliError: " << error << 
                  ", while reading file (" << image->path << 
                  ") from Database." << std::endl;
    ProgressBar::GetInstance()->Progress();
    return ipTrue;
  }

  if( fileExtension == "pic" )
  {
    if( mitkIpPicDescriptor* pic = mitkIpPicGet( (char*)pathAndFile.c_str(), NULL ) )
    {
      if( pic->dim == 1 )  //load cine-pics
      {
        StreamImageStruct newElement = callingObject->LoadStreamImage( pic );
        if( !newElement.imageList.empty() )
        {
          callingObject->m_StreamImageList.push_back( newElement );
        }
      }
      else  //load "normal" mitkIpPicDescriptors
      {
        callingObject->m_ImageList.push_back( pic );
      }

      if( remove( pathAndFile.c_str() ) != 0 )
      {
        std::cout << "LoadFromCHILI (GlobalIterateLoadImage): "
                     "Not able to  delete file: " << pathAndFile << std::endl;
      }
    }
  }
  else if( fileExtension == "dcm" )  //load dicom-files
  {
    ipUInt1_t* header = NULL;
    ipUInt4_t  header_size;
    ipUInt1_t* dcimage = NULL;
    ipUInt4_t  image_size;

    dicomGetHeaderAndImage( (char*)pathAndFile.c_str(), &header, &header_size, &dcimage, &image_size );

    if( header && dcimage )
    {
      //mitkIpPicDescriptor *pic = dicomToPic( header, header_size, image, image_size );
      ipPicDescriptor *pic = _dicomToPic( header, header_size, dcimage, image_size, ipFalse, ipTrue, ipTrue);
      mitkIpPicDescriptor *currentPic = mitkIpPicClone( reinterpret_cast<mitkIpPicDescriptor*>(pic) );
      ipPicFree( pic );
      if( currentPic )
      {
        callingObject->m_ImageList.push_back( currentPic );
        if( remove(  pathAndFile.c_str() ) != 0 )
        {
          std::cout << "LoadFromCHILI (GlobalIterateLoadImage): "
                       "Not able to  delete file: "<< pathAndFile << std::endl;
        }
      }
    }
    else 
    {
      std::cout<< "LoadFromCHILI (GlobalIterateLoadImage): "
                  "Could not get header or image." <<std::endl;
    }
  }
  else
  {
    callingObject->m_UnknownImageFormatPath.push_back( pathAndFile );
  }
  
  ProgressBar::GetInstance()->Progress();

  return ipTrue; // true = next element; false = break iterate
}

std::vector<mitk::DataTreeNode::Pointer> 
mitk::LoadFromCHILI::LoadTextsFromSeries( QcPlugin* instance, 
                                          const std::string& seriesInstanceUID, 
                                          const std::string& tmpDirectory )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

  std::string seriesOID = ChiliPlugin::GetChiliPluginInstance()->GetSeriesOIDFromInstanceUID( seriesInstanceUID );
  m_TextList.clear();
  m_TextListCounter = 0;
  m_SeriesInstanceUID = seriesInstanceUID;
  pIterateTexts( instance, (char*)seriesOID.c_str(), NULL, &LoadFromCHILI::GlobalIterateLoadTexts, this );

  for ( std::list<TextFilePathAndOIDStruct>::iterator iter = m_TextList.begin();
        iter != m_TextList.end();
        ++iter )
  {
    DataTreeNode::Pointer tempNode = LoadSingleText( instance, 
                                                     seriesInstanceUID, 
                                                     iter->instanceNumber, 
                                                     iter->TextFilePath, 
                                                     tmpDirectory );
    if( tempNode.IsNotNull() )
    {
      resultNodes.push_back( tempNode );
    }
  }

  return resultNodes;
}

ipBool_t mitk::LoadFromCHILI::GlobalIterateLoadTexts( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  LoadFromCHILI* callingObject = static_cast<LoadFromCHILI*>(user_data);

  //we dont want that to load the parent-child-relation-file
  std::string chiliDatabaseName = text->chiliText;
  std::string fileName = chiliDatabaseName.substr( chiliDatabaseName.find_last_of("-") + 1 );

  LoadFromCHILI::TextFilePathAndOIDStruct newTextFile;
  newTextFile.instanceNumber = callingObject->m_TextListCounter++; // increase afterwards! start counting from 0
  newTextFile.TextFilePath = text->chiliText;
  newTextFile.TextFileOID = text->oid;
  ChiliPlugin::GetChiliPluginInstance()->UpdateTextOIDFromSeriesInstanceUIDAndInstanceNumber( text->oid, 
                                                                                              callingObject->m_SeriesInstanceUID, 
                                                                                              newTextFile.instanceNumber );
  callingObject->m_TextList.push_back( newTextFile );
  
  return ipTrue; // true = next element; false = break iterate
}

mitk::DataTreeNode::Pointer 
mitk::LoadFromCHILI::LoadSingleText( QcPlugin* instance, 
                                     const std::string& seriesInstanceUID, 
                                     unsigned int instanceNumber, 
                                     const std::string& tmpDirectory )
{
  DataTreeNode::Pointer resultNode;

  text_t text;
  series_t series;
  initTextStruct( &text );
  initSeriesStruct( &series );
  std::string textOID = 
    ChiliPlugin::GetChiliPluginInstance()->GetTextOIDFromSeriesInstanceUIDAndInstanceNumber( seriesInstanceUID, instanceNumber );
  
  text.oid = strdup( textOID.c_str() );

  if( pQueryText( instance, &text, &series, NULL, NULL ) )
  {
    resultNode = LoadSingleText( instance, seriesInstanceUID, instanceNumber, text.chiliText, tmpDirectory );
  }
  else
  {
    std::cout << "LoadFromCHILI (LoadSingleText): pQueryText() failed. Abort." << std::endl;
  }

  clearTextStruct( &text );
  clearSeriesStruct( &series );

  return resultNode;
}

mitk::DataTreeNode::Pointer 
mitk::LoadFromCHILI::LoadSingleText( QcPlugin* itkNotUsed(instance), 
                                     const std::string& seriesInstanceUID, 
                                     unsigned int instanceNumber, 
                                     const std::string& textPath, 
                                     const std::string& tmpDirectory )
{
  std::string textOID = 
    ChiliPlugin::GetChiliPluginInstance()->GetTextOIDFromSeriesInstanceUIDAndInstanceNumber( seriesInstanceUID, instanceNumber );

  ProgressBar::GetInstance()->AddStepsToDo( 2 );
  DataTreeNode::Pointer resultNode;
  //get Filename and path to save to harddisk
  std::string fileName = textPath.substr( textPath.find_last_of("-") + 1 );
  std::string pathAndFile = tmpDirectory + fileName;

  //Load File from DB
  ipInt4_t error;
  pFetchDataToFile( textPath.c_str(), pathAndFile.c_str(), &error );
  ProgressBar::GetInstance()->Progress();
  if( error != 0 )
  {
    std::cout << "LoadFromCHILI (LoadSingleText): ChiliError: " << error << 
                 ", while reading file (" << fileName << 
                 ") from Database." << std::endl;
    ProgressBar::GetInstance()->Progress();
    return NULL;
  }
    
  //if there are no fileextension in the filename, the reader not able to load the file and the plugin crashed
  if( fileName.find_last_of(".") == std::string::npos )
  {
    std::cout << "LoadFromCHILI (LoadSingleText): "
                 "Reader not able to read file without extension.\n"
                  "If you dont close CHILI you can find the file here: " << pathAndFile << std::endl;
  }
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
        if ( resultNode.IsNotNull() && resultNode->GetData() != NULL )
        {
          //get the FileExtension and cut them from the filename
          std::string fileNameWithoutExtension = fileName.substr( 0, fileName.find_last_of(".") );
          
          //set the filename without extension as name-property
          // TODO "name" not better description from TextInformation?
          resultNode->SetProperty( "name", StringProperty::New( fileNameWithoutExtension ) ); 
          resultNode->SetProperty( "TextOID", StringProperty::New( textOID ) );
          resultNode->SetProperty( "SeriesInstanceUID", StringProperty::New( seriesInstanceUID ) );

          //it should be possible to override all non-image-entries
          resultNode->SetProperty( "CHILI: MANUFACTURER", StringProperty::New( "MITK" ) );
          resultNode->SetProperty( "CHILI: INSTITUTION NAME", StringProperty::New( "DKFZ.MBI" ) );

          if( remove(  pathAndFile.c_str() ) != 0 )
          {
            std::cout << "LoadFromCHILI (LoadSingleText): "
                         "Not able to  delete file: " << pathAndFile << std::endl;
          }
        }
      }
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open: " << ex );
    }
  }

  ProgressBar::GetInstance()->Progress();
  return resultNode;
}

void mitk::LoadFromCHILI::SetSeparateByAcquisitionNumber(bool on)
{
  m_GroupByAcquisitionNumber = on;
}

