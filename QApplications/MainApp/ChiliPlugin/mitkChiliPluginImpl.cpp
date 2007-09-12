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

//Chili
//#include <ipDicom/ipDicom.h>  //read DICOM-Files
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
#include <chili/qclightboxmanager.h>  //get newLightbox, currentLightbox
#include <ipPic/ipPic.h>  //ipPicDescriptor
#include <ipPic/ipPicTags.h>  //Tags
//MITK-Plugin
#include "mitkChiliPluginImpl.h"
#include "mitkChiliPluginEvents.h"
#include "mitkChiliPluginFactory.h"
#include "mitkPicDescriptorToNode.h"
#include "QmitkChiliPluginSaveDialog.h"
#include "mitkLightBoxImageReader.h"  //TODO entfernen wenn das neue Chili-Release installiert ist
//MITK
#include <mitkCoreObjectFactory.h>
#include <QmitkStdMultiWidget.h>
#include <QmitkEventCollector.h>
#include <QcMITKTask.h>
#include <SampleApp.h>
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

#include <mitk_chili_plugin.xpm>
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
  m_LightBoxCount( 0 ),
  m_InImporting (false)
{
  task = new QcMITKTask( xpm(), s_Parent, name() );

  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  m_FileList.clear();

  m_tempDirectory = GetTempDirectory();
  if( m_tempDirectory.empty() )
    QMessageBox::information( 0, "MITK", "MITK was not able to create a Temp-Directory.\nYou can only Load and Save via Lightbox.\nMore should not be possible." );
  else
    std::cout << "ChiliPlugin: Create and use directory "<< m_tempDirectory << std::endl;
}

