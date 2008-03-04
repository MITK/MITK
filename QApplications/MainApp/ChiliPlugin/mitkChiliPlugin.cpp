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

//Chili
#include <chili/isg.h>  //geometry
#include <ipDicom/ipDicom.h>  //read DICOM-Files
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
#include <chili/qclightboxmanager.h>  //get newLightbox, currentLightbox
#include <ipPic/ipPic.h>  //ipPicDescriptor
#include <ipPic/ipPicTags.h>  //Tags
//MITK-Plugin
#include "mitkChiliPlugin.h"
#include "mitkChiliPluginEvents.h"
#include "mitkChiliPluginFactory.h"
#include "mitkPicDescriptorToNode.h"
#include "mitkImageNumberFilter.h"
#include "mitkSingleSpacingFilter.h"
#include "mitkStreamReader.h"
#include "mitkSpacingSetFilter.h"
#include "QmitkChiliPluginSaveDialog.h"
#include "mitkLightBoxImageReader.h"  //TODO entfernen wenn das neue Chili-Release installiert ist
#include "mitkChiliMacros.h"

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
#include <qtimer.h>
#include <qobjectlist.h>
#include <qprogressbar.h>

#include <mitk_chili_plugin.xpm>
#include "chili_lightbox_import.xpm"
#include "chili_restart_mitk.xpm"

QWidget* mitk::ChiliPlugin::s_Parent = NULL;
/** Create event-logging object (ConferenceKit). */
Chili3ConferenceKitFactory chiliconferencekitfactory;

/** Entry point for Chili. */
extern "C"
QcEXPORT QObject* create( QWidget *parent )
{
  // save parent for parameter-less constructor
  mitk::ChiliPlugin::SetQtParent( parent );

  // overwrite implementation of mitk::ChiliPlugin with mitk::ChiliPlugin
  mitk::ChiliPluginFactory::RegisterOneFactory();

  return mitk::ChiliPlugin::GetQcPluginInstance();
}

/** Constructor with hidden parameter s_Parent. */
mitk::ChiliPlugin::ChiliPlugin()
: QcPlugin( s_Parent ),
  app( NULL ),
  m_UseReader( 0 ),
  m_LightBoxCount( 0 ),
  m_InImporting (false)
{
  task = new QcMITKTask( xpm(), s_Parent, name() );

  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  m_tempDirectory = GetTempDirectory();

  if( m_tempDirectory.empty() )
    QMessageBox::information( 0, "MITK", "MITK was not able to create a temporary directory.\nYou can only load data from CHILI using the yellow light box import buttons." );
  else
    std::cout << "ChiliPlugin: Create and use directory "<< m_tempDirectory << std::endl;
}

/** Destructor */
mitk::ChiliPlugin::~ChiliPlugin()
{
  std::string removeDirectory = "";

  #ifdef WIN32
    removeDirectory = "rmdir /S /Q " + m_tempDirectory;
    std::cout << "ChiliPlugin: Delete directory "<< m_tempDirectory << std::endl;
  #else
    removeDirectory = "rm -r " + m_tempDirectory;
    std::cout << "ChiliPlugin: Delete directory "<< m_tempDirectory << std::endl;
  #endif

  system( removeDirectory.c_str() );

  task->deleteLater();

  // tricky tricky...
  m_ReferenceCountLock.Lock();
  ++m_ReferenceCount; // increase count, so the following notice to mitk::ChiliPlugin won't trigger another delete on "this"
  m_ReferenceCountLock.Unlock();

  PACSPlugin::GetInstance(true); // set internal s_Instance = NULL, so ITK won't have another SmartPointer to us (hopefully nobody else has)

  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0; // set count to 0, so itk::LightObject won't trigger another delete...
  m_ReferenceCountLock.Unlock();
}

QString mitk::ChiliPlugin::name()
{
  return QString( "MITK" );
}

const char** mitk::ChiliPlugin::xpm()
{
  return (const char **)mitk_chili_plugin_xpm;
}

/** Set the qtparent, needed for QObject* create(). */
void mitk::ChiliPlugin::SetQtParent( QWidget* parent )
{
  s_Parent = parent;
}

/** Create a new SampleApp. */
void mitk::ChiliPlugin::CreateSampleApp()
{
  static bool done = false;
  if( !done ) // this is done here, beause if it's done in the constructor, then many button labels get wrong... (qt names as labels)
  {
    // toplevel layout
    horzlayout = new QHBoxLayout( task ); // member
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

    // button for application restart
    m_LightBoxImportToggleButton = new QToolButton( task );
    m_LightBoxImportToggleButton->setText(" "); // just some space that generates one short empty line of text
    m_LightBoxImportToggleButton->setPixmap( QPixmap(chili_restart_mitk_xpm) );
    m_LightBoxImportToggleButton->setAutoRaise(true);
    m_LightBoxImportToggleButton->setEnabled(true); //!! secret button to reinit application
    connect( m_LightBoxImportToggleButton, SIGNAL(clicked()), this, SLOT(OnApproachReinitializationOfWholeApplication()) );
    QSizePolicy policy = m_LightBoxImportToggleButton->sizePolicy();
    policy.setVerStretch( 1 );
    m_LightBoxImportToggleButton->setSizePolicy( m_LightBoxImportToggleButton->sizePolicy());
    vertlayout->addWidget( m_LightBoxImportToggleButton );
  }

  if (app)
  {
    // delete old application
    horzlayout->remove(app);
    delete app;
  }

  // create one instance of SampleApp
  app = new SampleApp( task, "sample", 0 );
  horzlayout->addWidget( app ); 
  horzlayout->activate();
  app->show();

  if (!done)
  {
    done = true;
  }

  SendStudySelectedEvent();
}

/** Return the plugininstance. */
QcPlugin* mitk::ChiliPlugin::GetQcPluginInstance()
{
  // give Chili the single instance of mitk::ChiliPlugin
  PACSPlugin::Pointer pluginInstance = PACSPlugin::GetInstance();
  ChiliPlugin::Pointer realPluginInstance = dynamic_cast<ChiliPlugin*>( pluginInstance.GetPointer() );

  static bool done = false;

  if (!done)
  {
    QTimer* timer = new QTimer(realPluginInstance);
    connect( timer, SIGNAL(timeout()), realPluginInstance, SLOT(CreateSampleApp()) );
    timer->start(4000, true);

    done = true;
  }

  return realPluginInstance.GetPointer();
}

/** Return the ConferenceID. */
int mitk::ChiliPlugin::GetConferenceID()
{
  return m_QmitkChiliPluginConferenceID;
}

/** Return true if the application run as plugin and false if its standalone. */
bool mitk::ChiliPlugin::IsPlugin()
{
  return true;
}

/** STUDY-, PATIENT-, SERIES- AND TEXT-INFORMATIONS */

