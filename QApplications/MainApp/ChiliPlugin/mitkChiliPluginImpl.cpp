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

#include <QmitkStdMultiWidget.h>
#include <QmitkEventCollector.h>
#include <QcMITKTask.h>
#include <SampleApp.h>
//Chili
#include <chili/isg.h>
#include <chili/cdbTypes.h>
#include <chili/plugin.xpm>
#include <chili/qclightboxmanager.h>
#include <ipPic/ipPicTags.h>
#include <ipPic/ipPic.h>
#include <ipDicom/ipDicom.h>
//MITKPlugin
#include "mitkChiliPluginImpl.h"
#include "mitkChiliPluginEvents.h"
#include "mitkChiliPluginFactory.h"
#include "mitkImageToPicDescriptor.h"
#include "mitkPicDescriptorToNode.h"
#include "QmitkChiliPluginDifferentStudiesDialog.h"
//TODO entfernen wenn das neue Chili-Release installiert ist
  #include "mitkLightBoxImageReader.h"
//teleconference
#include <mitkConferenceToken.h>
#include <mitkConferenceEventMapper.h>
#include <QmitkChili3ConferenceKitFactory.h>
//QT
#include <qlayout.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qtooltip.h>

#include <mitkCoreObjectFactory.h>

#include "chili_lightbox_import.xpm"

QWidget* mitk::ChiliPluginImpl::s_Parent = NULL;
/** create event-logging object (ConferenceKit) */
Chili3ConferenceKitFactory chiliconferencekitfactory;

/** entry point for Chili */
extern "C"
QcEXPORT QObject* create( QWidget *parent )
{
  // save parent for parameter-less constructor
  mitk::ChiliPluginImpl::SetQtParent( parent );

  // overwrite implementation of mitk::ChiliPlugin with mitk::ChiliPluginImpl
  mitk::ChiliPluginFactory::RegisterOneFactory();

  return mitk::ChiliPluginImpl::GetQcPluginInstance();
}

/** constructor with hidden parameter s_Parent */
mitk::ChiliPluginImpl::ChiliPluginImpl()
: QcPlugin( s_Parent ),
  app( NULL ),
  m_LightBoxCount( 0 )
{
  task = new QcMITKTask( xpm(), s_Parent, name() );

  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  m_FileList.clear();

  m_tempDirectory = GetTempDirectory();
  if( m_tempDirectory == NULL || m_tempDirectory == "" )
    QMessageBox::information( 0, "MITK", "MITK was not able to create a Temp-Directory.\nYou can only Load and Save via Lightbox.\nMore should not be possible." );
  else
    std::cout << "ChiliPlugin: Create and use directory "<< m_tempDirectory << std::endl;
}

mitk::ChiliPluginImpl::~ChiliPluginImpl()
{
  // delete the created TempDirectory
  static bool deleteOnlyOneTime = false; // the destructor get called two times
  if( !deleteOnlyOneTime )
  {
    deleteOnlyOneTime = true;
    #ifdef WIN32
      std::string removeDirectory = "rm /s " + m_tempDirectory;
      std::cout << "ChiliPlugin: Delete directory "<< m_tempDirectory << std::endl;
    #else
      std::string removeDirectory = "rm -r " + m_tempDirectory;
      std::cout << "ChiliPlugin: Delete directory "<< m_tempDirectory << std::endl;
    #endif
    system( removeDirectory.c_str() );
  }

  task->deleteLater();

  // tricky tricky...
  m_ReferenceCountLock.Lock();
  ++m_ReferenceCount; // increase count, so the following notice to mitk::ChiliPlugin won't trigger another delete on "this"
  m_ReferenceCountLock.Unlock();

  ChiliPlugin::GetInstance(true); // set internal s_Instance = NULL, so ITK won't have another SmartPointer to us (hopefully nobody else has)

  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0; // set count to 0, so itk::LightObject won't trigger another delete...
  m_ReferenceCountLock.Unlock();
}

QString mitk::ChiliPluginImpl::name()
{
  return QString( "MITK" );
}

const char** mitk::ChiliPluginImpl::xpm()
{
  return (const char **)plugin_xpm;
}

/** set the qtparent, needed for QObject* create() */
void mitk::ChiliPluginImpl::SetQtParent( QWidget* parent )
{
  s_Parent = parent;
}

/** return the plugininstance */
QcPlugin* mitk::ChiliPluginImpl::GetQcPluginInstance()
{
  // give Chili the single instance of mitk::ChiliPluginImpl
  ChiliPlugin::Pointer pluginInstance = ChiliPlugin::GetInstance();
  ChiliPluginImpl::Pointer realPluginInstance = dynamic_cast<ChiliPluginImpl*>( pluginInstance.GetPointer() );

  return realPluginInstance.GetPointer();
}

/** return the conferenceid, the set method dont get used */
int mitk::ChiliPluginImpl::GetConferenceID()
{
  return m_QmitkChiliPluginConferenceID;
}

/** return thats a realy plugin */
bool mitk::ChiliPluginImpl::IsPlugin()
{
  return true;
}

/** return the studyinformation */
mitk::ChiliPlugin::StudyInformation mitk::ChiliPluginImpl::GetStudyInformation( const std::string& seriesOID )
{
  StudyInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return resultInformation;

#else

  study_t study;
  series_t series;
  initStudyStruct( &study );
  initSeriesStruct( &series );

  if( seriesOID == "" )
  {
    //use current selected patient
    if( pCurrentStudy() != NULL )
    {
      study = (*dupStudyStruct( pCurrentStudy() ) );
    }
    else
    {
      std::cout << "ChiliPlugin (GetStudyInformation): pCurrentStudy() failed. Abort." << std::endl;
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      return resultInformation;
    }
  }
  else
  {
    //let chili search for study
    series.oid = strdup( seriesOID.c_str() );

    if( !pQuerySeries( this, &series, &study, NULL ) )
    {
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      std::cout << "ChiliPlugin (GetStudyInformation): pQueryStudy() failed. Abort." << std::endl;
      return resultInformation;
    }
  }

  if( &study != NULL )
  {
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
    resultInformation.ImportTime = study.importTime;
    resultInformation.ChiliSenderID = study.chiliSenderID;
    resultInformation.AccessionNumber = study.accessionNumber;
    resultInformation.InstitutionName = study.institutionName;
    resultInformation.WorkflowState = study.workflow_state;
    resultInformation.Flags = study.flags;
    resultInformation.PerformingPhysician = study.performingPhysician;
    resultInformation.ReportingPhysician = study.reportingPhysician;
    resultInformation.LastAccess = study.last_access;
  }

  clearStudyStruct( &study );
  clearSeriesStruct( &series );
  return resultInformation;
#endif
}