/** destructor */
mitk::ChiliPluginImpl::~ChiliPluginImpl()
{
  static bool deleteOnlyOneTime = false; // the destructor get called two times
  if( !deleteOnlyOneTime )
  {
    deleteOnlyOneTime = true;
    // delete the created TempDirectory
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
  return (const char **)mitk_chili_plugin_xpm;
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

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return resultInformation;

#else

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
#endif
}

/** return the patientinformation */
mitk::ChiliPlugin::PatientInformation mitk::ChiliPluginImpl::GetPatientInformation( const std::string& seriesOID )
{
  PatientInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
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
      patient = (*dupPatientStruct( pCurrentPatient() ) );  //copy patientstruct
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
#endif
}

/** return the seriesinformation */
mitk::ChiliPlugin::SeriesInformation mitk::ChiliPluginImpl::GetSeriesInformation( const std::string& seriesOID )
{
  SeriesInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return resultInformation;

#else

  series_t series;
  initSeriesStruct( &series );

  if( seriesOID == "" )
  {
    //use current selected series
    if( pCurrentSeries() != NULL )
      series = (*dupSeriesStruct( pCurrentSeries() ) );  //copy seriesstruct
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
#endif
}

/** return the seriesinformationlist */
mitk::ChiliPlugin::SeriesInformationList mitk::ChiliPluginImpl::GetSeriesInformationList( const std::string& studyOID )
{
  //get used to save all found series
  m_SeriesInformationList.clear();
  //iterate over all series from study
  iterateSeries( (char*)studyOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateSeriesCallback, this );
  //the function filled the SeriesInformationList
  return m_SeriesInformationList;
}

/** iterate over all series from one study */
ipBool_t mitk::ChiliPluginImpl::GlobalIterateSeriesCallback( int rows, int row, series_t* series, void* user_data )
{
  mitk::ChiliPluginImpl* callingObject = static_cast<mitk::ChiliPluginImpl*>(user_data);
  //create new element
  mitk::ChiliPlugin::SeriesInformation newSeries;
  //fill element
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
#ifdef CHILI_PLUGIN_VERSION_CODE
  newSeries.ImageCount = series->image_count;
#else
  newSeries.ImageCount = 0;
#endif
  //add to list
  callingObject->m_SeriesInformationList.push_back( newSeries );

  return ipTrue; // enum from ipTypes.h
}

/** return the seriesinformation */
mitk::ChiliPlugin::TextInformation mitk::ChiliPluginImpl::GetTextInformation( const std::string& textOID )
{
  TextInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return resultInformation;

#else

  if( textOID == "" )
    return resultInformation;

  text_t text;
  initTextStruct( &text );

  //let chili search for series
  text.oid = strdup( textOID.c_str() );
  if( !pQueryText( this, &text, NULL, NULL, NULL ) )
  {
    clearTextStruct( &text );
    std::cout << "ChiliPlugin (GetTextInformation): pQueryText() failed. Abort." << std::endl;
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
#endif
}

/** return the textinformationlist */
mitk::ChiliPlugin::TextInformationList mitk::ChiliPluginImpl::GetTextInformationList( const std::string& seriesOID )
{
  //get used to save all found text
  m_TextInformationList.clear();
#ifdef CHILI_PLUGIN_VERSION_CODE
  //iterate over all text from series
  pIterateTexts( this, (char*)seriesOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateTextOneCallback, this );
  //the function filled the TextInformationList
#endif
  return m_TextInformationList;
}

#ifdef CHILI_PLUGIN_VERSION_CODE
/** iterate over all texts from one series */
ipBool_t mitk::ChiliPluginImpl::GlobalIterateTextOneCallback( int rows, int row, text_t *text, void *user_data )
{
  mitk::ChiliPluginImpl* callingObject = static_cast<mitk::ChiliPluginImpl*>(user_data);
  //create new element
  mitk::ChiliPlugin::TextInformation resultText;
  //fill element
  resultText.OID = text->oid;
  resultText.MimeType = text->mime_type;
  resultText.ChiliText = text->chiliText;
  resultText.Status = text->status;
  resultText.FrameOfReferenceUID = text->frameOfReferenceUID;
  resultText.TextDate = text->text_date;
  resultText.Description = text->description;
  //add to list
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
    QMessageBox::information( 0, "MITK", "There is no selected Lightbox in Chili." );
  return currentLightbox;
}

/** load the image from the lightbox */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPluginImpl::LoadImagesFromLightbox( QcLightbox* lightbox )
{
  std::vector<DataTreeNode::Pointer> resultVector;
  resultVector.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  //QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
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
    mitk::PropertyList::Pointer tempPropertyList = reader->GetImageTagsAsPropertyList();
    //add properties to node
    for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
    {
      node->SetProperty( iter->first.c_str(), iter->second.first );
    }
    resultVector.push_back( node );
  }
  QApplication::restoreOverrideCursor();
  return resultVector;

#else

  if( lightbox == NULL )
  {
    lightbox = GetCurrentLightbox();
    if( lightbox == NULL )
      return resultVector;  //abort
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
    PicDescriptorToNode::Pointer converterToNode = PicDescriptorToNode::New();
    //input - processing - output - delete
    converterToNode->SetInput( lightboxPicDescriptorList, lightbox->currentSeries()->oid );
    converterToNode->Update();
    resultVector = converterToNode->GetOutput();

    QApplication::restoreOverrideCursor();
  }
  return resultVector;
#endif
}

/** load images- and text-files from series */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPluginImpl::LoadCompleteSeries( const std::string& seriesOID )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return resultNodes;

#else

  if( m_tempDirectory.empty() )
    return resultNodes;

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

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return resultNodes;

#else
  if( m_tempDirectory.empty() )
    return resultNodes;

  if( seriesOID == "" )
  {
    std::cout << "ChiliPlugin (LoadAllImagesFromSeries): No SeriesOID set. Do you select a Series?" << std::endl;
    return resultNodes;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  //this is the list of all loaded files from chili
  m_FileList.clear();

  //iterate over all images from this series, save them to harddisk and set all filenames to m_FileList
  iterateImages( (char*)seriesOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateImagesCallbackOne, this );

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
        picFileList.push_back( m_FileList.front() );
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
          std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << picFileList.front().c_str() << std::endl;
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
        std::cout<< "ChiliPlugin (LoadAllImagesFromSeries): Could not get header." <<std::endl;

      if( !image )
        std::cout<< "ChiliPlugin (LoadAllImagesFromSeries): Could not get image." <<std::endl;

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
      PicDescriptorToNode::Pointer converterToNode = PicDescriptorToNode::New();
      //input - processing - output
      converterToNode->SetInput( picDescriptorToNodeInput, seriesOID );
      converterToNode->Update();
      resultNodes = converterToNode->GetOutput();
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
                node->SetProperty( "name", new StringProperty( "no description" ) );
              else
                node->SetProperty( "name", new StringProperty( series.description ) );
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
              std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << otherFileList.front().c_str() << std::endl;
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
ipBool_t mitk::ChiliPluginImpl::GlobalIterateImagesCallbackOne( int rows, int row, image_t* image, void* user_data )
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
    std::cout << "ChiliPlugin (GlobalIterateImagesCallbackOne): ChiliError: " << error << ", while reading file (" << image->path << ") from Database." << std::endl;
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

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return resultNodes;

#else

  if( m_tempDirectory.empty() )
    return resultNodes;

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

  DataTreeNode::Pointer tempNode;
  //use the found oid's and path's to load
  while( !m_TextFileList.empty() )
  {
    tempNode = LoadOneText( seriesOID, m_TextFileList.front().TextFileOID, m_TextFileList.front().TextFilePath );
    if( tempNode.IsNotNull() )
      resultNodes.push_back( tempNode );
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

mitk::DataTreeNode::Pointer mitk::ChiliPluginImpl::LoadOneText( const std::string& textOID )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return NULL;

#else

  if( m_tempDirectory.empty() )
    return NULL;

  if( textOID == "" )
  {
    std::cout << "ChiliPlugin (LoadOneTextFromSeries): No Text-OID set. Abort." << std::endl;
    return NULL;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  text_t text;
  series_t series;
  initTextStruct( &text );
  initSeriesStruct( &series );

  text.oid = strdup( textOID.c_str() );
  if( !pQueryText( this, &text, &series, NULL, NULL ) )
  {
    clearTextStruct( &text );
    clearSeriesStruct( &series );
    std::cout << "ChiliPlugin (GetTextInformation): pQueryText() failed. Abort." << std::endl;
    QApplication::restoreOverrideCursor();
    return NULL;
  }

  mitk::DataTreeNode::Pointer result = LoadOneText( series.oid, text.oid, text.chiliText );
  clearTextStruct( &text );
  clearSeriesStruct( &series );
  QApplication::restoreOverrideCursor();
  return result;
#endif
}

mitk::DataTreeNode::Pointer mitk::ChiliPluginImpl::LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return NULL;

#else

  if( m_tempDirectory.empty() )
    return NULL;

  if( seriesOID == "" || textOID == "" ||textPath == "" )
  {
    std::cout << "ChiliPlugin (LoadOneText): Wrong textOID, seriesOID or textPath. Abort." << std::endl;
    return NULL;
  }

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  //get Filename and path to save to harddisk
  char* tempFileName;
  tempFileName = strrchr( textPath.c_str(), '-' );
  //we want no "-" in front of the filename
  tempFileName++;

  std::string fileName = tempFileName;
  std::string pathAndFile = m_tempDirectory + fileName;

  //Load File from DB
  ipInt4_t error;
  pFetchDataToFile( textPath.c_str(), pathAndFile.c_str(), &error );
  if( error != 0 )
  {
    std::cout << "ChiliPlugin (LoadOneText): ChiliError: " << error << ", while reading file (" << fileName << ") from Database." << std::endl;
    QApplication::restoreOverrideCursor();
    return NULL;
  }

  //if there are no fileextension in the filename, the reader not able to load the file and the plugin crashed
  char* fileExtension;
  fileExtension = strrchr( tempFileName, '.' );
  if( fileExtension == NULL )
  {
    //if the user dont close chili, he can look at the file in the tempdirectory, therefore the file downloaded first and checked afterwards
    std::cout << "ChiliPlugin (LoadOneText): Reader not able to read file without extension.\nIf you dont close Chili you can find the file here: "<< pathAndFile <<"." << std::endl;
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
          fileExtension = strrchr( textPath.c_str(), '.' );
          //cut the fileExtension from the filename
          int size = fileName.size() - sizeof( fileExtension );
          fileName.erase( size );
          //set the filename without extension as name-property
          node->SetProperty( "name", new StringProperty( fileName ) );
          node->SetProperty( "TextOID", new StringProperty( textOID ) );
          node->SetProperty( "SeriesOID", new StringProperty( seriesOID ) );
          //it should be possible to override all non-image-entries
          node->SetProperty( "CHILI: MANUFACTURER", new StringProperty( "MITK" ) );
          node->SetProperty( "CHILI: INSTITUTION NAME", new StringProperty( "DKFZ.MBI" ) );

          if( remove(  pathAndFile.c_str() ) != 0 )
            std::cout << "ChiliPlugin (LoadOneTextFromSeries): Not able to  delete file: " << pathAndFile << std::endl;

          QApplication::restoreOverrideCursor();
          return node;
        }
      }
    }
    catch ( itk::ExceptionObject & ex )
      itkGenericOutputMacro( << "Exception during file open: " << ex );
  }
  QApplication::restoreOverrideCursor();
  return NULL;
#endif
}