/** Return the studyinformation. If you dont set the seriesOID, you get the current selected study. If you want a specific study, set the OID. If no study could found, this function return StudyInformation.OID == "". */
mitk::PACSPlugin::StudyInformation mitk::ChiliPlugin::GetStudyInformation( const std::string& mitkHideIfNoVersionCode(seriesOID) )
{
  StudyInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your CHILI version does not support this function (GetStudyInformation)." );
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
      std::cout << "ChiliPlugin (GetStudyInformation): pQuerySeries() failed. Abort." << std::endl;
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

/** Return the patientinformation. If you dont set the seriesOID, you get the current selected patient. If you want a specific patient, set the OID. If no patient could found, this function return PatientInformation.OID == "". */
mitk::PACSPlugin::PatientInformation mitk::ChiliPlugin::GetPatientInformation( const std::string& mitkHideIfNoVersionCode(seriesOID) )
{
  PatientInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetPatientInformation)." );
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

/** Return the seriesinformation. If you dont set the seriesOID, you get the current selected series. If you want a specific series, set the OID. If no series could found, this function return SeriesInformation.OID == "". */
mitk::PACSPlugin::SeriesInformation mitk::ChiliPlugin::GetSeriesInformation( const std::string& mitkHideIfNoVersionCode(seriesOID) )
{
  SeriesInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetSeriesInformation)." );
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

/** Return the seriesinformationlist. If you dont set the studyOID, you get the seriesList of the current selected study. If you want a list of a specific study, set the OID. If no series could found, this function returns an empty list. */
mitk::PACSPlugin::SeriesInformationList mitk::ChiliPlugin::GetSeriesInformationList( const std::string& studyOID )
{
  //get used to save all found series
  m_SeriesInformationList.clear();
  //iterate over all series from study
  if( studyOID == "" )
    iterateSeries( (char*)GetStudyInformation().OID.c_str(), NULL, &ChiliPlugin::GlobalIterateSeriesForCompleteInformation, this );
  else
    iterateSeries( (char*)studyOID.c_str(), NULL, &ChiliPlugin::GlobalIterateSeriesForCompleteInformation, this );
  //the function filled the SeriesInformationList
  return m_SeriesInformationList;
}

/** This function Iterate over all series of one study and save the series-properties to m_SeriesInformationList. */
ipBool_t mitk::ChiliPlugin::GlobalIterateSeriesForCompleteInformation( int /*rows*/, int /*row*/, series_t* series, void* user_data )
{
  mitk::ChiliPlugin* callingObject = static_cast<mitk::ChiliPlugin*>(user_data);
  //create new element
  mitk::PACSPlugin::SeriesInformation newSeries;
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

/** Return the textinformation. You have to set the textOID to get the information. If no text could found, this function return TextInformation.OID == "". */
mitk::PACSPlugin::TextInformation mitk::ChiliPlugin::GetTextInformation( const std::string& mitkHideIfNoVersionCode(textOID) )
{
  TextInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetTextInformation)." );
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

/** Return a list of all textinformation. You have to set the seriesOID to get the textList. If no texts could found, this function returns an empty list. This function dont return the text which used to save and load the parent-child-relationship. */
mitk::PACSPlugin::TextInformationList mitk::ChiliPlugin::GetTextInformationList( const std::string& mitkHideIfNoVersionCode(seriesOID) )
{
  //get used to save all found text
  m_TextInformationList.clear();
#ifdef CHILI_PLUGIN_VERSION_CODE
  //iterate over all text from series
  pIterateTexts( this, (char*)seriesOID.c_str(), NULL, &ChiliPlugin::GlobalIterateTextForCompleteInformation, this );
  //the function filled the TextInformationList
#endif
  return m_TextInformationList;
}

#ifdef CHILI_PLUGIN_VERSION_CODE
/** This function Iterate over all texts of one series and save the text-properties to m_TextInformationList. */
ipBool_t mitk::ChiliPlugin::GlobalIterateTextForCompleteInformation( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  mitk::ChiliPlugin* callingObject = static_cast<mitk::ChiliPlugin*>(user_data);

  //we dont want that the parent-child-relation-file can be load ;)
  std::string chiliDatabaseName = text->chiliText;
  std::string fileName = chiliDatabaseName.substr( chiliDatabaseName.find_last_of("-") + 1 );
  if( fileName != "ParentChild.xml" )
  {
    //create new element
    mitk::PACSPlugin::TextInformation resultText;
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
  }
  return ipTrue; // enum from ipTypes.h
}
#endif

/** Return the number of current shown lightboxes and set by lightboxTiles(). */
unsigned int mitk::ChiliPlugin::GetLightboxCount()
{
  return m_LightBoxCount;
}

/** Return the first found empty chililightbox, using the lightboxmanager therefore. */
QcLightbox* mitk::ChiliPlugin::GetNewLightbox()
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
  QMessageBox::information( 0, "MITK", "Application asked for a new lightbox. CHILI cannot handle more than 4 lightboxes, so please close one first." );
  return NULL;
}

/** Return the current selected chililightbox, using the lightboxmanager therefore. */
QcLightbox* mitk::ChiliPlugin::GetCurrentLightbox()
{
  QcLightbox* currentLightbox = lightboxManager()->getActiveLightbox();
  if( currentLightbox == NULL)
    QMessageBox::information( 0, "MITK", "Application asked for the currently active lightbox, but there is none." );
  return currentLightbox;
}

/** Set the internal reader which used to combine slices.
0: PicDescriptorToNode(ImageNumber);
1: PicDescriptorToNodeSecond(SpacingCombination);
2: PicDescriptorToNodeThird(most common spacing) */
void mitk::ChiliPlugin::SetReaderType( unsigned int readerType )
{
  m_UseReader = readerType;
}

/** In the Parent-Child-XML-File are volumes saved. This function return all volumes to a given seriesOID */
mitk::PACSPlugin::PSRelationInformationList mitk::ChiliPlugin::GetSeriesRelationInformation( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  PSRelationInformationList result;
  result.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetSeriesRelationInformation)." );
  return result;

#else

  if( m_tempDirectory.empty() || seriesOID.empty() )
    return result;

  //init the right parent-child-xml-file
  StudyInformation currentStudy = GetStudyInformation( seriesOID );
  if( currentStudy.OID != "" && InitParentChild( currentStudy.OID ) )
  {
    TiXmlElement* volume = m_currentXmlDoc->FirstChildElement("volumes");
    if( !volume ) return result;

    //check every volume
    for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
    {
      if( singleVolume->FirstAttribute() )
      {
        //search for label, id and oid
        std::string label = "";
        std::string id = "";
        std::string oid = "";
        for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
        {
          std::string reference = search->Name();
          if( reference == "label" )
            label = search->Value();
          else
            if( reference == "id" )
              id = search->Value();
            else
              if( reference== "seriesOID" )
                oid = search->Value();
        }

        if( oid == seriesOID )  //if the saved oid equal the searched one
        {
          //save element
          PSRelationInformation newElement;
          newElement.label = label;
          newElement.id = id;
          newElement.oid = oid;
          newElement.parentLabel.clear();
          newElement.childLabel.clear();

          //is it image or text?
          TiXmlElement* singleID = singleVolume->FirstChildElement();
          std::string identification = singleID->Value();
          if( identification == "TextOID" )
            newElement.image = false;
          else newElement.image = true;

          result.push_back( newElement );
        }
      }
    }
  }
  return result;
#endif
}

mitk::PACSPlugin::PSRelationInformationList mitk::ChiliPlugin::GetStudyRelationInformation( const std::string& mitkHideIfNoVersionCode( studyOID ) )
{
  PSRelationInformationList result;
  result.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetStudyRelationInformation)." );
  return result;

#else

  if( m_tempDirectory.empty() )
    return result;

  bool okay = false;
  if( studyOID == "" )
  {
    if( pCurrentStudy() )
      okay = InitParentChild( pCurrentStudy()->oid );
  }
  else
    okay = InitParentChild( studyOID );

  //init the right parent-child-xml-file
  if( okay )
  {
    TiXmlElement* volume = m_currentXmlDoc->FirstChildElement("volumes");
    TiXmlElement* relation = m_currentXmlDoc->FirstChildElement("relations");
    if( !volume || !relation ) return result;

    //check every volume
    for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
    {
      if( singleVolume->FirstAttribute() )
      {
        //search for label, id and oid
        std::string label = "";
        std::string id = "";
        std::string oid = "";
        for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
        {
          std::string reference = search->Name();
          if( reference == "label" )
            label = search->Value();
          else
            if( reference == "id" )
              id = search->Value();
            else
              if( reference == "seriesOID" )
                oid = search->Value();
        }

        //save element
        PSRelationInformation newElement;
        newElement.label = label;
        newElement.id = id;
        newElement.oid = oid;
        newElement.parentLabel.clear();
        newElement.childLabel.clear();

        //is it image or text?
        TiXmlElement* singleID = singleVolume->FirstChildElement();
        std::string identification = singleID->Value();
        if( identification == "TextOID" )
          newElement.image = false;
        else newElement.image = true;

        //get relation
        for( TiXmlElement* singleRelation = relation->FirstChildElement(); singleRelation; singleRelation = singleRelation->NextSiblingElement() )
        {
          std::string volumeLabel = singleRelation->GetText();
          std::string parent = volumeLabel.substr( 0, volumeLabel.find(" ") );
          std::string child = volumeLabel.substr( volumeLabel.find(" ") + 1 );
          if( child == newElement.label )
            newElement.parentLabel.push_back( parent );
          if( parent == newElement.label )
            newElement.childLabel.push_back( child );
        }

        result.push_back( newElement );
      }
    }
  }
  return result;
#endif
}

/** LOAD-FUNCTIONS */

/** This function load single elements from the parent-child-xml-file. */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadParentChildElement( const std::string& mitkHideIfNoVersionCode( seriesOID ), const std::string& mitkHideIfNoVersionCode( label ) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadParentChildElement)." );
  return NULL;

#else

  if( m_tempDirectory.empty() || seriesOID.empty() )
    return NULL;

  //get studyinformation
  StudyInformation currentStudy = GetStudyInformation( seriesOID );
  if( currentStudy.OID == "" ) return NULL;
  //init the right parent-child-xml-file
  if( InitParentChild( currentStudy.OID ) )
  {
    TiXmlElement* volume = m_currentXmlDoc->FirstChildElement("volumes");
    if( !volume) return NULL;

    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    //search for the right element
    for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
    {
      if( singleVolume->FirstAttribute() )
      {
        //search for label and oid
        std::string labelAtt = "";
        std::string oidAtt = "";
        for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
        {
          std::string reference = search->Name();
          if( reference == "label" )
            labelAtt = search->Value();
          else
            if( reference == "seriesOID" )
              oidAtt = search->Value();
        }

        //check if the seriesOID and label are equal
        if( oidAtt == seriesOID && labelAtt == label )
        {
          m_SavedImageInstanceUIDs.clear();
          for( TiXmlElement* singleID = singleVolume->FirstChildElement(); singleID; singleID = singleID->NextSiblingElement() )
          {
            std::string identification = singleID->Value();
            std::string content = singleID->GetText();
            if( identification == "TextOID" )  //if the found element are a text-file -> load them
            {
              QApplication::restoreOverrideCursor();
              return LoadOneText( content );
            }
            else
              if( identification == "ImageInstanceUID" )  //if the found element are a image-volume -> load the single slices
                m_SavedImageInstanceUIDs.push_back( content );
          }

          //with the image_instance_UIDs of the slices whe have to load the ipPicDescriptors
          m_ImageList.clear();
          iterateImages( (char*)seriesOID.c_str(), NULL, &ChiliPlugin::GlobalIterateLoadOnlySingleImages, this );

          //with the ipPicDescriptors we can create a mitk::Image
          PicDescriptorToNode::Pointer converterToNode;
          converterToNode = SingleSpacingFilter::New();
          converterToNode->SetInput( m_ImageList, seriesOID );
          converterToNode->Update();
          std::vector<mitk::DataTreeNode::Pointer> tempResult = converterToNode->GetOutput();
          if( !tempResult.empty() )
          {
            //check if volume alway known in parent-child-xml
            std::vector< std::list< std::string > > ImageInstanceUIDs = converterToNode->GetImageInstanceUIDs();
            std::string result = CheckForVolumeLabel( ImageInstanceUIDs[0] );
            if( result != "" )
              tempResult[0]->SetProperty( "VolumeLabel", new StringProperty( result ) );

            QApplication::restoreOverrideCursor();
            return tempResult[0];  //there are only single-volumes saved in the xml-file, therefore the first result get used
          }
        }
      }
    }
    QApplication::restoreOverrideCursor();
  }
  return NULL;
