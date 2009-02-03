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

#include "mitkSaveToCHILI.h"

#include "mitkIpPicUnmangle.h"

//CHILI
#include <chili/plugin.h>
#include <ipPic/ipPicTags.h>
#include <ipPic/ipPic.h>

//MITK
#include "mitkChiliPlugin.h"
#include "mitkLevelWindowProperty.h"
#include "mitkFileWriter.h"
#include "mitkProgressBar.h"
#include "mitkIpPicUnmangle.h"

//Qt
#include <qapplication.h>
#include <qcursor.h>

mitk::SaveToCHILI::SaveToCHILI()
{
}

mitk::SaveToCHILI::~SaveToCHILI()
{
}

void mitk::SaveToCHILI::SaveAsNewSeries( QcPlugin* instance, 
                                         DataStorage::SetOfObjects::ConstPointer inputNodes, 
                                         const std::string& studyInstanceUID, 
                                         int seriesNumber, 
                                         const std::string& seriesDescription, 
                                         const std::string& tmpDirectory )
{
  std::string newSeriesDescription( seriesDescription );
  if ( seriesDescription.empty() )
  {
    for ( DataStorage::SetOfObjects::const_iterator iter = inputNodes->begin();
          iter != inputNodes->end();
          ++iter )
    {
      const DataTreeNode* node = iter->GetPointer();
      if (node)
      {
        node->GetStringProperty("name", newSeriesDescription);
      }
    }
  }

  if (newSeriesDescription.empty()) newSeriesDescription = "Never save objects without a name :-P";

  std::cout << "Request to save " << inputNodes->size() 
            << " nodes as series " << seriesDescription 
            << " defaulting to series description " << newSeriesDescription 
            << "' to study " << studyInstanceUID 
            << " with seriesNumber " << seriesNumber << std::endl;
  //CHILI-Version >= 3.12 ???
  #if CHILI_VERSION_CODE( 1, 1, 3 ) < CHILI_PLUGIN_VERSION_CODE

  std::string studyOID = ChiliPlugin::GetChiliPluginInstance()->GetStudyOIDFromInstanceUID( studyInstanceUID );

  study_t study;
  initStudyStruct( &study );
  study.oid = strdup( studyOID.c_str() );

  if( pQueryStudy( instance, &study, NULL ) )  //test if study exist
  {
    //create new series
    series_t* newSeries = ( series_t* )malloc( sizeof( series_t ) );
    initSeriesStruct( newSeries );
    newSeries->description = (char*)newSeriesDescription.c_str();
    newSeries->number = seriesNumber;
    if( pCreateSeries( instance, &study, newSeries ) )
    {
      ChiliPlugin::GetChiliPluginInstance()->UpdateSeriesOIDForInstanceUID( newSeries->oid, newSeries->instanceUID );
std::cout << " **** Created new series UID " << newSeries->instanceUID << std::endl;
      SaveToSeries( instance, inputNodes, newSeries->instanceUID, false, tmpDirectory );
    }
    else
    {
      std::cout << "SaveToCHILI (SaveAsNewSeries): Can not create a new Series." << std::endl;
    }
    
    free( newSeries );
  }
  else
  {
    std::cout << "SaveToCHILI (SaveAsNewSeries): Study not exist. Abort." << std::endl;
  }

  clearStudyStruct( &study );
  #endif
}