/** save image- and text-files to the chili-database */
void mitk::ChiliPluginImpl::SaveToChili( DataStorage::SetOfObjects::ConstPointer inputNodes )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return;

#else

  if( m_tempDirectory.empty() || inputNodes->begin() == inputNodes->end() )
    return;

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  QmitkChiliPluginSaveDialog chiliPluginDialog( 0 );

  //add all nodes to the dialog
  for( DataStorage::SetOfObjects::const_iterator nodeIter = inputNodes->begin(); nodeIter != inputNodes->end(); nodeIter++ )
  {
    if( (*nodeIter) )
    {
      mitk::BaseProperty::Pointer seriesOIDProperty = (*nodeIter)->GetProperty( "SeriesOID" );

      if( seriesOIDProperty.IsNotNull() )
      {
        //this nodes are loaded from chili
        PatientInformation tempPatient = GetPatientInformation( seriesOIDProperty->GetValueAsString() );
        StudyInformation tempStudy = GetStudyInformation( seriesOIDProperty->GetValueAsString() );
        SeriesInformation tempSeries = GetSeriesInformation( seriesOIDProperty->GetValueAsString() );

        chiliPluginDialog.AddStudySeriesAndNode( tempStudy.OID, tempPatient.Name, tempPatient.ID, tempStudy.Description, tempSeries.OID, tempSeries.Number, tempSeries.Description, (*nodeIter) );
      }
      else
      {
        //this nodes saved the first time to chili
        chiliPluginDialog.AddNode( (*nodeIter) );
      }
    }
  }

  //add the current selected study/patient/series
  PatientInformation currentselectedPatient = GetPatientInformation();
  StudyInformation currentselectedStudy = GetStudyInformation();
  SeriesInformation currentselectedSeries = GetSeriesInformation();
  if( currentselectedSeries.OID == "" )
  {
    chiliPluginDialog.AddStudy( currentselectedStudy.OID, currentselectedPatient.Name, currentselectedPatient.ID, currentselectedStudy.Description );
  }
  else
    chiliPluginDialog.AddStudyAndSeries( currentselectedStudy.OID, currentselectedPatient.Name, currentselectedPatient.ID, currentselectedStudy.Description, currentselectedSeries.OID, currentselectedSeries.Number, currentselectedSeries.Description );

  //let the user decide
  chiliPluginDialog.UpdateView();
  QApplication::restoreOverrideCursor();
  int dialogReturnValue = chiliPluginDialog.exec();

  if( dialogReturnValue == QDialog::Rejected )
    return; //user abort

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  if( chiliPluginDialog.GetSelection().UserDecision == QmitkChiliPluginSaveDialog::CreateNew )
  {
    SaveAsNewSeries( inputNodes, chiliPluginDialog.GetSelection().StudyOID, chiliPluginDialog.GetSeriesInformation().SeriesNumber, chiliPluginDialog.GetSeriesInformation().SeriesDescription );
  }
  else
  {
    if( chiliPluginDialog.GetSelection().UserDecision == QmitkChiliPluginSaveDialog::OverrideAll )
      SaveToSeries( inputNodes, chiliPluginDialog.GetSelection().StudyOID, chiliPluginDialog.GetSelection().SeriesOID, true );
    else
      SaveToSeries( inputNodes, chiliPluginDialog.GetSelection().StudyOID, chiliPluginDialog.GetSelection().SeriesOID, false );
  }
  QApplication::restoreOverrideCursor();
