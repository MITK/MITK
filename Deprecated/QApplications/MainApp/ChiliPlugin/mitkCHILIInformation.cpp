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

#include "mitkCHILIInformation.h"

//MITK
#include <mitkIpPicUnmangle.h>
//CHILI
#include <chili/plugin.h>

#include "mitkChiliPlugin.h"

mitk::CHILIInformation::CHILIInformation()
{
}

mitk::CHILIInformation::~CHILIInformation()
{
}
    
mitk::PACSPlugin::StudyInformation mitk::CHILIInformation::GetStudyInformation( QcPlugin* instance, const std::string& seriesInstanceUID)
{
std::cout << "*** In CHILIInformation::GetStudyInformation" << std::endl;
  PACSPlugin::StudyInformation resultInformation;

  study_t study;
  series_t series;
  initStudyStruct( &study );
  initSeriesStruct( &series );

  if( seriesInstanceUID == "" )
  {
    //use current selected study
    if( pCurrentStudy() != NULL )
    {
      clearStudyStruct( &study );
      study = (*dupStudyStruct( pCurrentStudy() ) );  //copy the StudyStruct
    }
    else
    {
      std::cout << "CHILIInformation (GetStudyInformation): pCurrentStudy() failed. Abort." << std::endl;
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      return resultInformation;
    }
  }
  else
  {
    //let CHILI search
    std::string seriesOID = ChiliPlugin::GetChiliPluginInstance()->GetSeriesOIDFromInstanceUID( seriesInstanceUID );
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( instance, &series, &study, NULL ) )
    {
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      std::cout << "CHILIInformation (GetStudyInformation): pQuerySeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }

  // remember OID for UID
  ChiliPlugin::GetChiliPluginInstance()->UpdateStudyOIDForInstanceUID( study.oid, study.instanceUID );

  if (study.instanceUID)
    resultInformation.StudyInstanceUID = study.instanceUID;

  if (study.id)
    resultInformation.StudyID = study.id;

  if (study.date)
    resultInformation.StudyDate = study.date;

  if (study.time)
    resultInformation.StudyTime = study.time;

  if (study.accessionNumber)
    resultInformation.AccessionNumber = study.accessionNumber;

  if (study.modality)
    resultInformation.ModalitiesInStudy = study.modality;

  if (study.referingPhysician)
    resultInformation.ReferringPhysician = study.referingPhysician;

  if (study.description)
    resultInformation.StudyDescription = study.description;

  // PS 3.3 2008 Annex C.7.2.1.1.1 defines only referring physician, physician of record, performing physician and reading physician
  //resultInformation.ReportingPhysician = study.reportingPhysician; // TODO CHILI should clarify the meaning of the corresponding field in study_t, perhaps we can add it to PACSPlugin::StudyInformation

  clearStudyStruct( &study );
  clearSeriesStruct( &series );
  return resultInformation;
}

mitk::PACSPlugin::PatientInformation mitk::CHILIInformation::GetPatientInformation( QcPlugin* instance, const std::string& seriesInstanceUID )
{
  PACSPlugin::PatientInformation resultInformation;

  patient_t patient;
  study_t study;
  series_t series;
  initStudyStruct( &study );
  initSeriesStruct( &series );
  initPatientStruct( &patient );

  if( seriesInstanceUID == "" )
  {
    //use current selected patient
    if( pCurrentPatient() != NULL )
      patient = (*dupPatientStruct( pCurrentPatient() ) );  //copy patientstruct
    else
    {
      std::cout << "CHILIInformation (GetPatientInformation): pCurrentPatient() failed. Abort." << std::endl;
      clearPatientStruct( &patient );
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      return resultInformation;
    }
  }
  else
  {
    //let CHILI search
    std::string seriesOID = ChiliPlugin::GetChiliPluginInstance()->GetSeriesOIDFromInstanceUID( seriesInstanceUID );
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( instance, &series, &study, &patient ) )
    {
      clearPatientStruct( &patient );
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      std::cout << "CHILIInformation (GetPatientInformation): pQueryPatient() failed. Abort." << std::endl;
      return resultInformation;
    }
  }

  if (patient.name)
    resultInformation.PatientsName = patient.name;

  if (patient.id)
    resultInformation.PatientID = patient.id;

  if (patient.birthDate)
    resultInformation.PatientsBirthDate = patient.birthDate;

  if (patient.birthTime)
    resultInformation.PatientsBirthTime = patient.birthTime;

  if (patient.sex)
    resultInformation.PatientsSex = patient.sex;

  if (patient.comment)
    resultInformation.PatientComments = patient.comment;

  clearStudyStruct( &study );
  clearSeriesStruct( &series );
  clearPatientStruct( &patient );
  return resultInformation;
}