#endif

//TODO add function to check all datatreenodes at the datatree and set the relations between them (the nodes have a property "VolumeLabel", this property is equal to the saved xml-label of the volumes; use them to set the relations)
}

/** This function return the ipPicDescriptors for the image_instance_uids saved at the m_SavedImageInstanceUIDs-list. */
ipBool_t mitk::ChiliPlugin::GlobalIterateLoadOnlySingleImages( int /*rows*/, int mitkHideIfNoVersionCode(row), image_t* mitkHideIfNoVersionCode(image), void* mitkHideIfNoVersionCode(user_data) )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  ChiliPlugin* callingObject = static_cast<ChiliPlugin*>( user_data );

  //if the current image_instance_UID exist in the m_SavedImageInstanceUIDs-list, this image get saved to the m_ImageList
  if( find( callingObject->m_SavedImageInstanceUIDs.begin(), callingObject->m_SavedImageInstanceUIDs.end(), image->instanceUID ) != callingObject->m_SavedImageInstanceUIDs.end() )
  {
    //get the FileExtension
    std::string imagePath = image->path;
    std::string fileExtension = imagePath.substr( imagePath.find_last_of(".") + 1 );

    //create a new FileName (1.*, 2.*, 3.*, ...) with FileExtension from DB
    std::ostringstream stringHelper;
    stringHelper << row << "." << fileExtension;
    std::string pathAndFile = callingObject->m_tempDirectory + stringHelper.str();

    //save to harddisk
    ipInt4_t error;
    pFetchDataToFile( image->path, pathAndFile.c_str(), &error );
    //all right?
    if( error != 0 )
      std::cout << "ChiliPlugin (GlobalIterateLoadImage): ChiliError: " << error << ", while reading file (" << image->path << ") from Database." << std::endl;
    else
    {
      if( fileExtension == "pic" )
      {
        ipPicDescriptor *pic;
        pic = ipPicGet( (char*)pathAndFile.c_str(), NULL );
        if( pic != NULL )
        {
          callingObject->m_ImageList.push_back( pic );
          if( remove(  pathAndFile.c_str() ) != 0 )
            std::cout << "ChiliPlugin (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
        }
      }
      else
      {
        if( fileExtension == "dcm" )
        {
          // read dicom-files
          ipUInt1_t* header = NULL;
          ipUInt4_t  header_size;
          ipUInt1_t* dcimage = NULL;
          ipUInt4_t  image_size;

          dicomGetHeaderAndImage( (char*)pathAndFile.c_str(), &header, &header_size, &dcimage, &image_size );

          if( header && dcimage )
          {
            //ipPicDescriptor *pic = dicomToPic( header, header_size, image, image_size );
            ipPicDescriptor *pic = _dicomToPic( header, header_size, dcimage, image_size, ipFalse, ipTrue, ipTrue);

            if( pic != NULL)
            {
              callingObject->m_ImageList.push_back( pic );
              if( remove(  pathAndFile.c_str() ) != 0 )
                std::cout << "ChiliPlugin (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
            }
          }
          else std::cout<< "ChiliPlugin (GlobalIterateLoadImage): Could not get header or image." <<std::endl;
        }
        else
        {
          callingObject->unknownImageFormatPath.push_back( pathAndFile );
        }
      }
    }
  }
#endif
  return ipTrue; // enum from ipTypes.h
}

/** Load all images from the given lightbox. If no lightbox set, the current lightbox get used. Chili dont support text-access via lightbox. If you want to load them, use LoadAllTextsFromSeries(...). The slices get combined with the internal set ReaderType. */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromLightbox( QcLightbox* lightbox )
{
  std::vector<DataTreeNode::Pointer> resultVector;
  resultVector.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  //QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function." );
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
    m_StreamImageList.clear();

    //read all frames from Lightbox
    for( unsigned int n = 0; n < lightbox->getFrames(); n++ )
    {
      ipPicDescriptor* pic = lightbox->fetchPic(n);

      #ifndef WIN32
      if( pic->dim == 1 )  //jpeg-stream
      {
        unsigned int frameNumber;
        ipPicTSV_t *tsv;
        void* data = NULL;
        ipUInt4_t len = 0;
        tsv = ipPicQueryTag( pic, (char*)"SOURCE HEADER" );
        if( tsv )
        {
          if( dicomFindElement( (unsigned char*) tsv->value, 0x0028, 0x0008, &data, &len ) )
          {
            if( data != NULL )
            {
              StreamImageStruct newElement;
              newElement.imageList.clear();

              sscanf( (char *) data, "%d", &frameNumber );

              ipUInt4_t *offset_table;
              offset_table = (ipUInt4_t *)malloc( sizeof(ipUInt4_t) * frameNumber );
              for( unsigned int i = 0; i < frameNumber; ++i )
                offset_table[i] = 0xffffffff;

              for( unsigned int i = 0; i < frameNumber; i++ )
              {
                ipPicDescriptor* cinePic = ipPicDecompressJPEG( lightbox->fetchPic(n), i, frameNumber, NULL, offset_table );
                newElement.imageList.push_back( cinePic );
              }
              free( offset_table );

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
                if( tsv )
                {
                  if( dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
                  {
                    newElement.seriesDescription = (char*)data;
                  }
                }
              }
              m_StreamImageList.push_back( newElement );
            }
          }
        }
      }
      else
      #endif
        lightboxPicDescriptorList.push_back( pic );
    }

    if( !lightboxPicDescriptorList.empty() )
    {
      PicDescriptorToNode::Pointer converterToNode;
      switch ( m_UseReader )
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
          std::cout << "ChiliPlugin (LoadImagesFromLightbox): ReaderType undefined." << std::endl;
          return resultVector;
        }
      }

      converterToNode->SetInput( lightboxPicDescriptorList, lightbox->currentSeries()->oid );
      converterToNode->Update();
      resultVector = converterToNode->GetOutput();

      //Init parent-child
      StudyInformation currentStudy = GetStudyInformation( lightbox->currentSeries()->oid );
      if( InitParentChild( currentStudy.OID ) )
      {
        //check if volume alway known in parent-child-xml
        std::vector< std::list< std::string > > ImageInstanceUIDs = converterToNode->GetImageInstanceUIDs();
        for( unsigned int x = 0; x < ImageInstanceUIDs.size(); x++ )
        {
          std::string result = CheckForVolumeLabel( ImageInstanceUIDs[x] );
          if( result != "" )
            resultVector[x]->SetProperty( "VolumeLabel", new StringProperty( result ) );
        }
      }
    }

    if( !m_StreamImageList.empty() )
    {
      StreamReader::Pointer streamReader = StreamReader::New();

      for( unsigned x = 0; x < m_StreamImageList.size(); x++ )
      {
        streamReader->SetInput( m_StreamImageList[x].imageList , lightbox->currentSeries()->oid );
        streamReader->SetSecondInput( m_StreamImageList[x].geometry, m_StreamImageList[x].seriesDescription );
        streamReader->Update();
        resultVector.push_back( streamReader->GetOutput()[0] );

        //delete the ipPicDescriptors and interSliceGeometry
        for( std::list<ipPicDescriptor*>::iterator imageIter = m_StreamImageList[x].imageList.begin(); imageIter != m_StreamImageList[x].imageList.end(); imageIter++ )
          ipPicFree( (*imageIter) );
        free( m_StreamImageList[x].geometry );
      }
      m_StreamImageList.clear();
    }
    QApplication::restoreOverrideCursor();
  }
  return resultVector;
#endif
}

/** Load all image- and text-files from the series. This function use LoadAllImagesFromSeries(...) and LoadAllTextsFromSeries(...). */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadCompleteSeries( const std::string& mitkHideIfNoVersionCode(seriesOID) )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadCompleteSeries)." );
  return resultNodes;