/** return the patientinformation */
mitk::ChiliPlugin::PatientInformation mitk::ChiliPluginImpl::GetPatientInformation( const std::string& seriesOID )
{
  PatientInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return resultInformation;

#else

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
    {
      patient = (*dupPatientStruct( pCurrentPatient() ) );
    }
    else
    {
      std::cout << "ChiliPlugin (GetPatientInformation): pCurrentPatient() failed. Abort." << std::endl;
      clearPatientStruct( &patient );
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      return resultInformation;
    }
  }
  else
  {
    //let chili search for patient
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( this, &series, &study, &patient ) )
    {
      clearPatientStruct( &patient );
      clearStudyStruct( &study );
      clearSeriesStruct( &series );
      std::cout << "ChiliPlugin (GetPatientInformation): pQueryPatient() failed. Abort." << std::endl;
      return resultInformation;
    }
  }

  if( &patient != NULL )
  {
    resultInformation.OID = patient.oid;
    resultInformation.Name = patient.name;
    resultInformation.ID = patient.id;
    resultInformation.BirthDate = patient.birthDate;
    resultInformation.BirthTime = patient.birthTime;
    resultInformation.Sex = patient.sex;
    resultInformation.MedicalRecordLocator = patient.medicalRecordLocator;
    resultInformation.Comment = patient.comment;
  }

  clearStudyStruct( &study );
  clearSeriesStruct( &series );
  clearPatientStruct( &patient );
  return resultInformation;
#endif
}

/** return the seriesinformation */
mitk::ChiliPlugin::SeriesInformation mitk::ChiliPluginImpl::GetSeriesInformation( const std::string& seriesOID )
{
  SeriesInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return resultInformation;

#else

  series_t series;
  initSeriesStruct( &series );

  if( seriesOID == "" )
  {
    //use current selected series
    if( pCurrentSeries() != NULL )
    {
      series = (*dupSeriesStruct( pCurrentSeries() ) );
    }
    else
    {
      std::cout << "ChiliPlugin (GetSeriesInformation): pCurrentSeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }
  else
  {
    //let chili search for series
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( this, &series, NULL, NULL ) )
    {
      clearSeriesStruct( &series );
      std::cout << "ChiliPlugin (GetSeriesInformation): pQuerySeries() failed. Abort." << std::endl;
      return resultInformation;
    }
  }

  if( &series != NULL )
  {
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
  }

  clearSeriesStruct( &series );
  return resultInformation;
#endif
}

/** return the seriesinformationlist */
mitk::ChiliPlugin::SeriesInformationList mitk::ChiliPluginImpl::GetSeriesInformationList( const std::string& studyOID )
{
  m_SeriesInformationList.clear();

  iterateSeries( (char*)studyOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateSeriesCallback, this );

  return m_SeriesInformationList;
}

/** iterate over all series from one study */
ipBool_t mitk::ChiliPluginImpl::GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data )
{
  mitk::ChiliPluginImpl* callingObject = static_cast<mitk::ChiliPluginImpl*>(user_data);

  mitk::ChiliPlugin::SeriesInformation newSeries;

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

  callingObject->m_SeriesInformationList.push_back( newSeries );

  return ipTrue; // enum from ipTypes.h
}

/** return the seriesinformation */
mitk::ChiliPlugin::TextInformation mitk::ChiliPluginImpl::GetTextInformation( const std::string& textOID )
{
//TODO
// diese funktion wird aktuell noch nicht von Chili bereit gestellt
// soll in kürze erfolgen
// wenn Funktion geliefert wird auf CHILI_PLUGIN_VERSION achten
// es sollten nur noch minimale änderungen am code notwendig sein

  TextInformation resultInformation;
  resultInformation.OID = "";
/*
#ifdef CHILI_PLUGIN_VERSION_CODE

  if( textOID == "" )
  {
    return resultInformation;
  }

  text_t text;
  initTextStruct( &text );

  //let chili search for series
  text.oid = strdup( textOID.c_str() );
  if( !pQueryText( this, &text ) )
  {
    clearSeriesStruct( &text );
    std::cout << "ChiliPlugin (GetTextInformation): pQueryText() failed. Abort." << std::endl;
    return resultInformation;
  }

  if( &text != NULL )
  {
    resultInformation.OID = text.oid;
    resultInformation.MimeType = text.mime_type;
    resultInformation.ChiliText = text.chiliText;
    resultInformation.Status = text.status;
    resultInformation.FrameOfReferenceUID = text.frameOfReferenceUID;
    resultInformation.TextDate = text.text_date;
    resultInformation.Description = text.description;
  }

  clearStudyStruct( &text );
#endif
*/
  return resultInformation;
}

/** return the textinformationlist */
mitk::ChiliPlugin::TextInformationList mitk::ChiliPluginImpl::GetTextInformationList( const std::string& seriesOID )
{
  m_TextInformationList.clear();

#ifdef CHILI_PLUGIN_VERSION_CODE

  pIterateTexts( this, (char*)seriesOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateTextOneCallback, this );

#endif

  return m_TextInformationList;
}

#ifdef CHILI_PLUGIN_VERSION_CODE
/** iterate over all texts from one series */
ipBool_t mitk::ChiliPluginImpl::GlobalIterateTextOneCallback( int rows, int row, text_t *text, void *user_data )
{
  mitk::ChiliPluginImpl* callingObject = static_cast<mitk::ChiliPluginImpl*>(user_data);

  mitk::ChiliPlugin::TextInformation resultText;

  resultText.OID = text->oid;
  resultText.MimeType = text->mime_type;
  resultText.ChiliText = text->chiliText;
  resultText.Status = text->status;
  resultText.FrameOfReferenceUID = text->frameOfReferenceUID;
  resultText.TextDate = text->text_date;
  resultText.Description = text->description;

  callingObject->m_TextInformationList.push_back( resultText );

  return ipTrue; // enum from ipTypes.h
}
#endif

/** get count, set by lightboxTiles()*/
unsigned int mitk::ChiliPluginImpl::GetLightboxCount()
{
  return m_LightBoxCount;
}

/** return the first found empty chililightbox, using the lightboxmanager therefore */
QcLightbox* mitk::ChiliPluginImpl::GetNewLightbox()
{
  QcLightbox* newLightbox;
  // chili can show maximal four lightboxes
  for( unsigned int i = 0; i < NUMBER_OF_THINKABLE_LIGHTBOXES; i++ )
  {
    newLightbox = lightboxManager()->getLightboxes().at(i);
    //return the first empty lightbox
    if( newLightbox->getFrames() == 0 )
    {
      newLightbox->activate();
      newLightbox->show();
      Modified();
      return newLightbox;
    }
  }
  QMessageBox::information( 0, "MITK", "You have reached the maximum count of Lightboxes. No one more can created." );
  return NULL;
}

/** return the current selected chililightbox, using the lightboxmanager therefore */
QcLightbox* mitk::ChiliPluginImpl::GetCurrentLightbox()
{
  QcLightbox* currentLightbox = lightboxManager()->getActiveLightbox();
  if( currentLightbox == NULL)
  {
    QMessageBox::information( 0, "MITK", "There is no selected Lightbox in Chili." );
  }
  return currentLightbox;
}