mitk::PACSPlugin::SeriesInformation mitk::CHILIInformation::GetSeriesInformation( QcPlugin* instance, const std::string& seriesInstanceUID )
{
  PACSPlugin::SeriesInformation resultInformation;
  resultInformation.SeriesInstanceUID = "";

  series_t series;
  initSeriesStruct( &series );

  if( seriesInstanceUID == "" )
  {
    //use current selected series
    if( pCurrentSeries() != NULL )
      series = (*dupSeriesStruct( pCurrentSeries() ) );  //copy seriesstruct
    else
    {
      clearSeriesStruct( &series );
      std::cout << "CHILIInformation (GetSeriesInformation): pCurrentSeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }
  else
  {
    //let CHILI search
    std::string seriesOID = ChiliPlugin::GetChiliPluginInstance()->GetSeriesOIDFromInstanceUID( seriesInstanceUID );
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( instance, &series, NULL, NULL ) )
    {
      clearSeriesStruct( &series );
      std::cout << "CHILIInformation (GetSeriesInformation): pQuerySeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }
  
  // remember OID for UID
  ChiliPlugin::GetChiliPluginInstance()->UpdateSeriesOIDForInstanceUID( series.oid, series.instanceUID );

  if (series.instanceUID)       
    resultInformation.SeriesInstanceUID = series.instanceUID;

  resultInformation.SeriesNumber = series.number;
  resultInformation.AcquisitionNumber = series.acquisition;
  resultInformation.EchoNumber = series.echoNumber;
  resultInformation.TemporalPosition = series.temporalPosition;

  if (series.date)       
    resultInformation.SeriesDate = series.date;

  if (series.time)       
    resultInformation.SeriesTime = series.time;

  if (series.description)       
    resultInformation.SeriesDescription = series.description;

  if (series.contrast)       
    resultInformation.ContrastAgent = series.contrast;

  if (series.bodyPartExamined)       
    resultInformation.BodyPartExamined = series.bodyPartExamined;

  if (series.scanningSequence)       
    resultInformation.ScanningSequence = series.scanningSequence;

  if (series.frameOfReferenceUID)       
    resultInformation.FrameOfReferenceUID = series.frameOfReferenceUID;

  clearSeriesStruct( &series );
  return resultInformation;
}

mitk::PACSPlugin::SeriesInformationList mitk::CHILIInformation::GetSeriesInformationList( QcPlugin* instance, const std::string& studyInstanceUID )
{
  // get study oid from uid
  std::string studyOID = ChiliPlugin::GetChiliPluginInstance()->GetStudyOIDFromInstanceUID( studyInstanceUID );

  //get used to save all series
  m_SeriesInformationList.clear();

  if( studyOID.empty() )
  {
    PACSPlugin::StudyInformation currentStudy = this->GetStudyInformation( instance, "" ); // gets info for currently selected study
    studyOID = ChiliPlugin::GetChiliPluginInstance()->GetStudyOIDFromInstanceUID( currentStudy.StudyInstanceUID );
  }

  //iterate over all series
  pIterateSeries( instance, (char*)studyOID.c_str(), NULL, &CHILIInformation::GlobalIterateSeries, this );

  //return the filled SeriesInformationList
  return m_SeriesInformationList;
}

ipBool_t mitk::CHILIInformation::GlobalIterateSeries( int /*rows*/, int /*row*/, series_t* series, void* user_data )
{
  CHILIInformation* callingObject = static_cast<CHILIInformation*>(user_data);
  //create new element and fill
  PACSPlugin::SeriesInformation newSeries;
  
  // remember OID for UID
  ChiliPlugin::GetChiliPluginInstance()->UpdateSeriesOIDForInstanceUID( series->oid, series->instanceUID );

  if (series)
  {
    if (series->instanceUID)       
      newSeries.SeriesInstanceUID = series->instanceUID;

    newSeries.SeriesNumber = series->number;
    newSeries.AcquisitionNumber = series->acquisition;
    newSeries.EchoNumber = series->echoNumber;
    newSeries.TemporalPosition = series->temporalPosition;

    if (series->date)       
      newSeries.SeriesDate = series->date;

    if (series->time)       
      newSeries.SeriesTime = series->time;

    if (series->description)       
      newSeries.SeriesDescription = series->description;

    if (series->contrast)       
      newSeries.ContrastAgent = series->contrast;

    if (series->bodyPartExamined)       
      newSeries.BodyPartExamined = series->bodyPartExamined;

    if (series->scanningSequence)       
      newSeries.ScanningSequence = series->scanningSequence;

    if (series->frameOfReferenceUID)       
      newSeries.FrameOfReferenceUID = series->frameOfReferenceUID;

    //add to list
    callingObject->m_SeriesInformationList.push_back( newSeries );
  }

  return ipTrue; // true = next element; false = break iterate
}

mitk::PACSPlugin::DocumentInformation mitk::CHILIInformation::GetDocumentInformation( QcPlugin* instance, const std::string& seriesInstanceUID, unsigned int textInstanceNumber )
{
  PACSPlugin::DocumentInformation resultInformation;
  // TODO insert call to clearDocumentInformation 

  if ( seriesInstanceUID.empty() )
    return resultInformation;

  text_t text;
  initTextStruct( &text );

  //let CHILI search
  std::string textOID = ChiliPlugin::GetChiliPluginInstance()->GetTextOIDFromSeriesInstanceUIDAndInstanceNumber( seriesInstanceUID, textInstanceNumber );
  text.oid = strdup( textOID.c_str() );
  if( !pQueryText( instance, &text, NULL, NULL, NULL ) )
  {
    clearTextStruct( &text );
    std::cout << "CHILIInformation (GetDocumentInformation): pQueryText() failed. Abort." << std::endl;
    return resultInformation;
  }

  resultInformation.SeriesInstanceUID = seriesInstanceUID;
  resultInformation.InstanceNumber = textInstanceNumber;
  if (text.mime_type)
    resultInformation.MimeType = text.mime_type;
  
  if (text.text_date)
    resultInformation.ContentDate = text.text_date;
  
  if (text.description)
    resultInformation.DocumentTitle = text.description;

  clearTextStruct( &text );
  return resultInformation;
}

mitk::PACSPlugin::DocumentInformationList mitk::CHILIInformation::GetDocumentInformationList( QcPlugin* instance, const std::string& seriesInstanceUID )
{
  //get used to save all text
  m_DocumentInformationList.clear();
  m_DocumentTextOIDList.clear();
  //iterate over all text
  std::string seriesOID = ChiliPlugin::GetChiliPluginInstance()->GetSeriesOIDFromInstanceUID( seriesInstanceUID );
  pIterateTexts( instance, (char*)seriesOID.c_str(), NULL, &CHILIInformation::GlobalIterateText, this );

  unsigned int instanceNumber = 0;
  std::list<std::string>::iterator OIDiter = m_DocumentTextOIDList.begin();
  for (PACSPlugin::DocumentInformationList::iterator iter = m_DocumentInformationList.begin(); 
       iter != m_DocumentInformationList.end(); 
       ++iter, ++OIDiter, ++instanceNumber )
  {
    iter->SeriesInstanceUID = seriesInstanceUID;
    iter->InstanceNumber = instanceNumber;
    
    ChiliPlugin::GetChiliPluginInstance()->UpdateTextOIDFromSeriesInstanceUIDAndInstanceNumber( *OIDiter, seriesInstanceUID, instanceNumber );
  }

  //return the filled DocumentInformationList
  return m_DocumentInformationList;
}

ipBool_t mitk::CHILIInformation::GlobalIterateText( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  CHILIInformation* callingObject = static_cast<CHILIInformation*>(user_data);

  //we hide the parent-child-relation-file -> should not load ;)
  std::string chiliDatabaseName = text->chiliText;
  std::string fileName = chiliDatabaseName.substr( chiliDatabaseName.find_last_of("-") + 1 );
  if( fileName != "ParentChild.xml" )
  {
    //create new element and fill
    PACSPlugin::DocumentInformation resultText;
    // InstanceNumber will be filled out by calling method GetDocumentInformationList
    if (text)
    {
      if (text->mime_type)
        resultText.MimeType = text->mime_type;
      if (text->text_date)
        resultText.ContentDate = text->text_date;
      if (text->description)
        resultText.DocumentTitle = text->description;

      //add to list
      callingObject->m_DocumentInformationList.push_back( resultText );
      callingObject->m_DocumentTextOIDList.push_back( text->oid );
    }
  }
  return ipTrue; // true = next element; false = break iterate
}