#else

  if( m_tempDirectory.empty() )
    return resultNodes;

  if( seriesOID == "" )
  {
    QMessageBox::information( 0, "MITK", "Attempt to load series without series ID. Is a series selected?" );
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

/** This function load all images from the given series(OID). This function load all files via FileDownload from chili. Chili save the files in the same file-format, like they saved on the server. That mean that *.pic or *.dcm are possible. Dicomfiles get transformed to pic. The slices get combined with the internal set ReaderType. Should other file-formats saved, they get load from harddisk with the DataTreeNodeFactory. */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadAllImagesFromSeries( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllImagesFromSeries)." );
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

  //different lists for different cases
  m_ImageList.clear();
  unknownImageFormatPath.clear();
  m_StreamImageList.clear();

  //iterate over all images from this series, save them to harddisk, load them and add the picDescriptors to m_ImageList, unknown imagefiles get saved to unknownImageFormatPath
  iterateImages( (char*)seriesOID.c_str(), NULL, &ChiliPlugin::GlobalIterateLoadImage, this );

  if( !m_ImageList.empty() )
  {
    //make the readed picDescriptors to multiple Nodes
    PicDescriptorToNode::Pointer converterToNode;

    switch ( m_UseReader )
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
        std::cout << "ChiliPlugin (LoadImagesFromLightbox): ReaderType undefined." << std::endl;
        return resultNodes;
      }
    }

    converterToNode->SetInput( m_ImageList, seriesOID );
    converterToNode->Update();
    resultNodes = converterToNode->GetOutput();

    //Init parent-child
    StudyInformation currentStudy = GetStudyInformation( seriesOID );
    if( InitParentChild( currentStudy.OID ) )
    {
      //check if volume alway known in parent-child-xml
      std::vector< std::list< std::string > > ImageInstanceUIDs = converterToNode->GetImageInstanceUIDs();
      for( unsigned int x = 0; x < ImageInstanceUIDs.size(); x++ )
      {
        std::string result = CheckForVolumeLabel( ImageInstanceUIDs[x] );
        if( result != "" )
          resultNodes[x]->SetProperty( "VolumeLabel", new StringProperty( result ) );
      }
    }

    //delete the loaded ipPicDescriptors
    for( std::list<ipPicDescriptor*>::iterator imageIter = m_ImageList.begin(); imageIter != m_ImageList.end(); imageIter++ )
    {
      ipPicFree( (*imageIter) );
    }
    m_ImageList.clear();
  }

  if( !m_StreamImageList.empty() )
  {
    StreamReader::Pointer streamReader = StreamReader::New();

    for( unsigned x = 0; x < m_StreamImageList.size(); x++ )
    {
      streamReader->SetInput( m_StreamImageList[x].imageList , seriesOID );
      streamReader->SetSecondInput( m_StreamImageList[x].geometry, m_StreamImageList[x].seriesDescription );
      streamReader->Update();
      resultNodes.push_back( streamReader->GetOutput()[0] );

      //delete the ipPicDescriptors and interSliceGeometry
      for( std::list<ipPicDescriptor*>::iterator imageIter = m_StreamImageList[x].imageList.begin(); imageIter != m_StreamImageList[x].imageList.end(); imageIter++ )
        ipPicFree( (*imageIter) );
      free( m_StreamImageList[x].geometry );
    }

    m_StreamImageList.clear();
  }

  //read the rest via DataTreeNodeFactory
  if( !unknownImageFormatPath.empty() )
  {
    DataTreeNodeFactory::Pointer factory = DataTreeNodeFactory::New();
    try
    {
      factory->SetFileName( unknownImageFormatPath.front().c_str() );
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
            if( series.description == NULL )
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
        while( !unknownImageFormatPath.empty() )
        {
          if( remove( unknownImageFormatPath.front().c_str() ) != 0 )
            std::cout << "ChiliPlugin (LoadAllImagesFromSeries): Not able to  delete file: " << unknownImageFormatPath.front().c_str() << std::endl;
          unknownImageFormatPath.pop_front();
        }
      }
    }
    catch ( itk::ExceptionObject & ex )
      itkGenericOutputMacro( << "Exception during file open: " << ex );
  }

  QApplication::restoreOverrideCursor();
  return resultNodes;
#endif
}

/** This function iterate over all images of a series. The single images get saved to harddisk and the filenames to a internal filelist. */
ipBool_t mitk::ChiliPlugin::GlobalIterateLoadImage( int /*rows*/, int mitkHideIfNoVersionCode(row), image_t* mitkHideIfNoVersionCode(image), void* mitkHideIfNoVersionCode(user_data) )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  ChiliPlugin* callingObject = static_cast<ChiliPlugin*>( user_data );

  //get the FileExtension
  std::string imagePath = image->path;
  std::string fileExtension = imagePath.substr( imagePath.find_last_of(".") + 1 );

  //create a new FileName (1.*, 2.*, 3.*, ...) with FileExtension from DB
  std::ostringstream stringHelper;
  stringHelper << row << "." << fileExtension;
  std::string pathAndFile = callingObject->m_tempDirectory + stringHelper.str();

  //save to harddisk
  ipInt4_t error;
  pFetchDataToFile( image->path, pathAndFile.c_str(), &error );
  //all right?
  if( error != 0 )
    std::cout << "ChiliPlugin (GlobalIterateLoadImage): ChiliError: " << error << ", while reading file (" << image->path << ") from Database." << std::endl;
  else
  {
    if( fileExtension == "pic" )
    {
      ipPicDescriptor *pic;
      pic = ipPicGet( (char*)pathAndFile.c_str(), NULL );
      if( pic != NULL )
      {

      #ifndef WIN32
        if( pic->dim == 1 )  //load cine-pics
        {
          unsigned int frameNumber;
          ipPicTSV_t *tsv;
          void* data = NULL;
          ipUInt4_t len = 0;
          tsv = ipPicQueryTag( pic, (char*)"SOURCE HEADER" );
          if( tsv )
          {
            if( dicomFindElement( (unsigned char*) tsv->value, 0x0028, 0x0008, &data, &len ) )
            {
              if( data != NULL )
              {
                StreamImageStruct newElement;
                newElement.imageList.clear();

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
                  if( tsv )
                  {
                    if( dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
                    {
                      newElement.seriesDescription = (char*)data;
                    }
                  }
                }
                callingObject->m_StreamImageList.push_back( newElement );
              }
            }
          }
        }
        else  //load single ipPicDescriptor
        #endif
          callingObject->m_ImageList.push_back( pic );

        if( remove(  pathAndFile.c_str() ) != 0 )
          std::cout << "ChiliPlugin (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
      }
    }
    else
    {
      if( fileExtension == "dcm" )
      {
        // read dicom-files
        ipUInt1_t* header = NULL;
        ipUInt4_t  header_size;
        ipUInt1_t* dcimage = NULL;
        ipUInt4_t  image_size;

        dicomGetHeaderAndImage( (char*)pathAndFile.c_str(), &header, &header_size, &dcimage, &image_size );

        if( header && dcimage )
        {
          //ipPicDescriptor *pic = dicomToPic( header, header_size, image, image_size );
          ipPicDescriptor *pic = _dicomToPic( header, header_size, dcimage, image_size, ipFalse, ipTrue, ipTrue);

          if( pic != NULL)
          {
            callingObject->m_ImageList.push_back( pic );
            if( remove(  pathAndFile.c_str() ) != 0 )
              std::cout << "ChiliPlugin (GlobalIterateLoadImage): Not able to  delete file: "<< pathAndFile << std::endl;
          }
        }
        else std::cout<< "ChiliPlugin (GlobalIterateLoadImage): Could not get header or image." <<std::endl;
      }
      else
      {
        callingObject->unknownImageFormatPath.push_back( pathAndFile );
      }
    }
  }
#endif
  return ipTrue; // enum from ipTypes.h
}

/** This function load all text-files from the series. Chili combine the filename, the OID, MimeType, ... to create the databasedirectory, so different files can be saved with the same filename. The filename from database is used to save the files. So we have to work sequently, otherwise we override the files ( twice filenames ). */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadAllTextsFromSeries( const std::string& mitkHideIfNoVersionCode(seriesOID) )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllTextsFromSeries)." );
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
  pIterateTexts( this, (char*)seriesOID.c_str(), NULL, &ChiliPlugin::GlobalIterateToLoadAllText, this );

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
/** This function iterate over all text-files from a series and save the text-path and text-oid to the m_TextFileList. */
ipBool_t mitk::ChiliPlugin::GlobalIterateToLoadAllText( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  //cast to chiliplugin to save the single textfileinformation
  ChiliPlugin* callingObject = static_cast<ChiliPlugin*>(user_data);

  //we dont want that to load the parent-child-relation-file
  std::string chiliDatabaseName = text->chiliText;
  std::string fileName = chiliDatabaseName.substr( chiliDatabaseName.find_last_of("-") + 1 );
  if( fileName != "ParentChild.xml" )
  {
    ChiliPlugin::TextFilePathAndOIDStruct newTextFile;
    newTextFile.TextFilePath = text->chiliText;
    newTextFile.TextFileOID = text->oid;
    callingObject->m_TextFileList.push_back( newTextFile );
  }
  return ipTrue; // enum from ipTypes.h
}
#endif

/** To load a single text-file, you need more than the textOID, this function search for the missing attributes and use LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath ). */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadOneText( const std::string& mitkHideIfNoVersionCode(textOID) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadOneText)." );
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