#endif
}

void mitk::ChiliPluginImpl::SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, std::string studyOID, int seriesNumber, std::string seriesDescription )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return;

#else

  if( m_tempDirectory.empty() || inputNodes->begin() == inputNodes->end() )
    return;

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  study_t study;
  initStudyStruct( &study );

  if( studyOID != "" )
  {
    study.oid = strdup( studyOID.c_str() );
    if( pQueryStudy( this, &study, NULL ) )
    {
      //create new series
      series_t* newSeries = ( series_t* )malloc( sizeof( series_t ) );
      initSeriesStruct( newSeries );
      newSeries->description = (char*)seriesDescription.c_str();
      newSeries->number = seriesNumber;
      if( pCreateSeries( this, &study, newSeries ) )
      {
        SaveToSeries( inputNodes, study.oid, newSeries->oid, false );
      }
      else
        std::cout << "ChiliPlugin (SaveToChili): Can not create a new Series." << std::endl;
      delete newSeries;
    }
    else
      std::cout << "ChiliPlugin (SaveAsNewSeries): pQueryStudy failed. Abort." << std::endl;
  }
  else
    std::cout << "ChiliPlugin (SaveAsNewSeries): studyOID is empty. Abort." << std::endl;

  clearStudyStruct( &study );
  QApplication::restoreOverrideCursor();