void mitk::SaveToCHILI::SaveToSeries( QcPlugin* instance, 
                                      DataStorage::SetOfObjects::ConstPointer inputNodes, 
                                      const std::string& seriesInstanceUID, 
                                      bool overrideExistingSeries, 
                                      const std::string& tmpDirectory )
{
  std::cout << "Request to save " << inputNodes->size() 
            << "nodes to series " << seriesInstanceUID << std::endl;
#if CHILI_PLUGIN_VERSION_CODE > CHILI_VERSION_CODE( 1, 1, 3 ) //CHILI-Version >= 3.12 ???

  study_t study;
  patient_t patient;
  series_t series;
  initStudyStruct( &study );
  initPatientStruct( &patient );
  initSeriesStruct( &series );
  
  std::string seriesOID = ChiliPlugin::GetChiliPluginInstance()->GetSeriesOIDFromInstanceUID( seriesInstanceUID );

  series.oid = strdup( seriesOID.c_str() );
  if( !pQuerySeries( instance, &series, &study, &patient ) )
  {
    std::cout << "SaveToCHILI (SaveToSeries): Series not exist. Abort." << std::endl;
    clearStudyStruct( &study );
    clearPatientStruct( &patient );
    clearSeriesStruct( &series );
    return;
  }

  //get current highest imagenumber
  unsigned int newImageNumber = GetMaximumImageNumber( instance, seriesOID ) + 1;

  //get the needed study-, patient- and seriesinformation to save
  ImageToPicDescriptor::TagInformationList picTagList = GetNeededTagList( &study, &patient, &series );

  ProgressBar::GetInstance()->AddStepsToDo( inputNodes->size() );

  for( DataStorage::SetOfObjects::const_iterator inputNodeIterator = inputNodes->begin(); 
      inputNodeIterator != inputNodes->end(); 
      ++inputNodeIterator )
  {
    DataTreeNode::Pointer node = *inputNodeIterator;
    if ( node.IsNull() ) continue;
      
    BaseData::Pointer data = node->GetData();
    if ( data.IsNull() ) continue;

    std::string currentNodesSeriesInstanceUID;
    node->GetStringProperty( "SeriesInstanceUID", currentNodesSeriesInstanceUID ); // TODO probably change to instance UID

    if ( Image::Pointer image = dynamic_cast<Image*>( data.GetPointer() ) ) // save image
    {
      // check if we are asked to save into a series that was loaded from CHILI (probably like overwriting)
      if( currentNodesSeriesInstanceUID == seriesInstanceUID )  
      {
        if( !overrideExistingSeries ) continue;

        BaseProperty::Pointer manufacturerProperty = node->GetProperty( "CHILI: MANUFACTURER" );
        BaseProperty::Pointer institutionNameProperty = node->GetProperty( "CHILI: INSTITUTION NAME" );
        
        //only data with "MANUFACTURER==MITK" and "INSTITUTION NAME==DKFZ.MBI" should be overwritten
        if( manufacturerProperty.IsNull() || 
            institutionNameProperty.IsNull() || 
            manufacturerProperty->GetValueAsString() != "MITK" || 
            institutionNameProperty->GetValueAsString() != "DKFZ.MBI" )
        {
          continue;
        }
      }

      // convert Image to ipPicDescriptor
      ImageToPicDescriptor::Pointer converterToDescriptor = ImageToPicDescriptor::New();
      converterToDescriptor->SetTempDirectory( tmpDirectory );
      LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<LevelWindowProperty*>( node->GetProperty("levelwindow") );
      if( levelWindowProperty.IsNotNull() )
        converterToDescriptor->SetLevelWindow( levelWindowProperty->GetLevelWindow() );
      converterToDescriptor->SetImage( image );
      converterToDescriptor->SetImageNumber( newImageNumber );

      // add the node-name as SeriesDescription to the Taglist (get saved as SeriesDescription)
      BaseProperty::Pointer nameProperty = node->GetProperty( "name" );
      ImageToPicDescriptor::TagInformationStruct temp;
      temp.PicTagDescription = tagSERIES_DESCRIPTION;
      if( nameProperty.IsNotNull() )
        temp.PicTagContent = nameProperty->GetValueAsString();
      else temp.PicTagContent = "no Description";
std::cout << " saving image " << temp.PicTagContent << std::endl;
      picTagList.push_back( temp );

      // to overwrite images, set useSavedPicTags == true
      converterToDescriptor->SetTagList( picTagList, 
                                         overrideExistingSeries 
                                         && (currentNodesSeriesInstanceUID == seriesInstanceUID)
                                       );

      //create the picdescriptorlist
      converterToDescriptor->Update();
      std::list< ipPicDescriptor* > myPicDescriptorList = converterToDescriptor->GetOutput();
      std::cout << " Generated " << myPicDescriptorList.size() << " from image volume." << std::endl;

      //increase the imageNumber
      newImageNumber = newImageNumber + myPicDescriptorList.size();
      
      //delete the current node-name from the picTagList (added as SeriesDescription but the picTagList get used for the next image too)
      picTagList.pop_back();

      //save the single slices to CHILI
      pOpenAssociation();

      int count = 0;
      ProgressBar::GetInstance()->AddStepsToDo( myPicDescriptorList.size() );
      for ( std::list< ipPicDescriptor* >::iterator picIterator = myPicDescriptorList.begin();
            picIterator != myPicDescriptorList.end();
            ++picIterator, ++count )
      {
        std::ostringstream helpFileName;
        if( node->GetProperty( "name" ) && !node->GetProperty( "name" )->GetValueAsString().empty() )
        {
          helpFileName << node->GetProperty( "name" )->GetValueAsString();
        }
        else 
        {
          helpFileName << "FileUpload";
        }
        std::cout << "   storing " << helpFileName << " to PACS" << std::endl;
        helpFileName << count << ".pic";
        ++count;

        std::string fileName = helpFileName.str();
        std::string pathAndFile = tmpDirectory + fileName;
        ipPicPut( (char*)pathAndFile.c_str(), *picIterator );
        ipPicFree( *picIterator );
        
        if( !pStoreDataFromFile( pathAndFile.c_str(), 
                                 fileName.c_str(), 
                                 NULL, 
                                 NULL, 
                                 study.instanceUID, 
                                 patient.oid, 
                                 study.oid, 
                                 series.oid, 
                                 NULL ) 
            )
        {
          std::cout << "ChiliPlugin (SaveToChili): "
                       "Error while saving File (" << fileName << 
                        ") to Database." << std::endl;
        }
        else if( remove( pathAndFile.c_str() ) != 0 )
        {
          std::cout << "ChiliPlugin (SaveToChili): "
                       "Not able to  delete file: " << pathAndFile << std::endl;
        }

        ProgressBar::GetInstance()->Progress();
      }

      pCloseAssociation();
      //set the new SERIESOID as Property
      if( currentNodesSeriesInstanceUID.empty() )
      {
        node->SetProperty( "SeriesInstanceUID", StringProperty::New( seriesInstanceUID ) );
      }
      else
      {
        node->ReplaceProperty( "SeriesInstanceUID", StringProperty::New( seriesInstanceUID ) );
      }
    }
    else  //SAVE TEXT
    {
      //first: check the TextOID
      std::string textOID;
      BaseProperty::Pointer currentNodesTextOID = node->GetProperty( "TextOID" );
      if( currentNodesTextOID.IsNotNull() )
      {
        //get the SeriesOID of the Text-File
        text_t text;
        series_t series;
        initTextStruct( &text );
        initSeriesStruct( &series );

        text.oid = strdup( currentNodesTextOID->GetValueAsString().c_str() );
        if( pQueryText( instance, &text, &series, NULL, NULL ) )
        {
          if ( seriesOID == series.oid )
          {
            if( overrideExistingSeries )  //to override the OIDs have to be the same
            {
              textOID = currentNodesTextOID->GetValueAsString().c_str();
            }
            else  //the Text exist in current series and entries should not be override
            {
              clearTextStruct( &text );
              clearSeriesStruct( &series );
              continue;
            }
          }
        }
        clearTextStruct( &text );
        clearSeriesStruct( &series );
      }
        
      textOID = pGetNewOID();

      //search for possible Writer
      std::list<FileWriter::Pointer> possibleWriter;
      std::list<LightObject::Pointer> allObjects = itk::ObjectFactoryBase::CreateAllInstance( "IOWriter" );
      for( std::list<LightObject::Pointer>::iterator iter = allObjects.begin(); 
           iter != allObjects.end(); 
           ++iter)
      {
        FileWriter* writer = dynamic_cast<FileWriter*>( iter->GetPointer() );
        if( writer )
        {
          possibleWriter.push_back( writer );
        }
        else 
        {
          std::cout << "SaveToCHILI (SaveToChili): no FileWriter returned" << std::endl;
        }
      }

      //use Writer
      for( std::list<FileWriter::Pointer>::iterator writerIter = possibleWriter.begin(); 
           writerIter != possibleWriter.end(); 
           ++writerIter )
      {
        FileWriter* writer = *writerIter;
        if( !writer->CanWriteDataType( node ) ) continue;
          
        ProgressBar::GetInstance()->AddStepsToDo(1);
        
        //create filename
        std::string fileName;
        if( node->GetProperty( "name" ) && !node->GetProperty( "name" )->GetValueAsString().empty() )
        {
          fileName = node->GetProperty( "name" )->GetValueAsString();
        }
        else 
        {
          fileName = "FileUpload";
        }
        //set extension
        fileName = fileName + writer->GetFileExtension();
        //path and filename
        std::string pathAndFile = tmpDirectory + fileName;

        //save via Writer to harddisk
        writer->SetFileName( pathAndFile.c_str() );
        writer->SetInput( node );
        writer->Write();

        //save file from harddisk to chili
        if( !pStoreDataFromFile( pathAndFile.c_str(), 
                                 fileName.c_str(), 
                                 writer->GetWritenMIMEType().c_str(), 
                                 NULL, 
                                 study.instanceUID, 
                                 patient.oid, 
                                 study.oid, 
                                 series.oid, 
                                 textOID.c_str() ) )
        {
          std::cout << "SaveToCHILI (SaveToChili): "
                       "Error while saving File (" << fileName << ") to Database." << std::endl;
        }
        else if( remove(  pathAndFile.c_str() ) != 0 )
        {
          std::cout << "SaveToCHILI (SaveToChili): "
                       "Not able to  delete file: " << writer->GetFileName() << std::endl;
        }

        //set the instance UIDs as Property
        if( currentNodesSeriesInstanceUID == "" )
        {
          node->SetProperty( "SeriesInstanceUID", StringProperty::New( seriesInstanceUID ) );
        }
        else
        {
          node->ReplaceProperty( "SeriesInstanceUID", StringProperty::New( seriesInstanceUID ) );
        }

        if( currentNodesTextOID.IsNull() )
        {
          node->SetProperty( "TextOID", StringProperty::New( textOID ) );
        }
        else
        {
          node->ReplaceProperty( "TextOID", StringProperty::New( textOID ) );
        }
        
        ProgressBar::GetInstance()->Progress();

        break; // only one writer should write a file (lucky first one)
      }
    }

    ProgressBar::GetInstance()->Progress();
  } // for all input nodes

  clearStudyStruct( &study );
  clearPatientStruct( &patient );
  clearSeriesStruct( &series );
  #endif
}