/** This function load a single text-file. The file get saved to harddisk, get readed via factory ( current: mitkImageWriterFactory, mitkPointSetWriterFactory, mitkSurfaceVtkWriterFactory ) to mitk and deleted from harddisk. */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadOneText( const std::string& mitkHideIfNoVersionCode(seriesOID), const std::string& mitkHideIfNoVersionCode(textOID), const std::string& mitkHideIfNoVersionCode(textPath) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadOneText)." );
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
  std::string fileName = textPath.substr( textPath.find_last_of("-") + 1 );
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
  if( fileName.find_last_of(".") == std::string::npos )
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
          std::string fileNameWithoutExtension = fileName.substr( 0, fileName.find_last_of(".") );
          //set the filename without extension as name-property
          node->SetProperty( "name", new StringProperty( fileNameWithoutExtension ) );
          node->SetProperty( "TextOID", new StringProperty( textOID ) );
          node->SetProperty( "SeriesOID", new StringProperty( seriesOID ) );
          //it should be possible to override all non-image-entries
          node->SetProperty( "CHILI: MANUFACTURER", new StringProperty( "MITK" ) );
          node->SetProperty( "CHILI: INSTITUTION NAME", new StringProperty( "DKFZ.MBI" ) );

          //Init parent-child
          StudyInformation currentStudy = GetStudyInformation( seriesOID );
          InitParentChild( currentStudy.OID );
          //check if volume alway known in parent-child-xml
          std::list< std::string > UID;
          UID.clear();
          UID.push_back( textOID );
          std::string result = CheckForVolumeLabel( UID );
          if( result != "" )
            node->SetProperty( "VolumeLabel", new StringProperty( result ) );

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

/** SAVE-FUNCTIONS */

/** This function provides a dialog where the user can decide if he want to create a new series, save to series, override, ... . Then SaveAsNewSeries(...) or SaveToSeries(...) get used. */
void mitk::ChiliPlugin::SaveToChili( DataStorage::SetOfObjects::ConstPointer mitkHideIfNoVersionCode(inputNodes) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToChili)." );
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

  bool override;
  if( chiliPluginDialog.GetSelection().UserDecision == QmitkChiliPluginSaveDialog::OverrideAll )
    override = true;
  else
    override = false;

  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  if( chiliPluginDialog.GetSelection().UserDecision == QmitkChiliPluginSaveDialog::CreateNew )
  {
    SaveAsNewSeries( inputNodes, chiliPluginDialog.GetSelection().StudyOID, chiliPluginDialog.GetSeriesInformation().SeriesNumber, chiliPluginDialog.GetSeriesInformation().SeriesDescription );
  }
  else
      SaveToSeries( inputNodes, chiliPluginDialog.GetSelection().SeriesOID, override );

  QApplication::restoreOverrideCursor();
#endif
}

/** This function create a new series and use the function SaveToSeries() . No dialog is used. */
void mitk::ChiliPlugin::SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer mitkHideIfNoVersionCode(inputNodes), const std::string& mitkHideIfNoVersionCode(studyOID), int mitkHideIfNoVersionCode(seriesNumber), const std::string& mitkHideIfNoVersionCode(seriesDescription) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveAsNewSeries)." );
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
        SaveToSeries( inputNodes, newSeries->oid, false );
      }
      else
        std::cout << "ChiliPlugin (SaveToChili): Can not create a new Series." << std::endl;
      free( newSeries );
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

/** This function save the nodes to via FileUpload to chili. */
void mitk::ChiliPlugin::SaveToSeries( DataStorage::SetOfObjects::ConstPointer mitkHideIfNoVersionCode(inputNodes), const std::string& mitkHideIfNoVersionCode(seriesOID), bool mitkHideIfNoVersionCode(overrideExistingSeries) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToSeries)." );
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
  }

  if( abort )
  {
    clearStudyStruct( &study );
    clearPatientStruct( &patient );
    clearSeriesStruct( &series );
    QApplication::restoreOverrideCursor();
    return;
  }

  //check study for parent-child-relation-file: if file exist, load this one; if no file exist, create a new on harddisk
  bool initParentChild = InitParentChild( study.oid );

  //we need a list of all saved nodes to create the parent-child-relation
  std::list<DataTreeNode::Pointer> xmlNodeList;
  xmlNodeList.clear();

  //get current highest imagenumber --> use as input for imgeToPicDescriptor
  int imageNumberCount = GetMaximumImageNumber( seriesOID ) + 1;

  //get the needed study-, patient- and seriesinformation to save
  mitk::ImageToPicDescriptor::TagInformationList picTagList = GetNeededTagList( &study, &patient, &series );

  //start and use every single node
  for( DataStorage::SetOfObjects::const_iterator nodeIter = inputNodes->begin(); nodeIter != inputNodes->end(); nodeIter++ )
  {
    if( (*nodeIter) )
    {
      BaseData* data = (*nodeIter)->GetData();
      if ( data )
      {
        mitk::BaseProperty::Pointer currentSeriesOID = (*nodeIter)->GetProperty( "SeriesOID" );
        mitk::BaseProperty::Pointer currentVolumeLabel = (*nodeIter)->GetProperty( "VolumeLabel" );

        Image* image = dynamic_cast<Image*>( data );
        if( image )
        //SAVE IMAGE
        {
          //check if the current data always exist in current series
          if( currentSeriesOID && currentSeriesOID->GetValueAsString() == seriesOID )
          {
            //if the always existing entries should not override, we have nothing to do
            if( !overrideExistingSeries )
              continue;
            else
            {
              mitk::BaseProperty::Pointer manufacturerProperty = (*nodeIter)->GetProperty( "CHILI: MANUFACTURER" );
              mitk::BaseProperty::Pointer institutionNameProperty = (*nodeIter)->GetProperty( "CHILI: INSTITUTION NAME" );
              //only data with "MANUFACTURER==MITK" and "INSTITUTION NAME==DKFZ.MBI" should be override
              if( !manufacturerProperty || !institutionNameProperty || manufacturerProperty->GetValueAsString() != "MITK" || institutionNameProperty->GetValueAsString() != "DKFZ.MBI" )
                continue;
            }
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
          mitk::BaseProperty::Pointer nameProperty = (*nodeIter)->GetProperty( "name" );
          mitk::ImageToPicDescriptor::TagInformationStruct temp;
          temp.PicTagDescription = tagSERIES_DESCRIPTION;
          temp.PicTagContent = nameProperty->GetValueAsString();
          picTagList.push_back( temp );

          //for override images use saved pic-tags -> true, for add images create new pic-tags
          if( overrideExistingSeries && currentSeriesOID && currentSeriesOID->GetValueAsString() == seriesOID )
            converterToDescriptor->SetTagList( picTagList, true );
          else
            converterToDescriptor->SetTagList( picTagList, false );

          //create the picdescriptorlist
          converterToDescriptor->Update();
          std::list< ipPicDescriptor* > myPicDescriptorList = converterToDescriptor->GetOutput();

          //add the volume to xml-file if: the volume have no Parent-Child-Label (currentVolumeLabel); the volume saved first time to CHILI(currentSeriesOID); the volume get saved new
          if( initParentChild && ( !currentVolumeLabel || !currentSeriesOID || ( currentSeriesOID && currentSeriesOID->GetValueAsString() != seriesOID ) ) )
          {
            std::list< std::string > CurrentImageInstanceUIDs = converterToDescriptor->GetSaveImageInstanceUIDs();
            AddNewEntryToXML( (*nodeIter), CurrentImageInstanceUIDs, seriesOID );
          }

          if( initParentChild )
            xmlNodeList.push_back( (*nodeIter) );

          //increase the imageNumber
          imageNumberCount = imageNumberCount + myPicDescriptorList.size();
          //delete the current node-name from the picTagList (added as SeriesDescription but the picTagList get used for the next image too)
          picTagList.pop_back();

          //save the single slices to CHILI
          pOpenAssociation();

          int count = 0;
          while( !myPicDescriptorList.empty() )
          {
            std::ostringstream helpFileName;
            if( (*nodeIter)->GetProperty( "name" )->GetValueAsString() != "" )
              helpFileName << (*nodeIter)->GetProperty( "name" )->GetValueAsString();
            else helpFileName << "FileUpload";
            helpFileName << count << ".pic";
            count++;
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
          }
          pCloseAssociation();
          //set the new SERIESOID as Property
          if( !currentSeriesOID )
            (*nodeIter)->SetProperty( "SeriesOID", new StringProperty( seriesOID ) );
          else
            (*nodeIter)->ReplaceProperty( "SeriesOID", new StringProperty( seriesOID ) );
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
            if( pQueryText( this, &text, &series, NULL, NULL ) )
            {
              if( strcmp( series.oid, seriesOID.c_str() ) == 0 )  //the SERIESOID from text and aim to save are equal
              {
                if( overrideExistingSeries )  //to override the OIDs have to be the same
                  textOID = currentTextOID->GetValueAsString().c_str();
                else  //the Text exist in current series and entries should not be override
                  continue;
              }
              else  //the SERIESOIDs are different
                textOID = pGetNewOID();
            }
            clearTextStruct( &text );
            clearSeriesStruct( &series );
          }
          else  //the Text-File saved first time
            textOID = pGetNewOID();

          //save Volume to Parent-Child-XML
          if( initParentChild && ( !currentVolumeLabel || !currentSeriesOID || ( currentSeriesOID && currentSeriesOID->GetValueAsString() != seriesOID ) ) )
          {
            std::list<std::string> inputList;
            inputList.clear();
            inputList.push_back(textOID);
            AddNewEntryToXML( (*nodeIter), inputList, seriesOID );
          }

          if( initParentChild )
            xmlNodeList.push_back( (*nodeIter) );

          //search for possible Writer
          std::list<FileWriter::Pointer> possibleWriter;
          std::list<LightObject::Pointer> allObjects = itk::ObjectFactoryBase::CreateAllInstance( "IOWriter" );
          for( std::list<LightObject::Pointer>::iterator iter = allObjects.begin(); iter != allObjects.end(); iter++ )
          {
            FileWriter* writer = dynamic_cast<FileWriter*>( iter->GetPointer() );
            if( writer )
              possibleWriter.push_back( writer );
            else std::cout << "ChiliPlugin (SaveToChili): no FileWriter returned" << std::endl;
          }

          //use Writer
          for( std::list<FileWriter::Pointer>::iterator it = possibleWriter.begin(); it != possibleWriter.end(); it++ )
          {
            if( it->GetPointer()->CanWrite( (*nodeIter) ) )
            {
              //create filename
              std::string fileName;
              if( (*nodeIter)->GetProperty( "name" )->GetValueAsString() != "" )
                fileName = (*nodeIter)->GetProperty( "name" )->GetValueAsString();
              else fileName = "FileUpload";
              //set extension
              fileName = fileName + it->GetPointer()->GetFileExtension();
              //path and filename
              std::string pathAndFile = m_tempDirectory + fileName;

              //save via Writer to harddisk
              it->GetPointer()->SetFileName( pathAndFile.c_str() );
              it->GetPointer()->SetInput( (*nodeIter) );
              it->GetPointer()->Write();

              //save file from harddisk to chili
              if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), it->GetPointer()->GetWritenMIMEType().c_str(), NULL, study.instanceUID, patient.oid, study.oid, series.oid, textOID.c_str() ) )
              {
                std::cout << "ChiliPlugin (SaveToChili): Error while saving File (" << fileName << ") to Database." << std::endl;
              }
              else
              {
                //delete saved file
                if( remove(  pathAndFile.c_str() ) != 0 )
                  std::cout << "ChiliPlugin (SaveToChili): Not able to  delete file: " << it->GetPointer()->GetFileName() << std::endl;
              }
              //set the newTextOID as Property
              if( !currentTextOID )
                (*nodeIter)->SetProperty( "TextOID", new StringProperty( textOID ) );
              else
                (*nodeIter)->ReplaceProperty( "TextOID", new StringProperty( textOID ) );
            }
          }
        }
      }
    }
  }
  //save relationship
  if( initParentChild )
  {
    //save the datatreenode-relations
    SaveRelationShip( xmlNodeList, seriesOID );
    //add the saved file to the series

    std::string pathAndFile = m_tempDirectory + "ParentChild.xml";
    if( !pStoreDataFromFile( pathAndFile.c_str(), "ParentChild.xml", "application/MITK.xml", NULL, study.instanceUID, patient.oid, study.oid, m_ParentSeriesOID.c_str(), m_ParentTextOID.c_str() ) )
    {
      std::cout << "ChiliPlugin (SaveToChili): Error while saving parent-child-relationship." << std::endl;
    }
   else
      if( remove(  pathAndFile.c_str() ) != 0 )
        std::cout << "ChiliPlugin (SaveToChili): Not able to  delete file: "<< pathAndFile << std::endl;
  }

  clearStudyStruct( &study );
  clearPatientStruct( &patient );
  clearSeriesStruct( &series );
  QApplication::restoreOverrideCursor();