/** load the image from the lightbox */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPluginImpl::LoadImagesFromLightbox( QcLightbox* lightbox )
{
  std::vector<DataTreeNode::Pointer> resultVector;
  resultVector.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  //QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  //return resultVector;

  //TODO entfernen wenn das neue Chili-Release installiert ist
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  LightBoxImageReader::Pointer reader = LightBoxImageReader::New();
  reader->SetLightBox( lightbox );
  Image::Pointer image = reader->GetOutput();
  image->Update();
  if( image->IsInitialized() )
  //create node
  {
    DataTreeNode::Pointer node = DataTreeNode::New();
    node->SetData( image );
    DataTreeNodeFactory::SetDefaultImageProperties( node );
    resultVector.push_back( node );
  }
  QApplication::restoreOverrideCursor();
  return resultVector;

#else

  if( lightbox == NULL )
  {
    lightbox = GetCurrentLightbox();
    if( lightbox == NULL )
    {
      return resultVector;  //abort
    }
  }

  if( lightbox->getFrames() > 0 )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    std::list< ipPicDescriptor* > lightboxPicDescriptorList;
    lightboxPicDescriptorList.clear();

    //read all frames from Lightbox
    for( unsigned int n = 0; n < lightbox->getFrames(); n++ )
    {
      lightboxPicDescriptorList.push_back( lightbox->fetchPic(n) );
    }

    //use class PicDescriptorToNode
    PicDescriptorToNode* converterToNode = new PicDescriptorToNode();
    //input - processing - output - delete
    converterToNode->SetInput( lightboxPicDescriptorList, lightbox->currentSeries()->oid );
    converterToNode->GenerateNodes();
    resultVector = converterToNode->GetOutput();
    delete converterToNode;

    QApplication::restoreOverrideCursor();
  }

  return resultVector;
#endif
}

/** save images to lightbox */
void mitk::ChiliPluginImpl::SaveImageToLightbox( Image* sourceImage, const mitk::PropertyList::Pointer propertyList , QcLightbox* lightbox )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return;

#else

  if( sourceImage == NULL )
  {
    std::cout << "ChiliPlugin (SaveImageToLightbox): The sourceImage is empty. Abort." << std::endl;
    return;
  }

  if( lightbox == NULL)
  {
    std::cout << "ChiliPlugin (SaveImageToLightbox): No lightbox set. Abort." << std::endl;
    return;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  TagInformationList picTagList;
  picTagList.clear();

  //get the seriesOID
  mitk::BaseProperty::Pointer seriesOIDProperty = propertyList->GetProperty( "SeriesOID" );
  if( !seriesOIDProperty )
  {
    //the image have no seriesOID -> it was not loaded from chili -> add to current selected series
    picTagList = GetAllAvailableStudyAndPatientPicTags( GetStudyInformation().OID );
  }
  else
  {
    //if the current study different to the study from the given series
    if( strcmp( GetStudyInformation( seriesOIDProperty->GetValueAsString() ).OID.c_str(), pCurrentStudy()->oid ) != 0 )
    {
      //ask the user which study he want to use
      QmitkChiliPluginDifferentStudiesDialog myDialog( 0 );

      //add current selected study
      myDialog.addStudy( GetStudyInformation(), GetPatientInformation(), "current seleted Series" );

      //add the series where the image from
      //get the nameProperty
      mitk::BaseProperty::Pointer nameProperty = propertyList->GetProperty( "name" );
      if( nameProperty )
      {
        myDialog.addStudy( GetStudyInformation( seriesOIDProperty->GetValueAsString() ), GetPatientInformation( seriesOIDProperty->GetValueAsString() ), nameProperty->GetValueAsString() );
      }
      else
      {
        myDialog.addStudy( GetStudyInformation( seriesOIDProperty->GetValueAsString() ), GetPatientInformation( seriesOIDProperty->GetValueAsString() ), "no name" );
      }

      QApplication::restoreOverrideCursor();
      int dialogReturnValue = myDialog.exec();
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      if( dialogReturnValue == QDialog::Rejected || myDialog.GetSelectedStudy() == "" )
      {
        QApplication::restoreOverrideCursor();
        return; //user abort
      }
      else
      {
        // TODO test ob überschreiben ( + überschreibendialog )
      }
    }
    else
    {
      //TODO überschreibendialog
      //picTagList = GetAllAvailableStudyAndPatientPicTags( GetStudyInformation( seriesOIDProperty->GetValueAsString() ).OID );
      //picTagList.clear();
    }
  }

  //get the levelWindow
  mitk::LevelWindow levelWindow;
  mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>( propertyList->GetProperty("levelwindow").GetPointer() );
  if( levelWindowProperty.IsNotNull() )
  {
    levelWindow = levelWindowProperty->GetLevelWindow();
  }
  else
  {
    levelWindow.SetAuto( sourceImage );
  }

  //seperate the image into PicDescriptors
  ImageToPicDescriptor* converterToPicDescriptor = new ImageToPicDescriptor();
  //input - processing - output
  converterToPicDescriptor->SetInput( sourceImage, picTagList, levelWindow );
  converterToPicDescriptor->Write();
  std::list< ipPicDescriptor* > resultPicDescriptorList = converterToPicDescriptor->GetOutput();
  //delete
  delete converterToPicDescriptor;
  //add the single slices to the lightbox, they dont get saved, only if the user press the Chili-save-button
  while( !resultPicDescriptorList.empty() )
  {
    lightbox->addImage( resultPicDescriptorList.front() );
    resultPicDescriptorList.pop_front();
  }
  QApplication::restoreOverrideCursor();
#endif
}

/** load images- and text-files from series */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPluginImpl::LoadCompleteSeries( const std::string& seriesOID )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return resultNodes;

#else

  if( m_tempDirectory == NULL || m_tempDirectory == "" )
  {
    return resultNodes;
  }

  if( seriesOID == "" )
  {
    QMessageBox::information( 0, "MITK", "No SeriesOID set. Do you select a Series?" );
    return resultNodes;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  //first: load the image from db
  resultNodes = LoadAllImagesFromSeries( seriesOID );
  //second: load the text-files from db
  std::vector<DataTreeNode::Pointer> tempResultNodes = LoadAllTextsFromSeries( seriesOID );
  while( !tempResultNodes.empty() )
  //add the text-nodes to the image-nodes
  {
    resultNodes.push_back( tempResultNodes.back() );
    tempResultNodes.pop_back();
  }

  QApplication::restoreOverrideCursor();
  return resultNodes;
#endif
}