#endif
}

void mitk::ChiliPluginImpl::SaveToSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, std::string studyOID, std::string seriesOID, bool overrideExistingSeries )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, youre current CHILI version does not support this function." );
  return;

#else

  if( m_tempDirectory.empty() || inputNodes->begin() == inputNodes->end() )
    return;

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  study_t study;
  patient_t patient;
  series_t series;
  initStudyStruct( &study );
  initPatientStruct( &patient );
  initSeriesStruct( &series );

  bool abort = false;
  if( seriesOID == "" )
  {
    std::cout << "ChiliPlugin (SaveToSeries): seriesOID is empty. Abort." << std::endl;
    abort = true;
  }
  else
  {
    series.oid = strdup( seriesOID.c_str() );
    if( !pQuerySeries( this, &series, &study, &patient ) )
    {
      std::cout << "ChiliPlugin (SaveToSeries): pQuerySeries failed. Abort." << std::endl;
      abort = true;
    }
    else
      if( study.oid != studyOID )
      {
        std::cout << "ChiliPlugin (SaveToSeries): The given StudyOID is different to the StudyOID of the found Series. Abort." << std::endl;
        abort = true;
      }
  }

  if( abort )
  {
    clearStudyStruct( &study );
    clearPatientStruct( &patient );
    clearSeriesStruct( &series );
    QApplication::restoreOverrideCursor();
    return;
  }

  int imageNumberCount = GetMaximumImageNumber( seriesOID ) + 1;

  mitk::ImageToPicDescriptor::TagInformationList picTagList = GetNeededTagList( &study, &patient, &series );

  for( DataStorage::SetOfObjects::const_iterator nodeIter = inputNodes->begin(); nodeIter != inputNodes->end(); nodeIter++ )
  {
    if( (*nodeIter) )
    {
      BaseData* data = (*nodeIter)->GetData();
      if ( data )
      {
        Image* image = dynamic_cast<Image*>( data );
        if( image )
        //save Image
        {
          mitk::BaseProperty::Pointer seriesOIDProperty = (*nodeIter)->GetProperty( "SeriesOID" );
          if( seriesOIDProperty && seriesOIDProperty->GetValueAsString() == seriesOID && !overrideExistingSeries )
            continue;

          //ImageToPicDescriptor
          ImageToPicDescriptor::Pointer converterToDescriptor = ImageToPicDescriptor::New();
          LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<LevelWindowProperty*>( (*nodeIter)->GetProperty("levelwindow").GetPointer() );
          if( levelWindowProperty.IsNotNull() )
          {
            converterToDescriptor->SetLevelWindow( levelWindowProperty->GetLevelWindow() );
          }
          converterToDescriptor->SetImage( image );
          converterToDescriptor->SetImageNumber( imageNumberCount );

          //add the node-name as SeriesDescription to the Taglist
          mitk::BaseProperty::Pointer nameProperty = (*nodeIter)->GetProperty( "name" );
          mitk::ImageToPicDescriptor::TagInformationStruct temp;
          temp.PicTagDescription = tagSERIES_DESCRIPTION;
          temp.PicTagContent = nameProperty->GetValueAsString();
          picTagList.push_back( temp );

          if( overrideExistingSeries && seriesOIDProperty && seriesOIDProperty->GetValueAsString() == seriesOID )
            converterToDescriptor->SetTagList( picTagList, true );
          else
            converterToDescriptor->SetTagList( picTagList, false );
          //create the picdescriptorlist
          converterToDescriptor->Update();
          std::list< ipPicDescriptor* > myPicDescriptorList = converterToDescriptor->GetOutput();

          //increase the imageNumber
          imageNumberCount = imageNumberCount + myPicDescriptorList.size();
          //delete the current node-name (SeriesnDescription) from the picTagList
          picTagList.pop_back();

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
            //get PicDescriptor
            ipPicDescriptor *pic = myPicDescriptorList.front();
            //save to harddisk
            ipPicPut( (char*)pathAndFile.c_str(), pic );
            //delete Descriptor
            ipPicFree( pic );
            myPicDescriptorList.pop_front();
            //save saved file to chili
            if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), NULL, NULL, study.instanceUID, patient.oid, study.oid, series.oid, NULL ) )
            {
              std::cout << "ChiliPlugin (SaveToChili): Error while saving File (" << fileName << ") to Database." << std::endl;
            }
            else
            {
              if( remove( pathAndFile.c_str() ) != 0 )
                std::cout << "ChiliPlugin (SaveToChili): Not able to  delete file: " << pathAndFile << std::endl;
            }
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
            else std::cout << "ChiliPlugin (SaveToChili): no FileWriter returned" << std::endl;
          }

          //write/save
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
              if( overrideExistingSeries && propertyTextOID )
                  textOID = propertyTextOID->GetValueAsString();
              else
                textOID = "";

              fileName = fileName + it->GetPointer()->GetFileExtension();

              std::string pathAndFile = m_tempDirectory + fileName;

              it->GetPointer()->SetFileName( pathAndFile.c_str() );
              it->GetPointer()->SetInput( (*nodeIter) );
              it->GetPointer()->Write();

              if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), it->GetPointer()->GetWritenMIMEType().c_str(), NULL, study.instanceUID, patient.oid, study.oid, series.oid, textOID.c_str() ) )
              {
                std::cout << "ChiliPlugin (SaveToChili): Error while saving File (" << fileName << ") to Database." << std::endl;
              }
              else
                 if( remove(  pathAndFile.c_str() ) != 0 )
                   std::cout << "ChiliPlugin (SaveToChili): Not able to  delete file: " << it->GetPointer()->GetFileName() << std::endl;
            }
          }
        }
      }
    }
  }
  clearStudyStruct( &study );
  clearPatientStruct( &patient );
  clearSeriesStruct( &series );
  QApplication::restoreOverrideCursor();
