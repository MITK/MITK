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

//CHILI
#include <chili/plugin.h>

#ifdef CHILI_PLUGIN_VERSION_CODE

mitk::CHILIInformation::CHILIInformation()
{
}

mitk::CHILIInformation::~CHILIInformation()
{
}

mitk::PACSPlugin::StudyInformation mitk::CHILIInformation::GetStudyInformation( QcPlugin* instance, const std::string& seriesOID)
{
  PACSPlugin::StudyInformation resultInformation;
  resultInformation.OID = "";

  study_t study;
  series_t series;
  initStudyStruct( &study );
  initSeriesStruct( &series );

  if( seriesOID == "" )
  {
    //use current selected study
    if( pCurrentStudy() != NULL )
      study = (*dupStudyStruct( pCurrentStudy() ) );  //copy the StudyStruct
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
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( instance, &series, &study, NULL ) )
    {
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      std::cout << "CHILIInformation (GetStudyInformation): pQuerySeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }

  resultInformation.OID = study.oid;
  resultInformation.InstanceUID = study.instanceUID;
  resultInformation.ID = study.id;
  resultInformation.Date = study.date;
  resultInformation.Time = study.time;
  resultInformation.Modality = study.modality;
  resultInformation.Manufacturer = study.manufacturer;
  resultInformation.ReferingPhysician = study.referingPhysician;
  resultInformation.Description = study.description;
  resultInformation.ManufacturersModelName = study.manufacturersModelName;
  resultInformation.AccessionNumber = study.accessionNumber;
  resultInformation.InstitutionName = study.institutionName;
  resultInformation.PerformingPhysician = study.performingPhysician;
  resultInformation.ReportingPhysician = study.reportingPhysician;
  resultInformation.LastAccess = study.last_access;
  resultInformation.ImageCount = study.image_count;

  clearStudyStruct( &study );
  clearSeriesStruct( &series );
  return resultInformation;
}

mitk::PACSPlugin::PatientInformation mitk::CHILIInformation::GetPatientInformation( QcPlugin* instance, const std::string& seriesOID )
{
  PACSPlugin::PatientInformation resultInformation;
  resultInformation.OID = "";

  patient_t patient;
  study_t study;
  series_t series;
  initStudyStruct( &study );
  initSeriesStruct( &series );
  initPatientStruct( &patient );

  if( seriesOID == "" )
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

  resultInformation.OID = patient.oid;
  resultInformation.Name = patient.name;
  resultInformation.ID = patient.id;
  resultInformation.BirthDate = patient.birthDate;
  resultInformation.BirthTime = patient.birthTime;
  resultInformation.Sex = patient.sex;
  resultInformation.MedicalRecordLocator = patient.medicalRecordLocator;
  resultInformation.Comment = patient.comment;

  clearStudyStruct( &study );
  clearSeriesStruct( &series );
  clearPatientStruct( &patient );
  return resultInformation;
}

mitk::PACSPlugin::SeriesInformation mitk::CHILIInformation::GetSeriesInformation( QcPlugin* instance, const std::string& seriesOID )
{
  PACSPlugin::SeriesInformation resultInformation;
  resultInformation.OID = "";

  series_t series;
  initSeriesStruct( &series );

  if( seriesOID == "" )
  {
    //use current selected series
    if( pCurrentSeries() != NULL )
      series = (*dupSeriesStruct( pCurrentSeries() ) );  //copy seriesstruct
    else
    {
      std::cout << "CHILIInformation (GetSeriesInformation): pCurrentSeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }
  else
  {
    //let CHILI search
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( instance, &series, NULL, NULL ) )
    {
      clearSeriesStruct( &series );
      std::cout << "CHILIInformation (GetSeriesInformation): pQuerySeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }

  resultInformation.OID = series.oid;
  resultInformation.InstanceUID = series.instanceUID;
  resultInformation.Number = series.number;
  resultInformation.Acquisition = series.acquisition;
  resultInformation.EchoNumber = series.echoNumber;
  resultInformation.TemporalPosition = series.temporalPosition;
  resultInformation.Date = series.date;
  resultInformation.Time = series.time;
  resultInformation.Description = series.description;
  resultInformation.Contrast = series.contrast;
  resultInformation.BodyPartExamined = series.bodyPartExamined;
  resultInformation.ScanningSequence = series.scanningSequence;
  resultInformation.FrameOfReferenceUID = series.frameOfReferenceUID;
  resultInformation.ImageCount = series.image_count;

  clearSeriesStruct( &series );
  return resultInformation;
}

mitk::PACSPlugin::SeriesInformationList mitk::CHILIInformation::GetSeriesInformationList( QcPlugin* instance, const std::string& studyOID )
{
  //get used to save all series
  m_SeriesInformationList.clear();

  //iterate over all series
  if( studyOID == "" )
    pIterateSeries( instance, (char*)CHILIInformation::GetStudyInformation( instance ).OID.c_str(), NULL, &CHILIInformation::GlobalIterateSeries, this );
  else
    pIterateSeries( instance, (char*)studyOID.c_str(), NULL, &CHILIInformation::GlobalIterateSeries, this );

  //return the filled SeriesInformationList
  return m_SeriesInformationList;
}

ipBool_t mitk::CHILIInformation::GlobalIterateSeries( int /*rows*/, int /*row*/, series_t* series, void* user_data )
{
  mitk::CHILIInformation* callingObject = static_cast<mitk::CHILIInformation*>(user_data);
  //create new element and fill
  mitk::PACSPlugin::SeriesInformation newSeries;
  newSeries.OID = series->oid;
  newSeries.InstanceUID = series->instanceUID;
  newSeries.Number = series->number;
  newSeries.Acquisition = series->acquisition;
  newSeries.EchoNumber = series->echoNumber;
  newSeries.TemporalPosition = series->temporalPosition;
  newSeries.Date = series->date;
  newSeries.Time = series->time;
  newSeries.Description = series->description;
  newSeries.Contrast = series->contrast;
  newSeries.BodyPartExamined = series->bodyPartExamined;
  newSeries.ScanningSequence = series->scanningSequence;
  newSeries.FrameOfReferenceUID = series->frameOfReferenceUID;
  //add to list
  callingObject->m_SeriesInformationList.push_back( newSeries );

  return ipTrue; // true = next element; false = break iterate
}

mitk::PACSPlugin::TextInformation mitk::CHILIInformation::GetTextInformation( QcPlugin* instance, const std::string& textOID )
{
  PACSPlugin::TextInformation resultInformation;
  resultInformation.OID = "";

  if( textOID == "" )
    return resultInformation;

  text_t text;
  initTextStruct( &text );

  //let CHILI search
  text.oid = strdup( textOID.c_str() );
  if( !pQueryText( instance, &text, NULL, NULL, NULL ) )
  {
    clearTextStruct( &text );
    std::cout << "CHILIInformation (GetTextInformation): pQueryText() failed. Abort." << std::endl;
    return resultInformation;
  }

  resultInformation.OID = text.oid;
  resultInformation.MimeType = text.mime_type;
  resultInformation.ChiliText = text.chiliText;
  resultInformation.Status = text.status;
  resultInformation.FrameOfReferenceUID = text.frameOfReferenceUID;
  resultInformation.TextDate = text.text_date;
  resultInformation.Description = text.description;

  clearTextStruct( &text );
  return resultInformation;
}

mitk::PACSPlugin::TextInformationList mitk::CHILIInformation::GetTextInformationList( QcPlugin* instance, const std::string& seriesOID )
{
  //get used to save all text
  m_TextInformationList.clear();
  //iterate over all text
  pIterateTexts( instance, (char*)seriesOID.c_str(), NULL, &CHILIInformation::GlobalIterateText, this );
  //return the filled TextInformationList
  return m_TextInformationList;
}

ipBool_t mitk::CHILIInformation::GlobalIterateText( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  mitk::CHILIInformation* callingObject = static_cast<mitk::CHILIInformation*>(user_data);

  //we hide the parent-child-relation-file -> should not load ;)
  std::string chiliDatabaseName = text->chiliText;
  std::string fileName = chiliDatabaseName.substr( chiliDatabaseName.find_last_of("-") + 1 );
  if( fileName != "ParentChild.xml" )
  {
    //create new element and fill
    PACSPlugin::TextInformation resultText;
    resultText.OID = text->oid;
    resultText.MimeType = text->mime_type;
    resultText.ChiliText = text->chiliText;
    resultText.Status = text->status;
    resultText.FrameOfReferenceUID = text->frameOfReferenceUID;
    resultText.TextDate = text->text_date;
    resultText.Description = text->description;
    //add to list
    callingObject->m_TextInformationList.push_back( resultText );
  }
  return ipTrue; // true = next element; false = break iterate
}

#endif