/** this function load all images from the given series(OID) */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPluginImpl::LoadAllImagesFromSeries( const std::string& seriesOID )
{
//TODO read DicomFiles via PicDescriptorToNode

  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return resultNodes;

#else
  if( m_tempDirectory == NULL || m_tempDirectory == "" )
  {
    return resultNodes;
  }

  if( seriesOID == "" )
  {
    std::cout << "ChiliPlugin (LoadAllImagesFromSeries): No SeriesOID set. Do you select a Series?" << std::endl;
    return resultNodes;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  //this is the list of all loaded files from chili
  m_FileList.clear();

  //iterate over all images from this series, save them to harddisk and set all filenames to m_FileList
  iterateImages( (char*)seriesOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateImagesCallback, this );

  //read the actually saved files
  if( !m_FileList.empty() )
  {
    //its possible that files with different fileextensions get saved, we have to handle them seperate
    std::list<std::string> picFileList;
    std::list<std::string> dicomFileList;
    std::list<std::string> otherFileList;
    picFileList.clear();
    dicomFileList.clear();
    otherFileList.clear();

    //seperate them
    while( !m_FileList.empty() )
    {
      char* fileExtension;
      fileExtension = strrchr( m_FileList.front().c_str(), '.' );
      if( strcmp( fileExtension, ".pic" ) == 0 )
      {
        picFileList.push_back( m_FileList.front() );
      }
      else
      {
//      if( strcmp( fileExtension, ".dcm" ) == 0 )
//      {
//        dicomFileList.push_back( m_FileList.front() );
//      }
//      else
//      {
          otherFileList.push_back( m_FileList.front() );
//      }
      }
      m_FileList.pop_front();
    }

    //pic- and dicom-Files get read via mitk::PicDescriptorToNode
    //the rest get read via mitk::DataTreeNodeFactory

    // read pic-files
    std::list<ipPicDescriptor*> picDescriptorToNodeInput;
    picDescriptorToNodeInput.clear();

    while( !picFileList.empty() )
    {
      //read the descriptor from harddisk
      ipPicDescriptor *pic;
      pic = ipPicGet( (char*)picFileList.front().c_str(), NULL );
      if( pic != NULL )
      {
        //add them to the input
        picDescriptorToNodeInput.push_back( pic );
        //after loading and adding we dont need them as file
        if( remove( picFileList.front().c_str() ) != 0 )
        {
          std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << picFileList.front().c_str() << std::endl;
        }
      }
      picFileList.pop_front();
    }
/*
    // read dicom-files
    ipUInt1_t* header = NULL;
    ipUInt4_t  header_size;
    ipUInt1_t* image = NULL;
    ipUInt4_t  image_size;

    while( !dicomFileList.empty() )
    {
      dicomGetHeaderAndImage( (char*)dicomFileList.front().c_str(), &header, &header_size, &image, &image_size );

      if( !header )
      {
        std::cout<< "ChiliPlugin (LoadAllImagesFromSeries): Could not get header." <<std::endl;
      }
      if( !image )
      {
        std::cout<< "ChiliPlugin (LoadAllImagesFromSeries): Could not get image." <<std::endl;
      }

      ipPicDescriptor *pic = dicomToPic( header, header_size, image, image_size );  //this PicDescriptor is right, but dont work

      if( pic != NULL)
      {
        //add them to the input
        picDescriptorToNodeInput.push_back( pic );
        if( remove( dicomFileList.front().c_str() ) != 0 )
        {
          std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << dicomFileList.front().c_str() << std::endl;
        }
      }
      dicomFileList.pop_front();
    }
*/
    //make the readed picDescriptors to multiple Nodes
    if( !picDescriptorToNodeInput.empty() )
    {
      PicDescriptorToNode* converterToNode = new PicDescriptorToNode();
      //input - processing - output
      converterToNode->SetInput( picDescriptorToNodeInput, seriesOID );
      converterToNode->GenerateNodes();
      resultNodes = converterToNode->GetOutput();
      delete converterToNode;
    }

    //read the rest via DataTreeNodeFactory
    if( !otherFileList.empty() )
    {
      DataTreeNodeFactory::Pointer factory = DataTreeNodeFactory::New();
      try
      {
        factory->SetFileName( otherFileList.front().c_str() );
        factory->Update();
        for( unsigned int i = 0 ; i < factory->GetNumberOfOutputs(); ++i )
        {
          DataTreeNode::Pointer node = factory->GetOutput( i );
          if ( ( node.IsNotNull() ) && ( node->GetData() != NULL )  )
          {
            //the PicDescriptorToNode add the seriesOID and description automatically, the DataTreeNodeFactory not
            series_t series;
            initSeriesStruct( &series );
            //set the seriesOID
            series.oid = strdup( seriesOID.c_str() );

            //Chili fill the rest
            if( pQuerySeries( this, &series, NULL, NULL ) )
            {
              if( series.description == "" )
              {
                node->SetProperty( "name", new StringProperty( "no description" ) );
              }
              else
              {
                node->SetProperty( "name", new StringProperty( series.description ) );
              }
            }
            clearSeriesStruct( &series );
            node->SetProperty( "SeriesOID", new StringProperty( seriesOID ) );
            resultNodes.push_back( node );
          }
        }
        //if there is no Output, something going wrong -> dont delete the files (until the application get closed)
        if( factory->GetNumberOfOutputs() > 0 )
        {
          while( !otherFileList.empty() )
          {
            if( remove(  otherFileList.front().c_str() ) != 0 )
            {
              std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << otherFileList.front().c_str() << std::endl;
            }
            otherFileList.pop_front();
          }
        }
      }
      catch ( itk::ExceptionObject & ex )
        itkGenericOutputMacro( << "Exception during file open: " << ex );
    }
  }
  QApplication::restoreOverrideCursor();
  return resultNodes;
#endif
}

/** function to iterate over all images from a series */
ipBool_t mitk::ChiliPluginImpl::GlobalIterateImagesCallback( int rows, int row, image_t* image, void* user_data )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  ChiliPluginImpl* callingObject = static_cast<ChiliPluginImpl*>( user_data );

  //get the FileExtension from the file ( the mitk-filereader need them )
  char* fileExtension;
  fileExtension = strrchr( image->path, '.' );

  //create a new FileName (1.*, 2.*, 3.*, ...) with FileExtension from DB
  std::ostringstream stringHelper;
  stringHelper << row << fileExtension;
  std::string pathAndFile = callingObject->m_tempDirectory + stringHelper.str();

  //save to harddisk
  ipInt4_t error;
  pFetchDataToFile( image->path, pathAndFile.c_str(), &error );
  //all right?
  if( error != 0 )
    std::cout << "ChiliPlugin (GlobalIterateImagesCallback): ChiliError: " << error << ", while reading file (" << image->path << ") from Database." << std::endl;
  else
    //then add the filename to the list
    callingObject->m_FileList.push_back( pathAndFile );
#endif
  return ipTrue; // enum from ipTypes.h
}

/** this function load all text-files from the series */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPluginImpl::LoadAllTextsFromSeries( const std::string& seriesOID )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return resultNodes;