#endif
}

int mitk::ChiliPluginImpl::GetMaximumImageNumber( std::string seriesOID )
{
  m_MaximumImageNumber = 0;
  iterateImages( (char*)seriesOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateImagesCallbackTwo, this );
  return m_MaximumImageNumber;
}

/** function to iterate over all images from a series */
ipBool_t mitk::ChiliPluginImpl::GlobalIterateImagesCallbackTwo( int rows, int row, image_t* image, void* user_data )
{
  ChiliPluginImpl* callingObject = static_cast<ChiliPluginImpl*>( user_data );
  if( image->number > callingObject->m_MaximumImageNumber )
    callingObject->m_MaximumImageNumber = image->number;
  return ipTrue; // enum from ipTypes.h
}

mitk::ImageToPicDescriptor::TagInformationList mitk::ChiliPluginImpl::GetNeededTagList( study_t* study, patient_t* patient, series_t* series )
{
  mitk::ImageToPicDescriptor::TagInformationList resultList;
  resultList.clear();

#ifdef CHILI_PLUGIN_VERSION_CODE

  //get all study- and patient-information
  if( study != NULL && patient != NULL && series != NULL )
  {
    mitk::ImageToPicDescriptor::TagInformationStruct temp;
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
  else
  {
    std::cout << "ChiliPlugin(GetAllAvailablePicTags): One input was empty." << std::endl;
    resultList.clear();
  }
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
void mitk::ChiliPluginImpl::SendStudySelectedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginStudySelected() );
}

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
//#using <mscorlib.dll>
//#using <System.dll>


// ist das ein c-sharp beispiel?? der namespace ist nicht c++
//using namespace System.IO;

/** create a temporary directory for windows */
std::string mitk::ChiliPluginImpl::GetTempDirectory()
{
/* TODO not tested and implemented yet

  std::string tempPath, resultTempPath;

  tempPath = GetTempPath();
  if( tempPath == NULL )
    tempPath = "C:\Temp";
  resultTempPath = GetRandomFileName();
  resultTempPath = tempPath + "\\" + resultTempPath;
  if( CreateDirectory( resultTempPath, NULL ) )
    return newTmpDirectory;
  else*/
 return "C:\\Temp";
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