#endif
}

/** RELATIONSHIP-FUNCTIONS */

/** This function init the xml-file for the overgiven study. */
bool mitk::ChiliPlugin::InitParentChild( const std::string& mitkHideIfNoVersionCode(studyOID) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE

  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (InitParentChild)." );
  return false;

#else

  if( m_tempDirectory.empty() || studyOID == "" )
    return false;

  //init
  m_ParentSeriesOID = "";
  m_ParentTextOID = "";

  //search
  pIterateSeries( this, (char*)studyOID.c_str(), NULL, &ChiliPlugin::GlobalIterateForRelation, this );

  if( ( m_ParentSeriesOID == "" && m_ParentTextOID == "" ) || ( m_ParentSeriesOID == "" ) )
    return false;

  if( m_ParentTextOID == "" )  //no file found
  {
    //create new file on harddisk
    m_currentXmlDoc = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
    m_currentXmlDoc->LinkEndChild( decl );
    TiXmlElement * volumes = new TiXmlElement( "volumes" );
    m_currentXmlDoc->LinkEndChild( volumes );
    TiXmlElement * relations = new TiXmlElement( "relations" );
    m_currentXmlDoc->LinkEndChild( relations );
    //create new text-oid
    m_ParentTextOID = pGetNewOID();
  }
  else  //file exist
  {
    //load file from harddisk
    std::string pathAndFile = m_tempDirectory + "ParentChild.xml";
    m_currentXmlDoc = new TiXmlDocument( pathAndFile.c_str() );
    if( !m_currentXmlDoc->LoadFile() )
    {
      std::cout << "ChiliPlugin (CheckCurrentSeriesForRelation): File for parent-child-relationship exist, but not able to load. Abort." << std::endl;
      return false;
    }
  }
  return true;

#endif
}

#ifdef CHILI_PLUGIN_VERSION_CODE
 /** This function iterate over all series and search the parent-child-xml-file. */
ipBool_t mitk::ChiliPlugin::GlobalIterateForRelation( int /*rows*/, int /*row*/, series_t* series, void* user_data )
{
  ChiliPlugin* callingObject = static_cast<ChiliPlugin*>(user_data);
  if( series != NULL )
  {
    //if no file found, we need the first found series-oid
    if( callingObject->m_ParentSeriesOID == "" )
      callingObject->m_ParentSeriesOID = series->oid;

    //search for file
    pIterateTexts( callingObject, series->oid, NULL, &ChiliPlugin::GlobalIterateForText, callingObject );
    if( callingObject->m_ParentTextOID != "" )
    {
      //file found, save series-oid
      callingObject->m_ParentSeriesOID = series->oid;
      return ipFalse; //stop iterate
    }
  }
  return ipTrue;
}

 /** This function iterate over all text and search the parent-child-xml-file. */
ipBool_t mitk::ChiliPlugin::GlobalIterateForText( int /*rows*/, int /*row*/, text_t *text, void *user_data )
{
  ChiliPlugin* callingObject = static_cast<ChiliPlugin*>(user_data);
  if( text != NULL )
  {
    std::string chiliDataBaseName = text->chiliText;
    std::string fileName = chiliDataBaseName.substr( chiliDataBaseName.find_last_of("-") + 1 );
    if( fileName == "ParentChild.xml" )
    {
      //create Filename and path to save to harddisk
      std::string pathAndFile = callingObject->m_tempDirectory + fileName;
      //Load File from DB
      ipInt4_t error;
      pFetchDataToFile( chiliDataBaseName.c_str(), pathAndFile.c_str(), &error );
      if( error != 0 )
      {
        std::cout << "ChiliPlugin (GlobalIterateForText): ChiliError: " << error << ", while reading file (" << fileName << ") from Database." << std::endl;
      }
      else
        callingObject->m_ParentTextOID = text->oid;
      return ipFalse;  //stop iterate
    }
  }
  return ipTrue;
}
#endif

/** This function add a new volume-entry for the overgiven nodes. */
void mitk::ChiliPlugin::AddNewEntryToXML( DataTreeNode::Pointer mitkHideIfNoVersionCode(node), std::list< std::string > mitkHideIfNoVersionCode(CurrentImageInstanceUIDs), const std::string& mitkHideIfNoVersionCode(seriesOID) )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  //check
  if( !m_currentXmlDoc ) return;
  TiXmlElement* volume = m_currentXmlDoc->FirstChildElement("volumes");
  if( !volume )  return;

  if( node )
  {
    BaseData* data = node->GetData();
    if ( data )
    {
       //get the count of all elements
      int count = 0;
      for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
        count++;

      //create a new volume
      mitk::BaseProperty::Pointer name = node->GetProperty( "name" );
      std::string volumeDescription;
      if( name )
        volumeDescription = name->GetValueAsString();
      else
        volumeDescription = "no Description";
      std::ostringstream stringHelper;
      stringHelper << "volume" << count + 1;
      std::string volumeLabel = stringHelper.str();
      TiXmlElement * element = new TiXmlElement( "volume" );
      element->SetAttribute( "label", volumeLabel.c_str() );
      node->SetProperty( "VolumeLabel", new StringProperty( volumeLabel ) );
      element->SetAttribute( "id", volumeDescription.c_str() );
      element->SetAttribute( "seriesOID", seriesOID.c_str() );

      std::string UIDDescription;
      Image* image = dynamic_cast<Image*>( data );
      if( image )
        UIDDescription = "ImageInstanceUID";
      else
        UIDDescription = "TextOID";

      //add UIDs
      while( !CurrentImageInstanceUIDs.empty() )
      {
        TiXmlElement * uid;
        uid = new TiXmlElement( UIDDescription.c_str() );
        uid->LinkEndChild( new TiXmlText( CurrentImageInstanceUIDs.front().c_str() ) );
        element->LinkEndChild( uid );
        CurrentImageInstanceUIDs.pop_front();
      }

      //save new volume
      volume->LinkEndChild( element );
      std::string pathAndFile = m_tempDirectory + "ParentChild.xml";
      m_currentXmlDoc->SaveFile( pathAndFile.c_str() );
    }
  }
#endif
}