mitk::ImageToPicDescriptor::TagInformationList mitk::SaveToCHILI::GetNeededTagList( study_t* study, patient_t* patient, series_t* series )
{
  ImageToPicDescriptor::TagInformationList resultList;
  resultList.clear();

  //get all study- and patient-information
  if( study && patient && series )
  {
    typedef ImageToPicDescriptor::TagInformationStruct Entry;
    //study
    resultList.push_back( Entry( tagSTUDY_INSTANCE_UID, study->instanceUID ) );
    resultList.push_back( Entry( tagSTUDY_DESCRIPTION,  study->description ) );
    resultList.push_back( Entry( tagSTUDY_DATE,         study->date ) );
    resultList.push_back( Entry( tagSTUDY_TIME,         study->time ) );
    resultList.push_back( Entry( tagSTUDY_ID,           study->id ) );

    //self created
    resultList.push_back( Entry( tagMANUFACTURER,       "MITK" ) );
    resultList.push_back( Entry( tagINSTITUTION_NAME,   "DKFZ.MBI" ) );
    resultList.push_back( Entry( tagMODALITY,           std::string(study->modality) + " processed" ) );
    
    //series
    resultList.push_back( Entry( tagSERIES_INSTANCE_UID,series->instanceUID ) );
    resultList.push_back( Entry( tagSERIES_DATE,series->date ) );
    resultList.push_back( Entry( tagSERIES_TIME,series->time ) );
    std::stringstream s;
    s << series->number;
    resultList.push_back( Entry( tagSERIES_NUMBER, s.str()) );
  }
  return resultList;
}

unsigned int mitk::SaveToCHILI::GetMaximumImageNumber( QcPlugin* instance, const std::string& seriesOID )
{
  m_MaximumImageNumber = -1;
  pIterateImages( instance, (char*)seriesOID.c_str(), NULL, &SaveToCHILI::GlobalIterateImagesForImageNumber, this );
  return m_MaximumImageNumber;
}

ipBool_t mitk::SaveToCHILI::GlobalIterateImagesForImageNumber( int /*rows*/, int /*row*/, image_t* image, void* user_data )
{
  SaveToCHILI* callingObject = static_cast<SaveToCHILI*>( user_data );

  if( image->number > callingObject->m_MaximumImageNumber )
  {
    callingObject->m_MaximumImageNumber = image->number;
  }

  return ipTrue; // true = next element; false = break iterate
}