#else

  if( m_tempDirectory == NULL || m_tempDirectory == "" )
  {
    return resultNodes;
  }

  if( seriesOID == "" )
  {
    std::cout << "ChiliPlugin (LoadAllTextsFromSeries): No SeriesOID set. Do you select a Series?" << std::endl;
    return resultNodes;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  //this is the list of all loaded files from chili
  m_TextFileList.clear();

  //now we iterate over all text-files from the series, save the db-path and text-oid to m_TextFileList
  //dont save here to harddisk; double filedescriptions override themselve
  pIterateTexts( this, (char*)seriesOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateTextTwoCallback, this );

  DataTreeNodeFactory::Pointer factory = DataTreeNodeFactory::New();

  //load the files from harddisk to mitk
  while( !m_TextFileList.empty() )
  {
    //get Filename and path to save to harddisk
    char* tempFileName;
    tempFileName = strrchr( m_TextFileList.front().TextFilePath.c_str(), '-' );
    //we want no "-" in front of the filename
    tempFileName++;
    std::string fileName = tempFileName;

    std::string pathAndFile = m_tempDirectory + fileName;

    //Load File from DB
    ipInt4_t error;
    pFetchDataToFile( m_TextFileList.front().TextFilePath.c_str(), pathAndFile.c_str(), &error );
    if( error != 0 )
    {
      std::cout << "ChiliPlugin (LoadAllTextsFromSeries): ChiliError: " << error << ", while reading file (" << fileName << ") from Database." << std::endl;
    }
    else
    {
      //if there are no fileextension in the filename, the reader not able to load the file and the plugin crashed
      //dont use DataTreeNodeFactory::GetFilePattern(): if someone create a new reader and dont add the extension to "GetFilePattern()", his reader doesnt work
      char* fileExtension;
      fileExtension = strrchr( tempFileName, '.' );
      if( fileExtension == NULL )
      {
        std::cout << "ChiliPlugin (LoadAllTextsFromSeries): Reader not able to read file without extension. ("<< pathAndFile << ")" << std::endl;
      }
      else
      {
        //try to Read the File
        try
        {
          factory->SetFileName( pathAndFile );
          factory->Update();
          for ( unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i )
          {
            DataTreeNode::Pointer node = factory->GetOutput( i );
            if ( ( node.IsNotNull() ) && ( node->GetData() != NULL )  )
            {
              //get the FileExtension and cut them from the filename
              char* fileExtension;
              fileExtension = strrchr( m_TextFileList.front().TextFilePath.c_str(), '.' );
              int size = fileName.size() - sizeof( fileExtension );
              fileName.erase( size );
              node->SetProperty( "name", new StringProperty( fileName ) );
              node->SetProperty( "TextOID", new StringProperty( m_TextFileList.front().TextFileOID ) );
               node->SetProperty( "SeriesOID", new StringProperty( seriesOID ) );
              resultNodes.push_back( node );
              if( remove(  pathAndFile.c_str() ) != 0 )
              {
                std::cout << "ChiliPlugin (LoadAllTextsFromSeries): Not able to  delete file: " << pathAndFile << std::endl;
              }
            }
          }
        }
        catch ( itk::ExceptionObject & ex )
          itkGenericOutputMacro( << "Exception during file open: " << ex );
      }
    }
  m_TextFileList.pop_front();
  }
  QApplication::restoreOverrideCursor();
  return resultNodes;
#endif
}

#ifdef CHILI_PLUGIN_VERSION_CODE
/** this function iterate over all text-files from a series */
ipBool_t mitk::ChiliPluginImpl::GlobalIterateTextTwoCallback( int rows, int row, text_t *text, void *user_data )
{
  //cast to chiliplugin to save the single textfileinformation
  ChiliPluginImpl* callingObject = static_cast<ChiliPluginImpl*>(user_data);

  if( text != NULL )
  {
    ChiliPluginImpl::TextFilePathAndOIDStruct newTextFile;
    newTextFile.TextFilePath = text->chiliText;
    newTextFile.TextFileOID = text->oid;
    callingObject->m_TextFileList.push_back( newTextFile );
  }
  return ipTrue; // enum from ipTypes.h
}
#endif

mitk::DataTreeNode::Pointer mitk::ChiliPluginImpl::LoadOneTextFromSeries( const std::string& seriesOID, const std::string& textOID )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return NULL;

#else

  if( m_tempDirectory == NULL || m_tempDirectory == "" )
  {
    return NULL;
  }

  if( textOID == "" || seriesOID == "" )
  {
    std::cout << "ChiliPlugin (LoadOneTextFromSeries): No Text-OID set. Abort." << std::endl;
    return NULL;
  }

  TextInformation textToLoad = GetTextInformation( textOID );
  if( textToLoad.OID != "" )
  {
    //get Filename and path to save to harddisk
    char* tempFileName;
    tempFileName = strrchr( textToLoad.ChiliText.c_str(), '-' );
    //we want no "-" in front of the filename
    tempFileName++;

    std::string fileName = tempFileName;
    std::string pathAndFile = m_tempDirectory + fileName;

    //Load File from DB
    ipInt4_t error;
    pFetchDataToFile( textToLoad.ChiliText.c_str(), pathAndFile.c_str(), &error );
    if( error != 0 )
    {
      std::cout << "ChiliPlugin (LoadOneTextFromSeries): ChiliError: " << error << ", while reading file (" << fileName << ") from Database." << std::endl;
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
          DataTreeNode::Pointer node = factory->GetOutput( i );
          if ( ( node.IsNotNull() ) && ( node->GetData() != NULL )  )
          {
            //get the FileExtension and cut them from the filename
            char* fileExtension;
            fileExtension = strrchr( textToLoad.ChiliText.c_str(), '.' );
            //cut the fileExtension from the filename
            int size = fileName.size() - sizeof( fileExtension );
            fileName.erase( size );
            //set the filename without extension as name-property
            node->SetProperty( "name", new StringProperty( fileName ) );
            node->SetProperty( "TextOID", new StringProperty( m_TextFileList.front().TextFileOID ) );
            node->SetProperty( "SeriesOID", new StringProperty( seriesOID ) );

            if( remove(  pathAndFile.c_str() ) != 0 )
            {
              std::cout << "ChiliPlugin (LoadOneTextFromSeries): Not able to  delete file: " << pathAndFile << std::endl;
            }
            return node;
          }
        }
      }
      catch ( itk::ExceptionObject & ex )
          itkGenericOutputMacro( << "Exception during file open: " << ex );
    }
  }
  else return NULL;
#endif
}

/** save image- and text-files to the chili-database */
void mitk::ChiliPluginImpl::SaveToChili( DataStorage::SetOfObjects::ConstPointer inputNodes )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Youre current ChiliVersion dont support that function." );
  return;