/** This function save the relations between the nodes. */
void mitk::ChiliPlugin::SaveRelationShip( std::list<DataTreeNode::Pointer> inputList, const std::string& seriesOID )
{
  if( m_tempDirectory.empty() ) return;

  InitCircleCheckStructure();

  while( !inputList.empty() )
  {
    //child-node
    DataTreeNode::Pointer currentNode = inputList.front();
    mitk::BaseProperty::Pointer childVolumeLabel = currentNode->GetProperty( "VolumeLabel" );
    //get relation
    DataStorage::SetOfObjects::ConstPointer currentRelations = DataStorage::GetInstance()->GetSources( currentNode );
    for( DataStorage::SetOfObjects::const_iterator relationIter = currentRelations->begin(); relationIter != currentRelations->end(); relationIter++ )
    {
      if( (*relationIter) )
      {
        //check Parent-Node
        mitk::BaseProperty::Pointer parentSeriesOID = (*relationIter)->GetProperty( "SeriesOID" );
        mitk::BaseProperty::Pointer parentVolumeLabel = (*relationIter)->GetProperty( "VolumeLabel" );

        if( !parentSeriesOID ) continue;  //the parent dont save at CHILI
        else
        {
          if( !parentVolumeLabel )
          {
            //if no parentVolumeLabel exist, the node saved to CHILI but not know as Parent-Child-Volumen, so we have to add it
            std::list< std::string > currentUIDs;
            currentUIDs.clear();
            //check if image or text
            mitk::BaseProperty::Pointer parentTextOID = (*relationIter)->GetProperty( "TextOID" );
            if( parentTextOID )
              currentUIDs.push_back( parentTextOID->GetValueAsString() );
            else
            {
              BaseData* data = (*relationIter)->GetData();
              if ( data )
              {
                Image* image = dynamic_cast<Image*>( data );
                if( image )
                {
                  //use the class ImageToPicDescriptor to get the single ImageInstanceUIDS
                  ImageToPicDescriptor::Pointer converterToDescriptor = ImageToPicDescriptor::New();
                  converterToDescriptor->SetImage( image );
                  mitk::ImageToPicDescriptor::TagInformationList empty;
                  empty.clear();
                  converterToDescriptor->SetTagList( empty, true );
                  converterToDescriptor->Update();
                  currentUIDs = converterToDescriptor->GetSaveImageInstanceUIDs();
                }
              }
            }
            //add parent-volume to xml-file
            AddNewEntryToXML( (*relationIter), currentUIDs, seriesOID );
            //get the volume-label
            parentVolumeLabel = (*relationIter)->GetProperty( "VolumeLabel" );
            //add relation
            if( !SaveSingleRelation( childVolumeLabel->GetValueAsString(), parentVolumeLabel->GetValueAsString() ) )
              std::cout << "ChiliPlugin (SaveRelationShip): Relation " << parentVolumeLabel->GetValueAsString() << " - " << childVolumeLabel->GetValueAsString() << " create a circle. Relation not added." << std::endl;
          }
          else
            if( !SaveSingleRelation( childVolumeLabel->GetValueAsString(), parentVolumeLabel->GetValueAsString() ) )
              std::cout << "ChiliPlugin (SaveRelationShip): Relation " << parentVolumeLabel->GetValueAsString() << " - " << childVolumeLabel->GetValueAsString() << " create a circle. Relation not added." << std::endl;
        }
      }
    }
    inputList.pop_front();
  }
}

/** This function initialize the parent-child-struct to test if new relations create circle. */
void mitk::ChiliPlugin::InitCircleCheckStructure()
{
  if( m_tempDirectory.empty() )
    return;

  //check
  if( !m_currentXmlDoc ) return;
  TiXmlElement* relation = m_currentXmlDoc->FirstChildElement("relations");
  TiXmlElement* volume = m_currentXmlDoc->FirstChildElement("volumes");
  if( !volume || !relation ) return;

  //use all volumes to create the list
  m_CircleCheckStructure.clear();
  for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
  {
    CircleCheckStruct newElement;
    newElement.VolumeLabel = "";

    for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
    {
      std::string reference = search->Name();
      if( reference == "label" )
        newElement.VolumeLabel = search->Value();
    }

    newElement.ParentCount = 0;
    newElement.ChildList.clear();
    m_CircleCheckStructure.push_back( newElement );
  }

  //now set the count and parentList
  for( TiXmlElement* singleRelation = relation->FirstChildElement(); singleRelation; singleRelation = singleRelation->NextSiblingElement() )
  {
    std::string volumeLabel = singleRelation->GetText();
    std::string parent = volumeLabel.substr( 0, volumeLabel.find(" ") );
    std::string child = volumeLabel.substr( volumeLabel.find(" ") + 1 );

    for( std::list<CircleCheckStruct>::iterator iter = m_CircleCheckStructure.begin(); iter != m_CircleCheckStructure.end(); iter++ )
    {
      if( iter->VolumeLabel == parent )
        iter->ChildList.push_back( child );
      if( iter->VolumeLabel == child )
        iter->ParentCount++;
    }
  }
}

/** This function save a single relation to the xml-file. Therefore all existing relations get check for circles. */
bool mitk::ChiliPlugin::SaveSingleRelation( const std::string& mitkHideIfNoVersionCode( childVolumeLabel ), const std::string& mitkHideIfNoVersionCode( parentVolumeLabel ) )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  if( !m_currentXmlDoc ) return false;
  TiXmlElement* relation = m_currentXmlDoc->FirstChildElement("relations");
  if( !relation ) return false;

  //delete possible existing relations
  std::string possibleCombinationOne =childVolumeLabel + " " + parentVolumeLabel;
  std::string possibleCombinationTwo =parentVolumeLabel + " " + childVolumeLabel;
  for( TiXmlElement* singleRelation = relation->FirstChildElement(); singleRelation; singleRelation = singleRelation->NextSiblingElement() )
  {
    if( singleRelation->GetText() == possibleCombinationOne || singleRelation->GetText() == possibleCombinationTwo )
      relation->RemoveChild( singleRelation );
  }

  //CIRCLE-CHECK
  std::list<CircleCheckStruct> copyToWork = m_CircleCheckStructure;
  //add relation to workingCopy
  for( std::list<CircleCheckStruct>::iterator iter = copyToWork.begin(); iter != copyToWork.end(); iter++ )
  {
    if( iter->VolumeLabel == parentVolumeLabel )
      iter->ChildList.push_back( childVolumeLabel );
    if( iter->VolumeLabel == childVolumeLabel )
      iter->ParentCount++;
  }

  //test of circle
  bool search = true;
  bool circleFound = false;
  while( search )
  {
    //search for element with no parent
    std::list<CircleCheckStruct>::iterator parentLessElement;
    for( parentLessElement = copyToWork.begin(); parentLessElement != copyToWork.end(); parentLessElement++ )
    {
      if( parentLessElement->ParentCount == 0 ) break;
    }

    if( parentLessElement != copyToWork.end() )
    {
      parentLessElement->ParentCount = -1;
      //decrease parent count of all children
      for( std::list<std::string>::iterator childListIter = parentLessElement->ChildList.begin(); childListIter != parentLessElement->ChildList.end(); childListIter++ )
      {
        for( std::list<CircleCheckStruct>::iterator circleListIter = copyToWork.begin(); circleListIter != copyToWork.end(); circleListIter++ )
        {
          if( circleListIter->VolumeLabel == (*childListIter) )
          {
            circleListIter->ParentCount--;
            break;
          }
        }
      }
    }
    else
    {
      search = false;
      //check if the ParentCount of all elements are -1
      for( std::list<CircleCheckStruct>::iterator circleListIter = copyToWork.begin(); circleListIter != copyToWork.end(); circleListIter++ )
      {
        if( circleListIter->ParentCount != -1 )
        {
          circleFound = true;
          break;
        }
      }
    }
  }
  if( !circleFound )
  {
    m_CircleCheckStructure = copyToWork;
    std::string relationship = parentVolumeLabel + " " + childVolumeLabel;
    TiXmlElement * relationElement = new TiXmlElement( "Relation" );
    relationElement->LinkEndChild( new TiXmlText( relationship.c_str() ) );
    relation->LinkEndChild( relationElement );
    std::string pathAndFile = m_tempDirectory + "ParentChild.xml";
    m_currentXmlDoc->SaveFile( pathAndFile.c_str() );
  }
  return !circleFound;
#endif
  return false;
}

/** This function return the volume-label. Therefore the saved volumes get checked with the overgiven image_instance_UIDs. */
std::string mitk::ChiliPlugin::CheckForVolumeLabel( std::list< std::string > ImageInstanceUIDs )
{
  if( !m_currentXmlDoc ) return "";
  TiXmlElement* volume = m_currentXmlDoc->FirstChildElement("volumes");
  if( !volume )  return "";

  for( TiXmlElement* singleVolume = volume->FirstChildElement(); singleVolume; singleVolume = singleVolume->NextSiblingElement() )
  {
    unsigned int idCount = 0;  //we have to count the ids, they can be different sized
    bool match = true;
    TiXmlElement* singleID = singleVolume->FirstChildElement();
    while( singleID && match )
    {
      if( find( ImageInstanceUIDs.begin(), ImageInstanceUIDs.end(), singleID->GetText() ) != ImageInstanceUIDs.end() )
        idCount++;
      else
        match = false; //one slice dont match, thats not the searched volume, take the next
      singleID = singleID->NextSiblingElement();
    }
    if( ( ImageInstanceUIDs.size() == idCount ) && match )
    {
      for( TiXmlAttribute* search = singleVolume->FirstAttribute(); search; search = search->Next() )
      {
        std::string reference = search->Name();
        if( reference == "label" )
          return search->Value();
      }
      return "";
    }
  }
  return "";
}

