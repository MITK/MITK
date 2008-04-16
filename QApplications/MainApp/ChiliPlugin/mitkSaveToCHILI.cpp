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

//CHILI
#include <chili/plugin.h>
#include <ipPic/ipPicTags.h>  //ipPicTags
//MITK
#include "mitkPACSPlugin.h"
#include "mitkLevelWindowProperty.h"
#include "mitkFileWriter.h"
#include "QmitkChiliPluginSaveDialog.h"
#include "mitkProgressBar.h"

//Qt
#include <qapplication.h>
#include <qcursor.h>

#ifdef CHILI_PLUGIN_VERSION_CODE

mitk::SaveToCHILI::SaveToCHILI()
{
    m_ParentChild = ParentChild::New();
}

mitk::SaveToCHILI::~SaveToCHILI()
{
}

void mitk::SaveToCHILI::SaveToChili( QcPlugin* instance, DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& tmpDirectory )
{
  //CHILI-Version >= 3.12 ???
  #if CHILI_VERSION_CODE( 1, 1, 3 ) < CHILI_PLUGIN_VERSION_CODE

  QmitkChiliPluginSaveDialog chiliPluginDialog( 0 );

  //add all nodes to the dialog
  for( DataStorage::SetOfObjects::const_iterator nodeIter = inputNodes->begin(); nodeIter != inputNodes->end(); nodeIter++ )
  {
    if( (*nodeIter) )
    {
      BaseProperty::Pointer seriesOIDProperty = (*nodeIter)->GetProperty( "SeriesOID" );

      if( seriesOIDProperty.IsNotNull() )  //nodes with oid loaded from CHILI
      {
        PACSPlugin::PatientInformation tempPatient = PACSPlugin::GetInstance()->GetPatientInformation( seriesOIDProperty->GetValueAsString() );
        PACSPlugin::StudyInformation tempStudy = PACSPlugin::GetInstance()->GetStudyInformation( seriesOIDProperty->GetValueAsString() );
        PACSPlugin::SeriesInformation tempSeries = PACSPlugin::GetInstance()->GetSeriesInformation( seriesOIDProperty->GetValueAsString() );

        if( tempPatient.OID == "" || tempStudy.OID == "" || tempSeries.OID == "" )  //no information found
          chiliPluginDialog.AddNode( (*nodeIter) );
        else  //add study-, patient- and series-information
          chiliPluginDialog.AddStudySeriesAndNode( tempStudy.OID, tempPatient.Name, tempPatient.ID, tempStudy.Description, tempSeries.OID, tempSeries.Number, tempSeries.Description, (*nodeIter) );
      }
      else  //saved first time
        chiliPluginDialog.AddNode( (*nodeIter) );
    }
  }

  //add the current selected study/patient/series
  PACSPlugin::PatientInformation currentselectedPatient = PACSPlugin::GetInstance()->GetPatientInformation();
  PACSPlugin::StudyInformation currentselectedStudy = PACSPlugin::GetInstance()->GetStudyInformation();
  PACSPlugin::SeriesInformation currentselectedSeries = PACSPlugin::GetInstance()->GetSeriesInformation();
  if( currentselectedSeries.OID == "" )
  {
    chiliPluginDialog.AddStudy( currentselectedStudy.OID, currentselectedPatient.Name, currentselectedPatient.ID, currentselectedStudy.Description );
  }
  else
    chiliPluginDialog.AddStudyAndSeries( currentselectedStudy.OID, currentselectedPatient.Name, currentselectedPatient.ID, currentselectedStudy.Description, currentselectedSeries.OID, currentselectedSeries.Number, currentselectedSeries.Description );

  //show dialog
  chiliPluginDialog.UpdateView();
  QApplication::restoreOverrideCursor();
  int dialogReturnValue = chiliPluginDialog.exec();

  if( dialogReturnValue == QDialog::Rejected ) return; //user abort

  bool override;
  if( chiliPluginDialog.GetSelection().UserDecision == QmitkChiliPluginSaveDialog::OverrideAll )
    override = true;
  else
    override = false;

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  if( chiliPluginDialog.GetSelection().UserDecision == QmitkChiliPluginSaveDialog::CreateNew )
    SaveAsNewSeries( instance, inputNodes, chiliPluginDialog.GetSelection().StudyOID, chiliPluginDialog.GetSeriesInformation().SeriesNumber, chiliPluginDialog.GetSeriesInformation().SeriesDescription, tmpDirectory );
  else
      SaveToSeries( instance, inputNodes, chiliPluginDialog.GetSelection().SeriesOID, override, tmpDirectory );
  #endif
}