#else
/*
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  if( m_tempDirectory == NULL || m_tempDirectory == "" )
  {
    QApplication::restoreOverrideCursor();
    return;
  }

  //get the destination-study
  std::list<std::string> studyOIDList;
  studyOIDList.clear();

  for( DataStorage::SetOfObjects::const_iterator nodeIter = inputNodes->begin(); nodeIter != inputNodes->end(); nodeIter++ )
  {
    if ( (*nodeIter) )
    {
      mitk::BaseProperty::Pointer propertySeriesOID = (*nodeIter)->GetProperty( "SeriesOID" );
      if( propertySeriesOID && propertySeriesOID->GetValueAsString() != "" )
      {
        series_t series;
        study_t study;
        initSeriesStruct( &series );
        initStudyStruct( &study );

        series.oid = strdup( propertySeriesOID->GetValueAsString().c_str() );
        if( pQuerySeries( this, &series, &study, NULL ) )
          studyOIDList.push_back( study.oid );
      }
    }
  }
  //dont forget the currentOID
  studyOIDList.push_back( pCurrentStudy()->oid );

  studyOIDList.sort();  //unique needed
  studyOIDList.unique();  //eleminate all double entries

  std::string destinationStudyOID = "";

  if( studyOIDList.size() > 1 )
  {
    QmitkChiliPluginDifferentStudiesDialog myDialog( 0 );

    for( std::list<std::string>::iterator iter = studyOIDList.begin(); iter != studyOIDList.end(); iter++)
    {
      study_t study;
      patient_t patient;
      initStudyStruct( &study );
      initPatientStruct( &patient );

      study.oid = strdup( (*iter).c_str() );
      if( pQueryStudy( this, &study, &patient ) )
        myDialog.addStudy( study.oid, patient.name, patient.id, study.description, study.date, study.modality );

      clearStudyStruct( &study );
      clearPatientStruct( &patient );
    }

    QApplication::restoreOverrideCursor();
    int dialogReturnValue = myDialog.exec();
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    if( dialogReturnValue == QDialog::Rejected || myDialog.GetSelectedStudy() == "" )
    {
      QApplication::restoreOverrideCursor();
      return; //user abort
    }
    else destinationStudyOID = myDialog.GetSelectedStudy();
  }
  else destinationStudyOID = studyOIDList.front();

  study_t resultStudy;
  patient_t resultPatient;
  initStudyStruct( &resultStudy );
  initPatientStruct( &resultPatient );
  resultStudy.oid = strdup( destinationStudyOID.c_str() );
  if( !pQueryStudy( this, &resultStudy, &resultPatient ) )
  {
    QApplication::restoreOverrideCursor();
    std::cout << "ChiliPlugin-Warning: pQueryStudy() failed." << std::endl;
    return; //user abort
  }

  //now we can save the single nodes
  for( DataStorage::SetOfObjects::const_iterator nodeIter = inputNodes->begin(); nodeIter != inputNodes->end(); nodeIter++ )
  {
    if ( (*nodeIter) )
    {
      BaseData* data = (*nodeIter)->GetData();
      if ( data )
      {
        std::string destinationSeriesOID = "";

        //check if the current node already exist in the destination-study
        mitk::BaseProperty::Pointer propertySeriesOID = (*nodeIter)->GetProperty( "SeriesOID" );
        if( propertySeriesOID && propertySeriesOID->GetValueAsString() != "" )
        {
          series_t series;
          study_t study;
          initSeriesStruct( &series );
          initStudyStruct( &study );

          series.oid = strdup( propertySeriesOID->GetValueAsString().c_str() );
          if( pQuerySeries( this, &series, &study, NULL ) )
          {
            if( study.oid == destinationStudyOID )
            {
              //TODO ask what to do
            }
            else
            {
              //create a new series in the study
              destinationSeriesOID = CreateNewSeries( &resultStudy );
              if( destinationSeriesOID == "" )
              {
                QApplication::restoreOverrideCursor();
                return;
              }
            }
          }
          else
          {
            QApplication::restoreOverrideCursor();
            std::cout << "ChiliPlugin-Warning: pQuerySeries() failed." << std::endl;
            return;
          }
          clearStudyStruct( &study );
          clearSeriesStruct( &series );
        }
        else
        {
          //create a new series in the study
          destinationSeriesOID = CreateNewSeries( &resultStudy );
          if( destinationSeriesOID == "" )
          {
            QApplication::restoreOverrideCursor();
            return;
          }
        }

        Image* image = dynamic_cast<Image*>( data );
        if( image )
        //save Image
        {
          //levelWindow
          LevelWindow levelWindow;
          LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<LevelWindowProperty*>( (*nodeIter)->GetProperty("levelwindow").GetPointer() );
          if( levelWindowProperty.IsNotNull() )
            levelWindow = levelWindowProperty->GetLevelWindow();
          else levelWindow.SetAuto( image );

          //ImageToPicDescriptor
          ImageToPicDescriptor* converterToDescriptor = new ImageToPicDescriptor();
          //input - processing - output
          //TODO converterToDescriptor->SetInput( image, NULL, levelWindow );
          converterToDescriptor->Write();
          std::list< ipPicDescriptor* > myPicDescriptorList = converterToDescriptor->GetOutput();

          int count = 0;
          while( !myPicDescriptorList.empty() )
          {
            std::ostringstream helpFileName;
            if( (*nodeIter)->GetProperty( "name" )->GetValueAsString() != "" )
              helpFileName << (*nodeIter)->GetProperty( "name" )->GetValueAsString();
            else helpFileName << "FileUpload";
            helpFileName << count << ".pic";
            std::string fileName = helpFileName.str();

            std::string pathAndFile = m_tempDirectory + fileName;

            ipPicDescriptor *pic = myPicDescriptorList.front();

            ipPicPut( (char*)pathAndFile.c_str(), pic );

            if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), NULL, resultStudy.instanceUID, resultPatient.oid, resultStudy.oid, destinationSeriesOID.c_str(), NULL ) )
              std::cout << "ChiliPlugin-Warning: Error while saving File (" << fileName << ") to Database." << std::endl;
            else
              if( remove( pathAndFile.c_str() ) != 0 ) std::cout << "ChiliPlugin-Warning: Not able to  delete file: " << pathAndFile << std::endl;
            myPicDescriptorList.pop_front();
            count++;
          }
        }
        else
        //save text
        {
          //Searching for possible Writer
          std::list<FileWriter::Pointer> possibleWriter;
          std::list<LightObject::Pointer> allObjects = itk::ObjectFactoryBase::CreateAllInstance( "IOWriter" );
          for( std::list<LightObject::Pointer>::iterator iter = allObjects.begin(); iter != allObjects.end(); iter++ )
          {
            FileWriter* writer = dynamic_cast<FileWriter*>( iter->GetPointer() );
            if( writer )
              possibleWriter.push_back( writer );
            else std::cout << "ChiliPlugin-Warning: no FileWriter returned" << std::endl;
          }

          for( std::list<FileWriter::Pointer>::iterator it = possibleWriter.begin(); it != possibleWriter.end(); it++ )
          {
            if( it->GetPointer()->CanWrite( (*nodeIter) ) )
            {
              std::string fileName;
              if( (*nodeIter)->GetProperty( "name" )->GetValueAsString() != "" )
                fileName = (*nodeIter)->GetProperty( "name" )->GetValueAsString();
              else fileName = "FileUpload";

              std::string textOID;
              BaseProperty::Pointer propertyTextOID = (*nodeIter)->GetProperty( "TextOID" );
              if( propertyTextOID )
                textOID = propertyTextOID->GetValueAsString();
              else
                textOID = "";

              fileName = fileName + it->GetPointer()->GetFileExtension();

              std::string pathAndFile = m_tempDirectory + fileName;

              it->GetPointer()->SetFileName( pathAndFile.c_str() );
              it->GetPointer()->SetInput( (*nodeIter) );
              it->GetPointer()->Write();

              if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), it->GetPointer()->GetWritenMIMEType().c_str(), resultStudy.instanceUID, resultPatient.oid, resultStudy.oid, destinationSeriesOID.c_str(), textOID.c_str() ) )
                std::cout << "ChiliPlugin-Warning: Error while saving File (" << fileName << ") to Database." << std::endl;
              else if( remove(  pathAndFile.c_str() ) != 0 ) std::cout << "ChiliPlugin-Warning: Not able to  delete file: " << it->GetPointer()->GetFileName() << std::endl;
            }
          }
        }
      }
    }
  }
  clearStudyStruct( &resultStudy );
  clearPatientStruct( &resultPatient );
  QApplication::restoreOverrideCursor();
*/
#endif
}