/** This function found the highes imagenumber in a series, its needed because the slice-sort-reader use the imagenumber as criteria. */
int mitk::ChiliPlugin::GetMaximumImageNumber( const std::string& seriesOID )
{
  m_MaximumImageNumber = 0;
  iterateImages( (char*)seriesOID.c_str(), NULL, &ChiliPlugin::GlobalIterateImagesForMaximalImageNumber, this );
  return m_MaximumImageNumber;
}

/** Function to iterate over all images from a series. */
ipBool_t mitk::ChiliPlugin::GlobalIterateImagesForMaximalImageNumber( int /*rows*/, int /*row*/, image_t* image, void* user_data )
{
  ChiliPlugin* callingObject = static_cast<ChiliPlugin*>( user_data );
  if( image->number > callingObject->m_MaximumImageNumber )
    callingObject->m_MaximumImageNumber = image->number;
  return ipTrue; // enum from ipTypes.h
}

/** Create the needed tags to save a pic-file to chili. */
mitk::ImageToPicDescriptor::TagInformationList mitk::ChiliPlugin::GetNeededTagList( study_t* mitkHideIfNoVersionCode(study), patient_t* mitkHideIfNoVersionCode(patient), series_t* mitkHideIfNoVersionCode(series) )
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

/** Event if a new study get selected (from QcPlugin). */
void mitk::ChiliPlugin::studySelected( study_t* /*study*/ )
{
  SendStudySelectedEvent();
}

/** Event if the lightbox get tiles (from QcPlugin). */
void mitk::ChiliPlugin::lightboxTiles( QcLightboxManager* /*lbm*/, int tiles )
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

/** Button to import the lightbox. */
void mitk::ChiliPlugin::lightBoxImportButtonClicked(int row)
{
  if( m_InImporting ) return;

  if( ChiliFillingLightbox() )
  {
    QMessageBox::information( 0, "MITK", "Lightbox not ready (still loading slices). Try again when lightbox filling is completed!" );
    return;
  }

  QPtrList<QcLightbox>& lightboxes = QcPlugin::lightboxManager()->getLightboxes();
  QcLightbox* selectedLightbox = lightboxes.at(row);

  if( selectedLightbox )
  {
    m_InImporting = true;
    std::vector<DataTreeNode::Pointer> resultNodes;
    //dont use "LoadCompleteSeries( ... )", the Buttons called LightboxImportButtons, so use the LoadImagesFromLightbox-Function too
    resultNodes = LoadImagesFromLightbox( selectedLightbox );  //load all images

    if( resultNodes.size() > 8 )
    {
      if( QMessageBox::question( 0, tr("MITK"), QString("MITK detected %1 distinct image volumes.\nDo you want to load all of them (might slow down MITK)?").arg(resultNodes.size()), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
      {
        for( unsigned int n = 0; n < resultNodes.size(); n++ )
          DataStorage::GetInstance()->Add( resultNodes[n] );
      }
      else
      {
        for( unsigned int n = 0; n < resultNodes.size(); n++ )
        {
          mitk::BaseProperty::Pointer propertyImageNumber = resultNodes[n]->GetProperty( "NumberOfSlices" );
          if( propertyImageNumber.IsNull() || ( propertyImageNumber.IsNotNull() && propertyImageNumber->GetValueAsString() != "1" ) )
            DataStorage::GetInstance()->Add( resultNodes[n] );
        }
      }
    }
    else
      for( unsigned int n = 0; n < resultNodes.size(); n++ )
        DataStorage::GetInstance()->Add( resultNodes[n] );

#ifdef CHILI_PLUGIN_VERSION_CODE
    if( selectedLightbox->currentSeries() )
    {
      resultNodes = LoadAllTextsFromSeries( selectedLightbox->currentSeries()->oid );  //load all texts
      for( unsigned int n = 0; n < resultNodes.size(); n++ )
        DataStorage::GetInstance()->Add( resultNodes[n] );
    }
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

/** Throw an event, if the study changed. */
void mitk::ChiliPlugin::SendStudySelectedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginStudySelected() );
}

/** Throw an event, if the lightboxcount changed. */
void mitk::ChiliPlugin::SendLightBoxCountChangedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginLightBoxCountChanged() );
}

/** Reinitialize the application. */
void mitk::ChiliPlugin::OnApproachReinitializationOfWholeApplication()
{
  static int clickCount = 0;

  clickCount++;

  if (clickCount > 0)
  {
    if ( QMessageBox::question( NULL, tr("MITK"), 
                                  /*QString("Desparate clicking suggests a strong wish to restart the whole MITK plugin.\n\nDo you want to restart MITK?"),*/
                                  QString("Do you want to restart MITK and loose all work results?"),
                                  QMessageBox::Yes | QMessageBox::Default, 
                                  QMessageBox::No  | QMessageBox::Escape
                                ) == QMessageBox::Yes )
    {
      CreateSampleApp();
    }

    clickCount = 0;
  }
}

/** TELECONFERENCE METHODS */

void mitk::ChiliPlugin::connectPartner()
{
  ConferenceToken* ct = ConferenceToken::GetInstance();
  ct->ArrangeToken();
}

void mitk::ChiliPlugin::disconnectPartner()
{
  ConferenceToken* ct = ConferenceToken::GetInstance();
  ct->SetToken( 0 );
}

void mitk::ChiliPlugin::handleMessage( ipInt4_t type, ipMsgParaList_t *list )
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
        //std::cout<<"ChiliPlugin::handleMessage() -> MouseMoveEvent couldn't pass through"<<std::endl;

    break;   
    default:
//       message = QString( " type %1" ).arg( type );
//       arguments = "argumente unbekannt";
      break;
  }
}

#ifdef WIN32

#include <Windows.h>
#include <stdio.h>
#define BUFSIZE 512

/** Create a temporary directory for windows. */
std::string mitk::ChiliPlugin::GetTempDirectory()
{
  DWORD BufSize=BUFSIZE;
  char PathBuffer[BUFSIZE];
  char TempName[BUFSIZE];  
  DWORD RetVal;
  UINT uRetVal;

  //get the temp path
  RetVal = GetTempPath( BufSize, PathBuffer );
  if( RetVal > BufSize || RetVal == 0 )
  {
    return "";
  }

  //create a unique name
  uRetVal = GetTempFileName( PathBuffer, "CHI", 0, TempName );
  if( uRetVal == 0 )
  {
    return "";
  }

  //the function create a random, not existing file; we dont want them, we want a directory
  std::string tmpName = TempName;
  tmpName = "del /Q " + tmpName;
  system( tmpName.c_str() );

  //use the random created name
  std::string fileName = TempName;
  fileName = fileName.substr( 0, fileName.find_first_of(".") );
  fileName = fileName + "\\";
  std::string makeTmpDirectory = "mkdir " + fileName;

  //create directory
  system( makeTmpDirectory.c_str() );

  return fileName;
}

#else

#include <stdlib.h>

/** Create a temporary directory for linux. */
std::string mitk::ChiliPlugin::GetTempDirectory()
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

QObject* mitk::ChiliPlugin::findProgressBar(QObject* object)
{
  const QObjectList* children = object->children();
  if (children)
  {
    QObjectListIt it( *children );
    QObject* child;
    while ( (child = it.current()) != 0 )
    {
      //std::cout << "Testing child '" << child->name() << "' (" << child->className() << ")" << std::endl;
      if ( std::string(child->className()) == "QProgressBar" )
        return child;

      QObject* result = findProgressBar( child );
      if (result) 
        return result;

      ++it;
    }
  }

  return NULL;
}

bool mitk::ChiliPlugin::ChiliFillingLightbox()
{
  // find lightBoxArea
  QObject* current(this);
  QObject* lightboxAreaObject(NULL);
  while (current)
  {
    /*
    std::cout << "============================================" << std::endl;
    std::cout << "Current object: '" << current->name() << "' (" << current->className() << ")" << std::endl;
    */

    const QObjectList* children = current->children();
    if (children)
    {
      QObjectListIt it( *children );
      QObject* child;
      while ( (child = it.current()) != 0 )
      {
        //std::cout << "  Child '" << child->name() << "' (" << child->className() << ")" << std::endl;

        if ( std::string(child->name()) == "lightboxArea" ) 
        {
          lightboxAreaObject = child;
          break;
        }
        ++it;
      }
    }

    if (lightboxAreaObject) break;

    current = current->parent();
  }

  if (lightboxAreaObject)
  {
    QProgressBar* progressBar = dynamic_cast<QProgressBar*>( findProgressBar( lightboxAreaObject ) );
    if (progressBar)
    {
      //std::cout << "Found progressbar, progress " << progressBar->progress() << std::endl;
      //return progressBar->progress() != 0;
      return progressBar->isVisible();
    }
    else
    {
      //std::cout << "Couldn't find progressbar -- assuming CHILI is not filling lightbox." << std::endl;
      return false;
    }
  }

  return false;
}