void mitk::SaveToCHILI::SaveAsNewSeries( QcPlugin* instance, DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& studyOID, int seriesNumber, const std::string& seriesDescription, const std::string& tmpDirectory )
{
  //CHILI-Version >= 3.12 ???
  #if CHILI_VERSION_CODE( 1, 1, 3 ) < CHILI_PLUGIN_VERSION_CODE

  study_t study;
  initStudyStruct( &study );
  study.oid = strdup( studyOID.c_str() );

  if( pQueryStudy( instance, &study, NULL ) )  //test if study exist
  {
    //create new series
    series_t* newSeries = ( series_t* )malloc( sizeof( series_t ) );
    initSeriesStruct( newSeries );
    newSeries->description = (char*)seriesDescription.c_str();
    newSeries->number = seriesNumber;
    if( pCreateSeries( instance, &study, newSeries ) )
      SaveToSeries( instance, inputNodes, newSeries->oid, false, tmpDirectory );
    else
      std::cout << "SaveToCHILI (SaveAsNewSeries): Can not create a new Series." << std::endl;
    free( newSeries );
  }
  else
    std::cout << "SaveToCHILI (SaveAsNewSeries): Study not exist. Abort." << std::endl;
  clearStudyStruct( &study );
  #endif
}

void mitk::SaveToCHILI::SaveToSeries( QcPlugin* instance, DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& seriesOID, bool overrideExistingSeries, const std::string& tmpDirectory )
{
  //CHILI-Version >= 3.12 ???
  #if CHILI_VERSION_CODE( 1, 1, 3 ) < CHILI_PLUGIN_VERSION_CODE
  study_t study;
  patient_t patient;
  series_t series;
  initStudyStruct( &study );
  initPatientStruct( &patient );
  initSeriesStruct( &series );

  series.oid = strdup( seriesOID.c_str() );
  if( !pQuerySeries( instance, &series, &study, &patient ) )
  {
    std::cout << "SaveToCHILI (SaveToSeries): Series not exist. Abort." << std::endl;
    clearStudyStruct( &study );
    clearPatientStruct( &patient );
    clearSeriesStruct( &series );
    return;
  }

  //check study for parent-child-relation-file: if file exist, load this one; if no file exist, create a new on harddisk
  m_ParentChild->InitParentChild( instance, study.oid, study.instanceUID, patient.oid, tmpDirectory );
  //get current highest imagenumber
  unsigned int imageNumberCount = GetMaximumImageNumber( instance, seriesOID ) + 1;
  //get the needed study-, patient- and seriesinformation to save
  ImageToPicDescriptor::TagInformationList picTagList = GetNeededTagList( &study, &patient, &series );

  ProgressBar::GetInstance()->AddStepsToDo( inputNodes.size() );
  for( DataStorage::SetOfObjects::const_iterator nodeIter = inputNodes->begin(); nodeIter != inputNodes->end(); nodeIter++ )
  {
    if( (*nodeIter) )
    {
      BaseData* data = (*nodeIter)->GetData();
      if ( data )
      {
        std::string currentSeriesOID = "";
        std::string currentVolumeLabel = "";
        if( (*nodeIter)->GetProperty( "SeriesOID" ) )
           currentSeriesOID = (*nodeIter)->GetProperty( "SeriesOID" )->GetValueAsString();
        if( (*nodeIter)->GetProperty( "VolumeLabel" ) )
           currentVolumeLabel = (*nodeIter)->GetProperty( "VolumeLabel" )->GetValueAsString();

        Image* image = dynamic_cast<Image*>( data );
        if( image )  //SAVE IMAGE
        {
          if( currentSeriesOID == seriesOID )  //check if the current data always exist in current series
          {
            if( !overrideExistingSeries ) continue;

            BaseProperty::Pointer manufacturerProperty = (*nodeIter)->GetProperty( "CHILI: MANUFACTURER" );
            BaseProperty::Pointer institutionNameProperty = (*nodeIter)->GetProperty( "CHILI: INSTITUTION NAME" );
            //only data with "MANUFACTURER==MITK" and "INSTITUTION NAME==DKFZ.MBI" should be override
            if( !manufacturerProperty || !institutionNameProperty || manufacturerProperty->GetValueAsString() != "MITK" || institutionNameProperty->GetValueAsString() != "DKFZ.MBI" )
              continue;
          }

          //ImageToPicDescriptor
          ImageToPicDescriptor::Pointer converterToDescriptor = ImageToPicDescriptor::New();
          //Input
          LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<LevelWindowProperty*>( (*nodeIter)->GetProperty("levelwindow") );
          if( levelWindowProperty.IsNotNull() )
            converterToDescriptor->SetLevelWindow( levelWindowProperty->GetLevelWindow() );
          converterToDescriptor->SetImage( image );
          converterToDescriptor->SetImageNumber( imageNumberCount );

          //add the node-name as SeriesDescription to the Taglist (get saved as SeriesDescription)
          BaseProperty::Pointer nameProperty = (*nodeIter)->GetProperty( "name" );
          ImageToPicDescriptor::TagInformationStruct temp;
          temp.PicTagDescription = tagSERIES_DESCRIPTION;
          if( nameProperty )
            temp.PicTagContent = nameProperty->GetValueAsString();
          else temp.PicTagContent = "no Description";
          picTagList.push_back( temp );

          //for override images use saved pic-tags -> true, for add images create new pic-tags
          if( overrideExistingSeries && currentSeriesOID == seriesOID )
            converterToDescriptor->SetTagList( picTagList, true );
          else
            converterToDescriptor->SetTagList( picTagList, false );

          //create the picdescriptorlist
          converterToDescriptor->Update();
          std::list< ipPicDescriptor* > myPicDescriptorList = converterToDescriptor->GetOutput();

          //add the volume to xml-file if: the volume have no Parent-Child-Label (currentVolumeLabel); the volume saved first time to CHILI(currentSeriesOID); the volume get saved new
          if( currentVolumeLabel == "" || currentSeriesOID == "" || currentSeriesOID != seriesOID )
            m_ParentChild->AddEntry( (*nodeIter), converterToDescriptor->GetSaveImageInstanceUIDs(), seriesOID );

          //increase the imageNumber
          imageNumberCount = imageNumberCount + myPicDescriptorList.size();
          //delete the current node-name from the picTagList (added as SeriesDescription but the picTagList get used for the next image too)
          picTagList.pop_back();

          //save the single slices to CHILI
          pOpenAssociation();

          int count = 0;
          ProgressBar::GetInstance()->AddStepsToDo( myPicDescriptorList.size() );
          while( !myPicDescriptorList.empty() )
          {
            std::ostringstream helpFileName;
            if( (*nodeIter)->GetProperty( "name" )->GetValueAsString() != "" )
              helpFileName << (*nodeIter)->GetProperty( "name" )->GetValueAsString();
            else helpFileName << "FileUpload";
            helpFileName << count << ".pic";
            count++;
            std::string fileName = helpFileName.str();
            std::string pathAndFile = tmpDirectory + fileName;
            ipPicDescriptor *pic = myPicDescriptorList.front();
            ipPicPut( (char*)pathAndFile.c_str(), pic );
            ipPicFree( pic );
            myPicDescriptorList.pop_front();
            if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), NULL, NULL, study.instanceUID, patient.oid, study.oid, series.oid, NULL ) )
              std::cout << "ChiliPlugin (SaveToChili): Error while saving File (" << fileName << ") to Database." << std::endl;
            else
              if( remove( pathAndFile.c_str() ) != 0 )
                std::cout << "ChiliPlugin (SaveToChili): Not able to  delete file: " << pathAndFile << std::endl;
            ProgressBar::GetInstance()->Progress();
          }

          pCloseAssociation();
          //set the new SERIESOID as Property
          if( currentSeriesOID == "" )
            (*nodeIter)->SetProperty( "SeriesOID", StringProperty::New( seriesOID ) );
          else
            (*nodeIter)->ReplaceProperty( "SeriesOID", StringProperty::New( seriesOID ) );
        }
        else  //SAVE TEXT
        {
          //first: check the TextOID
          std::string textOID;
          BaseProperty::Pointer currentTextOID = (*nodeIter)->GetProperty( "TextOID" );
          if( currentTextOID )
          {
            //get the SeriesOID from the Text-File
            text_t text;
            series_t series;
            initTextStruct( &text );
            initSeriesStruct( &series );

            text.oid = strdup( currentTextOID->GetValueAsString().c_str() );
            if( pQueryText( instance, &text, &series, NULL, NULL ) )
            {
              if( strcmp( series.oid, seriesOID.c_str() ) == 0 )  //the SERIESOID from text and aim to save are equal
              {
                if( overrideExistingSeries )  //to override the OIDs have to be the same
                  textOID = currentTextOID->GetValueAsString().c_str();
                else  //the Text exist in current series and entries should not be override
                {
                  clearTextStruct( &text );
                  clearSeriesStruct( &series );
                  continue;
                }
              }
              else  //the SERIESOIDs are different
                textOID = pGetNewOID();
            }
            clearTextStruct( &text );
            clearSeriesStruct( &series );
            textOID = pGetNewOID();
          }
          else  //the Text-File saved first time
            textOID = pGetNewOID();

          //save Volume to Parent-Child-XML
          if( currentVolumeLabel == "" || currentSeriesOID == "" || currentSeriesOID != seriesOID )
          {
            std::list<std::string> inputList;
            inputList.clear();
            inputList.push_back( textOID );
            m_ParentChild->AddEntry( (*nodeIter), inputList, seriesOID );
          }

          //search for possible Writer
          std::list<FileWriter::Pointer> possibleWriter;
          std::list<LightObject::Pointer> allObjects = itk::ObjectFactoryBase::CreateAllInstance( "IOWriter" );
          for( std::list<LightObject::Pointer>::iterator iter = allObjects.begin(); iter != allObjects.end(); iter++ )
          {
            FileWriter* writer = dynamic_cast<FileWriter*>( iter->GetPointer() );
            if( writer )
              possibleWriter.push_back( writer );
            else std::cout << "SaveToCHILI (SaveToChili): no FileWriter returned" << std::endl;
          }

          //use Writer
          for( std::list<FileWriter::Pointer>::iterator it = possibleWriter.begin(); it != possibleWriter.end(); it++ )
          {
            if( it->GetPointer()->CanWrite( (*nodeIter) ) )
            {
              ProgressBar::GetInstance()->AddStepsToDo();
              //create filename
              std::string fileName;
              if( (*nodeIter)->GetProperty( "name" )->GetValueAsString() != "" )
                fileName = (*nodeIter)->GetProperty( "name" )->GetValueAsString();
              else fileName = "FileUpload";
              //set extension
              fileName = fileName + it->GetPointer()->GetFileExtension();
              //path and filename
              std::string pathAndFile = tmpDirectory + fileName;

              //save via Writer to harddisk
              it->GetPointer()->SetFileName( pathAndFile.c_str() );
              it->GetPointer()->SetInput( (*nodeIter) );
              it->GetPointer()->Write();

              //save file from harddisk to chili
              if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), it->GetPointer()->GetWritenMIMEType().c_str(), NULL, study.instanceUID, patient.oid, study.oid, series.oid, textOID.c_str() ) )
                std::cout << "SaveToCHILI (SaveToChili): Error while saving File (" << fileName << ") to Database." << std::endl;
              else
                if( remove(  pathAndFile.c_str() ) != 0 )
                  std::cout << "SaveToCHILI (SaveToChili): Not able to  delete file: " << it->GetPointer()->GetFileName() << std::endl;
              //set the OID�s as Property
              if( currentSeriesOID == "" )
                (*nodeIter)->SetProperty( "SeriesOID", StringProperty::New( seriesOID ) );
              else
                (*nodeIter)->ReplaceProperty( "SeriesOID", StringProperty::New( seriesOID ) );
              if( !currentTextOID )
                (*nodeIter)->SetProperty( "TextOID", StringProperty::New( textOID ) );
              else
                (*nodeIter)->ReplaceProperty( "TextOID", StringProperty::New( textOID ) );
              ProgressBar::GetInstance()->Progress();
            }
          }
        }
      }
    }
    ProgressBar::GetInstance()->Progress();
  }
  m_ParentChild->SaveRelationShip();
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
  if( study != NULL && patient != NULL && series != NULL )
  {
    ImageToPicDescriptor::TagInformationStruct temp;
    //study
    temp.PicTagDescription = tagSTUDY_INSTANCE_UID;
    temp.PicTagContent = study->instanceUID;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_DESCRIPTION;
    temp.PicTagContent = study->description;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_DATE;
    temp.PicTagContent = study->date;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_TIME;
    temp.PicTagContent = study->time;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_ID;
    temp.PicTagContent = study->id;
    resultList.push_back( temp );
    //self created
    temp.PicTagDescription = tagMANUFACTURER;
    temp.PicTagContent = "MITK";
    resultList.push_back( temp );
    temp.PicTagDescription = tagINSTITUTION_NAME;
    temp.PicTagContent = "DKFZ.MBI";
    resultList.push_back( temp );
    temp.PicTagDescription = tagMODALITY;
    std::string tempString = study->modality;
    tempString = tempString + " processed";
    temp.PicTagContent = tempString;
    resultList.push_back( temp );
    //series
    temp.PicTagDescription = tagSERIES_INSTANCE_UID;
    temp.PicTagContent = series->instanceUID;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSERIES_DATE;
    temp.PicTagContent = series->date;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSERIES_TIME;
    temp.PicTagContent = series->time;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSERIES_NUMBER;
    temp.PicTagContent = series->number;
    resultList.push_back( temp );
  }
  return resultList;
}

unsigned int mitk::SaveToCHILI::GetMaximumImageNumber( QcPlugin* instance, const std::string& seriesOID )
{
  m_MaximumImageNumber = 0;
  pIterateImages( instance, (char*)seriesOID.c_str(), NULL, &SaveToCHILI::GlobalIterateImagesForImageNumber, this );
  return m_MaximumImageNumber;
}

ipBool_t mitk::SaveToCHILI::GlobalIterateImagesForImageNumber( int /*rows*/, int row, image_t* image, void* user_data )
{
  SaveToCHILI* callingObject = static_cast<SaveToCHILI*>( user_data );
  if( image->number > callingObject->m_MaximumImageNumber )
    callingObject->m_MaximumImageNumber = image->number;
  return ipTrue; // true = next element; false = break iterate
}

#endif