mitk::ChiliPlugin::TagInformationList mitk::ChiliPluginImpl::GetAllAvailableStudyAndPatientPicTags( const std::string& studyOID )
{
  TagInformationList resultList;
  resultList.clear();

#ifdef CHILI_PLUGIN_VERSION_CODE

  study_t study;
  patient_t patient;

  initStudyStruct( &study );
  initPatientStruct( &patient );

  study.oid = strdup( studyOID.c_str() );
  if( !pQueryStudy( this, &study, &patient ) )
  {
    clearStudyStruct( &study );
    clearPatientStruct( &patient );
  }

  //get all study- and patient-information
  if( &study != NULL && &patient != NULL )
  {
    TagInformationStruct temp;

    //study
    temp.PicTagDescription = tagPERFORMING_PHYSICIAN_NAME;
    temp.PicTagContent = study.performingPhysician;
    resultList.push_back( temp );
    temp.PicTagDescription = tagREFERING_PHYSICIAN_NAME;
    temp.PicTagContent = study.referingPhysician;
    resultList.push_back( temp );
    temp.PicTagDescription = tagINSTITUTION_NAME;
    temp.PicTagContent = study.institutionName;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_INSTANCE_UID;
    temp.PicTagContent = study.instanceUID;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_DESCRIPTION;
    temp.PicTagContent = study.description;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_DATE;
    temp.PicTagContent = study.date;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_TIME;
    temp.PicTagContent = study.time;
    resultList.push_back( temp );
    temp.PicTagDescription = tagSTUDY_ID;
    temp.PicTagContent = study.id;
    resultList.push_back( temp );
    temp.PicTagDescription = tagMODALITY;
    temp.PicTagContent = study.modality;
    resultList.push_back( temp );
    temp.PicTagDescription = tagMANUFACTURER;
    temp.PicTagContent = "DKFZ.MBI";  //TODO subversion-number, chili-number???
    resultList.push_back( temp );
    temp.PicTagDescription = tagMODEL_NAME;
    temp.PicTagContent = study.manufacturersModelName;
    resultList.push_back( temp );
    //patient
    temp.PicTagDescription = tagPATIENT_NAME;
    temp.PicTagContent = patient.name;
    resultList.push_back( temp );
    temp.PicTagDescription = tagPATIENT_SEX;
    temp.PicTagContent = patient.sex;
    resultList.push_back( temp );
    temp.PicTagDescription = tagPATIENT_ID;
    temp.PicTagContent = patient.id;
    resultList.push_back( temp );
    temp.PicTagDescription = tagPATIENT_BIRTHDATE;
    temp.PicTagContent = patient.birthDate;
    resultList.push_back( temp );
    temp.PicTagDescription = tagPATIENT_BIRTHTIME;
    temp.PicTagContent = patient.birthTime;
    resultList.push_back( temp );
    temp.PicTagDescription = tagMEDICAL_RECORD_LOCATOR;
    temp.PicTagContent = patient.medicalRecordLocator;
    resultList.push_back( temp );
  }
  else
  {
    std::cout << "ChiliPlugin(GetAllAvailableStudyAndPatientPicTags): The Study- and/or Patient-Information are empty." << std::endl;
    resultList.clear();
  }

  clearStudyStruct( &study );
  clearPatientStruct( &patient );

#endif

  return resultList;
}

/** event if the lightbox get tiles */
void mitk::ChiliPluginImpl::lightboxTiles( QcLightboxManager *lbm, int tiles )
{
  m_LightBoxCount = tiles;

  for( int row = 0; row < tiles; ++row )
  {
    try
    {
      QIDToolButton* button = m_LightBoxImportButtons.at(row);
      button->show();
    }
    catch(...) {}
  }
  for( int row = tiles; row < NUMBER_OF_THINKABLE_LIGHTBOXES; ++row )
  {
    try
    {
      QIDToolButton* button = m_LightBoxImportButtons.at(row);
      button->hide();
    }
    catch(...) {}
  }

  SendLightBoxCountChangedEvent(); // tell the application
}

/** event if a new study get selected */
void mitk::ChiliPluginImpl::studySelected( study_t* study )
{
  static bool done = false;
  if( !done ) // this is done here, beause if it's done in the constructor, then many button labels get wrong... (qt names as labels)
  {
    // toplevel layout
    QHBoxLayout* horzlayout = new QHBoxLayout( task );
    QVBoxLayout* vertlayout = new QVBoxLayout( horzlayout );

    // 4 lightbox import buttons
    for (unsigned int row = 0; row < NUMBER_OF_THINKABLE_LIGHTBOXES; ++row)
    {
      QIDToolButton* toolbutton = new QIDToolButton( row, task );
      toolbutton->setUsesTextLabel( true );
      QToolTip::add( toolbutton, "Import this lightbox to MITK");
      toolbutton->setTextPosition( QToolButton::BelowIcon );
      toolbutton->setPixmap( QPixmap(chili_lightbox_import_xpm) );

      QSizePolicy policy = toolbutton->sizePolicy();
      policy.setVerStretch( 2 );
      toolbutton->setSizePolicy( policy );

      connect( toolbutton, SIGNAL(clicked(int)), this, SLOT(lightBoxImportButtonClicked(int)));

      m_LightBoxImportButtons.push_back( toolbutton ); // we need to know these for import of lightboxes
      vertlayout->addWidget( toolbutton );

      if (row < m_LightBoxCount)
      {
        toolbutton->show();
      }
      else
      {
        toolbutton->hide();
      }
    }

    // unused toggle button. How to hide without integrating the hide/show button into the actual application?
    m_LightBoxImportToggleButton = new QToolButton( task );
    m_LightBoxImportToggleButton->setText(" "); // just some space that generates one short empty line of text
    //m_LightBoxImportToggleButton->setToggleButton(true);
    //m_LightBoxImportToggleButton->setPixmap( QPixmap(chili_lightbox_import) );
    m_LightBoxImportToggleButton->setAutoRaise(true);
    m_LightBoxImportToggleButton->setEnabled(true); //!! secret button to reinit application
    connect( m_LightBoxImportToggleButton, SIGNAL(clicked()), this, SLOT(OnApproachReinitializationOfWholeApplication()) );
    QSizePolicy policy = m_LightBoxImportToggleButton->sizePolicy();
    policy.setVerStretch( 1 );
    m_LightBoxImportToggleButton->setSizePolicy( m_LightBoxImportToggleButton->sizePolicy());
    vertlayout->addWidget( m_LightBoxImportToggleButton );

    // create one instance of SampleApp
    app = new SampleApp( task, "sample", 0 );
    horzlayout->addWidget( app ); // fromRow, toRow, fromCol, toCol

    done = true;
  }
}

/** button to import the lightbox */
void mitk::ChiliPluginImpl::lightBoxImportButtonClicked(int row)
{
  if( m_InImporting ) return;
  QPtrList<QcLightbox>& lightboxes = QcPlugin::lightboxManager()->getLightboxes();
  QcLightbox* selectedLightbox = lightboxes.at(row);

  if( selectedLightbox )
  {
    m_InImporting = true;
    //dont use "LoadCompleteSeries( ... )", the Buttons called LightboxImportButtons, so use the LoadImagesFromLightbox-Function, too

    //for images
    std::vector<DataTreeNode::Pointer> resultNodes = LoadImagesFromLightbox( selectedLightbox );
    for( unsigned int n = 0; n < resultNodes.size(); n++ )
    {
      mitk::DataStorage::GetInstance()->Add( resultNodes[n] );
    }

#ifdef CHILI_PLUGIN_VERSION_CODE
    //for text
    resultNodes = LoadAllTextsFromSeries( selectedLightbox->currentSeries()->oid );
    for( unsigned int n = 0; n < resultNodes.size(); n++ )
      DataStorage::GetInstance()->Add( resultNodes[n] );
#endif

    // stupid, that this is still necessary
    DataTreePreOrderIterator treeiter( app->GetTree() );
    app->GetMultiWidget()->InitializeStandardViews( &treeiter );
    app->GetMultiWidget()->Fit();
    app->GetMultiWidget()->ReInitializeStandardViews();
    RenderingManager::GetInstance()->RequestUpdateAll();
    qApp->processEvents();
    m_InImporting = false;
  }
}

/** if you want to show the current selected study, you have to know when they changed */
/*
void mitk::ChiliPluginImpl::SendStudySelectedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginStudySelected() );
}
*/

/** throw an event, if the lightboxcount changed */
void mitk::ChiliPluginImpl::SendLightBoxCountChangedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginLightBoxCountChanged() );
}

/** reinitialize the application */
void mitk::ChiliPluginImpl::OnApproachReinitializationOfWholeApplication()
{
  static int clickCount = 0;

  clickCount++;

  if (clickCount > 10)
  {
    if ( QMessageBox::question( NULL, tr("MITK"), 
                                  QString("Desparate clicking suggests a strong wish to restart the whole MITK plugin.\n\nDo you want to restart MITK?"),
                                  QMessageBox::Yes | QMessageBox::Default, 
                                  QMessageBox::No  | QMessageBox::Escape
                                ) == QMessageBox::Yes )
    {
      QMessageBox::information(NULL, "MITK", QString("Still unimplemented..."), QMessageBox::Ok);
    }

    clickCount = 0;
  }
}

/** teleconference methods */

void mitk::ChiliPluginImpl::connectPartner()
{
  ConferenceToken* ct = ConferenceToken::GetInstance();
  ct->ArrangeToken();
}

void mitk::ChiliPluginImpl::disconnectPartner()
{
  ConferenceToken* ct = ConferenceToken::GetInstance();
  ct->SetToken( 0 );
}

void mitk::ChiliPluginImpl::handleMessage( ipInt4_t type, ipMsgParaList_t *list )
{
  //QString message;
  //QString arguments;

  ipInt4_t eventID;
  ipFloat4_t w1,w2,w3,p1,p2;
  ipInt4_t sender,receiver,etype,estate,ebuttonstate,ekey;
  char *str = NULL;

  switch( type )
  {
    case m_QmitkChiliPluginConferenceID + MITKc:
      ipMsgListToVar( list,
                      ipTypeInt4, &eventID,
                      ipTypeStringPtr, &str,
                      ipTypeInt4, &etype, 
                      ipTypeInt4, &estate, 
                      ipTypeInt4, &ebuttonstate, 
                      ipTypeInt4, &ekey, 
                      ipTypeFloat4, &w1,
                      ipTypeFloat4, &w2,
                      ipTypeFloat4, &w3,
                      ipTypeFloat4, &p1,
                      ipTypeFloat4, &p2);

      if(!ConferenceEventMapper::MapEvent(eventID, str,etype,estate,ebuttonstate,ekey,w1,w2,w3,p1,p2))
        //std::cout<<"EventMaper no funciona"<<std::endl;

    break;
    case m_QmitkChiliPluginConferenceID +QTc :
      ipMsgListToVar( list,
                      ipTypeStringPtr, &str );
      //std::cout<<"CONFERENCE: (1) "<<str;
      if( ! EventCollector::PostEvent(QString(str), task))
      //std::cout<<" NO SUCCES!"<<std::endl;
      //std::cout<<std::endl;

    break;
    case m_QmitkChiliPluginConferenceID + ARRANGEc:
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &sender );
        //std::cout<<"CONFERENCE TOKEN ID:  "<<sender<<std::endl;

        ConferenceToken* ct = ConferenceToken::GetInstance();
        ct->ArrangeToken(sender);
        }
    break;
    case m_QmitkChiliPluginConferenceID + TOKENREQUESTc:
        //FROM: AskForToken(...)
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &receiver );
        //std::cout<<"CONFERENCE TOKEN REQUEST FROM:  "<<receiver<<std::endl;

        ConferenceToken* ct = ConferenceToken::GetInstance();
        ct->GetToken(receiver);
        }
    break;
    case m_QmitkChiliPluginConferenceID + TOKENSETc:
        //FROM: AskForToken(...)
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &sender,
                        ipTypeInt4, &receiver );
        //std::cout<<"CONFERENCE TRY SET TOKEN FROM:  "<<sender<<" TO:"<<receiver<<std::endl;

        ConferenceToken* ct = ConferenceToken::GetInstance();
        ct->SetToken( receiver );
        }
    break;
    case m_QmitkChiliPluginConferenceID + MOUSEMOVEc:
      ipMsgListToVar( list,
                      ipTypeStringPtr, &str,
                      ipTypeFloat4, &w1,
                      ipTypeFloat4, &w2,
                      ipTypeFloat4, &w3
          );

      if( !ConferenceEventMapper::MapEvent( str, w1, w2, w3 ) )
        //std::cout<<"ChiliPluginImpl::handleMessage() -> MouseMoveEvent couldn't pass through"<<std::endl;

    break;   
    default:
//       message = QString( " type %1" ).arg( type );
//       arguments = "argumente unbekannt";
      break;
  }
}

#ifdef WIN32

#include <windows.h>
#using <mscorlib.dll>
#using <System.dll>


// ist das ein c-sharp beispiel?? der namespace ist nicht c++
//using namespace System.IO;

/** create a temporary directory for windows */
std::string mitk::ChiliPluginImpl::GetTempDirectory()
{
/* TODO not tested yet

  std::string tempPath, resultTempPath;

  tempPath = GetTempPath();
  if( tempPath == NULL )
    tempPath = "C:\Temp";
  resultTempPath = GetRandomFileName();
  resultTempPath = tempPath + "\\" + resultTempPath;
  if( CreateDirectory( resultTempPath, NULL ) )
    return newTmpDirectory;
  else*/
 return "C:\Temp";
}

/*
  DWORD dwRetVal;
  DWORD dwBufSize = BUFSIZE; // length of the buffer
  char lpPathBuffer[BUFSIZE]; // buffer for path

  // Get the temp path
  dwRetVal = GetTempPath( dwBufSize, lpPathBuffer );
  std::string newTmpDirectory;
  if (dwRetVal > dwBufSize || lpPathBuffer == NULL )
    newTmpDirectory = "C:\TEMP";
  else
    newTmpDirectory = lpPathBuffer;
*/

#else

#include <stdlib.h>

/** create a temporary directory for linux */
std::string mitk::ChiliPluginImpl::GetTempDirectory()
{
  char *tmpDirectory = getenv( "TMPDIR" ); //return NULL if the TempDirectory is "/tmp/"
  std::string newTmpDirectory;
  if( tmpDirectory == NULL )
    newTmpDirectory = "/tmp/ChiliTempXXXXXX"; //mkdtemp need 6-10 dummies
  else
  {
    newTmpDirectory = tmpDirectory;
    newTmpDirectory = newTmpDirectory + "/ChiliTempXXXXXX";
  }
  tmpDirectory = mkdtemp( (char*)newTmpDirectory.c_str() ); //create a new unique directory
  if( tmpDirectory != NULL )
  {
    newTmpDirectory = tmpDirectory;
    newTmpDirectory = newTmpDirectory + "/";
    return newTmpDirectory;
  }
  else
    return "";
}

#endif
