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
#include <QmitkCommonFunctionality.h>
#include <SampleApp.h>
#include <QmitkEventCollector.h>
#include <QcMITKTask.h>
//Chili
#include <chili/plugin.xpm>
#include <chili/qclightboxmanager.h> //needed for iterateseries, ... //
//MITKPlugin
#include "mitkChiliPluginImpl.h"
#include <mitkChiliPluginFactory.h>
#include <mitkLightBoxImageReader.h>
#include <mitkDataTreeNodeFactory.h>
#include <mitkDataStorage.h>
//Reader- and Writer-Factory´s
#include "mitkSurfaceVtkWriterFactory.h"
#include "mitkImageWriterFactory.h"
#include "mitkPointSetWriterFactory.h"
//teleconference
#include <mitkConferenceToken.h>
#include <mitkConferenceEventMapper.h>
#include <QmitkChili3ConferenceKitFactory.h>
//GUI
#include <qlayout.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qtooltip.h>

#include <mitkPropertyList.h>
#include <mitkDataStorage.h>
#include <mitkCoreObjectFactory.h>
#include "chili_lightbox_import.xpm"

#define NUMBER_OF_THINKABLE_LIGHTBOXES 4

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

  mitk::SurfaceVtkWriterFactory::RegisterOneFactory();
  mitk::PointSetWriterFactory::RegisterOneFactory();
  mitk::ImageWriterFactory::RegisterOneFactory();

  // give Chili the single instance of mitk::ChiliPluginImpl
  mitk::ChiliPlugin::Pointer pluginInstance = mitk::ChiliPlugin::GetInstance();
  mitk::ChiliPluginImpl::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPluginImpl*>( pluginInstance.GetPointer() );

  return realPluginInstance.GetPointer();
}

/** constructor with hidden parameter s_Parent */
mitk::ChiliPluginImpl::ChiliPluginImpl()
: QcPlugin( s_Parent ),
  app( NULL )
{
  task = new QcMITKTask( xpm(), s_Parent, name() );

  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  m_CurrentSeries.clear();
}

mitk::ChiliPluginImpl::~ChiliPluginImpl()
{
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

bool mitk::ChiliPluginImpl::IsPlugin()
{
  return true;
}

#if ( CHILI_VERSION >= 38 )

mitk::ChiliPlugin::StudyInformation mitk::ChiliPluginImpl::GetCurrentSelectedStudy()
{
  study_t* tempStudy = pCurrentStudy();
  mitk::ChiliPlugin::StudyInformation resultStudy;
  if( tempStudy != NULL )
  {
    resultStudy.OID = tempStudy->oid;
    resultStudy.InstanceUID = tempStudy->instanceUID;
    resultStudy.ID = tempStudy->id;
    resultStudy.Date = tempStudy->date;
    resultStudy.Time = tempStudy->time;
    resultStudy.Modality = tempStudy->modality;
    resultStudy.Manufacturer = tempStudy->manufacturer;
    resultStudy.ReferingPhysician = tempStudy->referingPhysician;
    resultStudy.Description = tempStudy->description;
    resultStudy.ManufacturersModelName = tempStudy->manufacturersModelName;
    resultStudy.ImportTime = tempStudy->importTime;
    resultStudy.ChiliSenderID = tempStudy->chiliSenderID;
    resultStudy.AccessionNumber = tempStudy->accessionNumber;
    resultStudy.InstitutionName = tempStudy->institutionName;
    resultStudy.WorkflowState = tempStudy->workflow_state;
    resultStudy.Flags = tempStudy->flags;
    resultStudy.PerformingPhysician = tempStudy->performingPhysician;
    resultStudy.ReportingPhysician = tempStudy->reportingPhysician;
    resultStudy.LastAccess = tempStudy->last_access;
  }
  return resultStudy;
}

mitk::ChiliPlugin::SeriesList mitk::ChiliPluginImpl::GetCurrentSelectedSeries()
{
  if( m_currentStudyChanged )
  {
    m_CurrentSeries.clear();
    m_currentStudyChanged = false;
    char* currentStudyOID_c = (char*)GetCurrentSelectedStudy().OID.c_str();
    iterateSeries( currentStudyOID_c, NULL, &mitk::ChiliPluginImpl::GlobalIterateSeriesCallback, this );
  }
  return m_CurrentSeries;
}

mitk::ChiliPlugin::PatientInformation mitk::ChiliPluginImpl::GetCurrentSelectedPatient()
{
  patient_t* tempPatient = pCurrentPatient();
  mitk::ChiliPlugin::PatientInformation resultPatient;
  if( tempPatient != NULL )
  {
    resultPatient.OID = tempPatient->oid;
    resultPatient.Name = tempPatient->name;
    resultPatient.ID = tempPatient->id;
    resultPatient.BirthDate = tempPatient->birthDate;
    resultPatient.BirthTime = tempPatient->birthTime;
    resultPatient.Sex = tempPatient->sex;
    resultPatient.MedicalRecordLocator = tempPatient->medicalRecordLocator;
    resultPatient.Comment = tempPatient->comment;
  }
  return resultPatient;
}

mitk::ChiliPlugin::TextFileList mitk::ChiliPluginImpl::GetTextFileInformation( std::string seriesOID )
{
  m_TextFileList.clear();

  std::string tempSeriesOID;
  if( seriesOID == "" )
  {
    series_t* currentSeries = pCurrentSeries();
    if( !currentSeries )
    {
      std::cout << "ERROR: no Series selected!" << std::endl;
      return m_TextFileList;
    }
    tempSeriesOID = currentSeries->oid;
  }
  else tempSeriesOID = seriesOID;

  mitk::ChiliPluginImpl* realPluginInstance = dynamic_cast<mitk::ChiliPluginImpl*>( mitk::ChiliPlugin::GetInstance() );
  pIterateTexts( realPluginInstance, (char*)tempSeriesOID.c_str(), NULL, &mitk::ChiliPluginImpl::GlobalIterateTextCallback, this );

  return m_TextFileList;
}

ipBool_t mitk::ChiliPluginImpl::GlobalIterateTextCallback( int rows, int row, text_t *text, void *user_data )
{
  mitk::ChiliPlugin::TextFileInformation resultText;
  resultText.OID = text->oid;
  resultText.MimeType = text->mime_type;
  resultText.ChiliText = text->chiliText;
  resultText.Status = text->status;
  resultText.FrameOfReferenceUID = text->frameOfReferenceUID;
  resultText.TextDate = text->text_date;
  resultText.Description = text->description;

  mitk::ChiliPluginImpl* callingObject = static_cast<mitk::ChiliPluginImpl*>(user_data);
  callingObject->m_TextFileList.push_back( resultText );

  return ipTrue; // enum from ipTypes.h
}

#else

mitk::ChiliPlugin::StudyInformation mitk::ChiliPluginImpl::GetCurrentSelectedStudy()
{
  return m_CurrentStudy;
}

mitk::ChiliPlugin::SeriesList mitk::ChiliPluginImpl::GetCurrentSelectedSeries()
{
  return m_CurrentSeries;
}

#endif

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

  callingObject->m_CurrentSeries.push_back( newSeries );

  return ipTrue; // enum from ipTypes.h
}

unsigned int mitk::ChiliPluginImpl::GetLightBoxCount()
{
  return m_LightBoxCount;
}

int mitk::ChiliPluginImpl::GetConferenceID()
{
  return m_QmitkChiliPluginConferenceID;
}

void mitk::ChiliPluginImpl::SendStudySelectedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginStudySelected() );
}

void mitk::ChiliPluginImpl::SendLightBoxCountChangedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginLightBoxCountChanged() );
}

void mitk::ChiliPluginImpl::AddPropertyListToNode( const mitk::PropertyList::Pointer property, mitk::DataTreeNode* dst )
{
  for( mitk::PropertyList::PropertyMap::const_iterator iter = property->GetMap()->begin(); iter != property->GetMap()->end(); iter++ )
  {
    dst->SetProperty( iter->first.c_str(), iter->second.first );
  }
}

void mitk::ChiliPluginImpl::SetQtParent( QWidget* parent )
{
  s_Parent = parent;
}

QString mitk::ChiliPluginImpl::name()
{
  return QString( "MITK" );
}

const char** mitk::ChiliPluginImpl::xpm()
{
  return (const char **)plugin_xpm;
}

void mitk::ChiliPluginImpl::lightboxFilled (QcLightbox* lightbox)
{
}

void mitk::ChiliPluginImpl::lightboxTiles (QcLightboxManager *lbm, int tiles)
{
  m_LightBoxCount = tiles;

  for (int row = 0; row < tiles; ++row)
  {
    try
    {
      QIDToolButton* button = m_LightBoxImportButtons.at(row);
      button->show();
    }
    catch(...) {}
  }
  for (int row = tiles; row < NUMBER_OF_THINKABLE_LIGHTBOXES; ++row)
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

// teleconference methods

void mitk::ChiliPluginImpl::connectPartner()
{
  mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
  ct->ArrangeToken();
}

void mitk::ChiliPluginImpl::disconnectPartner()
{
  mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
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
    case mitk::m_QmitkChiliPluginConferenceID + mitk::MITKc:
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

      if(!mitk::ConferenceEventMapper::MapEvent(eventID, str,etype,estate,ebuttonstate,ekey,w1,w2,w3,p1,p2))
        //std::cout<<"EventMaper no funciona"<<std::endl;

    break;
    case mitk::m_QmitkChiliPluginConferenceID +mitk::QTc :
      ipMsgListToVar( list,
                      ipTypeStringPtr, &str );
      //std::cout<<"CONFERENCE: (1) "<<str;
      if( ! EventCollector::PostEvent(QString(str), task))
      //std::cout<<" NO SUCCES!"<<std::endl;
      //std::cout<<std::endl;

    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::ARRANGEc:
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &sender );
        //std::cout<<"CONFERENCE TOKEN ID:  "<<sender<<std::endl;

        mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
        ct->ArrangeToken(sender);
        }
    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENREQUESTc:
        //FROM: AskForToken(...)
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &receiver );
        //std::cout<<"CONFERENCE TOKEN REQUEST FROM:  "<<receiver<<std::endl;

        mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
        ct->GetToken(receiver);
        }
    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENSETc:
        //FROM: AskForToken(...)
        {
        ipMsgListToVar( list,
                        ipTypeInt4, &sender,
                        ipTypeInt4, &receiver );
        //std::cout<<"CONFERENCE TRY SET TOKEN FROM:  "<<sender<<" TO:"<<receiver<<std::endl;

        mitk::ConferenceToken* ct = mitk::ConferenceToken::GetInstance();
        ct->SetToken( receiver );
        }
    break;
    case mitk::m_QmitkChiliPluginConferenceID + mitk::MOUSEMOVEc:
      ipMsgListToVar( list,
                      ipTypeStringPtr, &str,
                      ipTypeFloat4, &w1,
                      ipTypeFloat4, &w2,
                      ipTypeFloat4, &w3
          );

      if( !mitk::ConferenceEventMapper::MapEvent( str, w1, w2, w3 ) )
        //std::cout<<"mitk::ChiliPluginImpl::handleMessage() -> MouseMoveEvent couldn't pass through"<<std::endl;

    break;   
    default:
//       message = QString( " type %1" ).arg( type );
//       arguments = "argumente unbekannt";
      break;
  }
}

void mitk::ChiliPluginImpl::studySelected( study_t* study )
{
  // create a gui the first time a study is selected
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
    m_LightBoxImportToggleButton->setEnabled(false);
    QSizePolicy policy = m_LightBoxImportToggleButton->sizePolicy();
    policy.setVerStretch( 1 );
    m_LightBoxImportToggleButton->setSizePolicy( m_LightBoxImportToggleButton->sizePolicy());
    vertlayout->addWidget( m_LightBoxImportToggleButton );

    // create one instance of SampleApp
    app = new SampleApp( task, "sample", 0 );
    horzlayout->addWidget( app ); // fromRow, toRow, fromCol, toCol

    done = true;
  }

#if ( CHILI_VERSION >= 38 )
  m_currentStudyChanged = true;
  SendStudySelectedEvent();
#else
  if (study)
  {
    //set StudyInformation
    mitk::ChiliPlugin::StudyInformation newStudy;

    newStudy.OID = study->oid;
    newStudy.InstanceUID = study->instanceUID;
    newStudy.ID = study->id;
    newStudy.Date = study->date;
    newStudy.Time = study->time;
    newStudy.Modality = study->modality;
    newStudy.Manufacturer = study->manufacturer;
    newStudy.ReferingPhysician = study->referingPhysician;
    newStudy.Description = study->description;
    newStudy.ManufacturersModelName = study->manufacturersModelName;
    newStudy.ImportTime = study->importTime;
    newStudy.ChiliSenderID = study->chiliSenderID;
    newStudy.AccessionNumber = study->accessionNumber;
    newStudy.InstitutionName = study->institutionName;
    newStudy.WorkflowState = study->workflow_state;
    newStudy.Flags = study->flags;
    newStudy.PerformingPhysician = study->performingPhysician;
    newStudy.ReportingPhysician = study->reportingPhysician;
    newStudy.LastAccess = study->last_access;

    m_CurrentStudy = newStudy; //this is friend of ChiliPluginImpl
    m_CurrentSeries.clear();

    //get OID for iterate
    std::string currentStudyOID = study->oid;
    //iterate
    iterateSeries( (char*)currentStudyOID.c_str(), NULL, &ChiliPluginImpl::GlobalIterateSeriesCallback, this );
    //send event for all application listeners
    SendStudySelectedEvent();
  }
  else
  {
    std::cout<< "No Study selected." <<std::endl;
    m_CurrentSeries.clear();
    mitk::ChiliPlugin::StudyInformation emptyList;
    m_CurrentStudy = emptyList;
  }
#endif
}

void mitk::ChiliPluginImpl::lightBoxImportButtonClicked(int row)
{
  if ( m_InImporting ) return;
  QPtrList<QcLightbox>& lightboxes = QcPlugin::lightboxManager()->getLightboxes();
  QcLightbox* selectedLightbox = lightboxes.at(row);
  if (selectedLightbox)
  {
    m_InImporting = true;
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    mitk::LightBoxImageReader::Pointer reader = mitk::LightBoxImageReader::New();
    reader->SetLightBox( selectedLightbox );
    mitk::Image::Pointer image = reader->GetOutput();
    image->Update();
    qApp->processEvents();
    m_InImporting = false;

    if( image->IsInitialized() )
    {
      // add to scene
      mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();                                         // create a new empty node
      node->SetData( image );
      mitk::DataTreeNodeFactory::SetDefaultImageProperties( node );
      mitk::ChiliPlugin::GetInstance()->AddPropertyListToNode( reader->GetImageTagsAsPropertyList(), node.GetPointer() );
      mitk::DataStorage::GetInstance()->Add( node );

      // stupid, that this is still necessary
      mitk::DataTreePreOrderIterator treeiter( app->GetTree() );
      app->GetMultiWidget()->InitializeStandardViews( &treeiter );
      app->GetMultiWidget()->Fit();
      app->GetMultiWidget()->ReInitializeStandardViews();

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else
    if( !reader->userAbort() )
    {
      QMessageBox::information(NULL, "MITK", "Couldn't read this image. \n\nIf you feel this should be possible, report a bug to mitk-users@lists.sourceforge.net", QMessageBox::Ok);
    }
    QApplication::restoreOverrideCursor();
  }
  else
  {
    std::cerr << "Lightbox import selected for lightbox #" << row << ", but lightbox manager doesn't know this lightbox." << std::endl;
  }
}

#if ( CHILI_VERSION >= 38 )
void mitk::ChiliPluginImpl::UploadViaFile( DataTreeNode* node, std::string studyInstanceUID, std::string patientOID, std::string studyOID, std::string seriesOID)
{
  if( node )
  {
    //Searching for possible Writer
    std::list<FileWriter::Pointer> possibleWriter;
    std::list<LightObject::Pointer> allObjects = itk::ObjectFactoryBase::CreateAllInstance( "IOWriter" );
    for( std::list<LightObject::Pointer>::iterator iter = allObjects.begin(); iter != allObjects.end(); iter++ )
    {
      FileWriter* writer = dynamic_cast<FileWriter*>( iter->GetPointer() );
      if( writer )
        possibleWriter.push_back( writer );
      else std::cout << "ERROR: no FileWriter returned" << std::endl;
    }

    //checking the parameter and setting the input for "pStoreDataFromFile"
    std::string tempStudyInstanceUID, tempPatientOID, tempStudyOID, tempSeriesOID;

    //there is no input beside the datatreenode -> save to current selected study and series
    if( studyInstanceUID == "" && patientOID == "" && studyOID == "" && seriesOID == "" )
    {
      patient_t* currentPatient = pCurrentPatient();
      study_t* currentStudy = pCurrentStudy();
      series_t* currentSeries = pCurrentSeries();
      if( !currentStudy || !currentPatient || !currentSeries )
      {
        QMessageBox::information(NULL, "MITK", "ERROR: no Series selected!", QMessageBox::Ok);
        return;
      }
      tempStudyInstanceUID = currentStudy->instanceUID;
      tempPatientOID = currentPatient->oid;
      tempStudyOID = currentStudy->oid;
      tempSeriesOID = currentSeries->oid;
    }
    //save to specific study and series
    else
      if( studyInstanceUID == "" || patientOID == "" || studyOID == "" || seriesOID == "" )
        std::cout << "ERROR: one parameter is NULL." <<std::endl;
      else
      {
        tempStudyInstanceUID = studyInstanceUID;
        tempPatientOID = patientOID;
        tempStudyOID = studyOID;
        tempSeriesOID = seriesOID;
      }

    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

    for( std::list<FileWriter::Pointer>::iterator it = possibleWriter.begin(); it != possibleWriter.end(); it++ )
    {
      if( it->GetPointer()->CanWrite( node ) )
      {
        std::string fileName;
        if( node->GetProperty( "name" )->GetValueAsString() != "" )
          fileName = node->GetProperty( "name" )->GetValueAsString();
        else fileName = "FileUpload";

        fileName = fileName + it->GetPointer()->GetFileExtension();
        std::string pathAndFile = "/tmp/" + fileName;
        it->GetPointer()->SetFileName( pathAndFile.c_str() );
        it->GetPointer()->SetInput( node );
        it->GetPointer()->Write();
        //its very important, that the filename get used two times.
        //the first parameter is the target to load from, this dont get saved.
        //so if you want to know the filename to download from db, you have to set it as "name" too.
        if( !pStoreDataFromFile( pathAndFile.c_str(), fileName.c_str(), it->GetPointer()->GetWritenMIMEType().c_str(), tempStudyInstanceUID.c_str(), tempPatientOID.c_str(), tempStudyOID.c_str(), tempSeriesOID.c_str() ) )
        std::cout << "ERROR: while saving File (" << fileName << ") to Database." << std::endl;

        if( remove(  pathAndFile.c_str() ) != 0 )
          std::cout << "ERROR: while deleting file: " << it->GetPointer()->GetFileName() << std::endl;
      }
    }
    QApplication::restoreOverrideCursor();
  }
}

void mitk::ChiliPluginImpl::DownloadViaFile( std::string chiliText, std::string MimeType, mitk::DataTreeIteratorBase* parentIterator )
{
  if( chiliText != "" && MimeType != "" )
  {
    //get the FileExtension from the file
    char* fileExtension;
    fileExtension = strrchr( chiliText.c_str(), '.' );
    //check the FileExtension if MITK can read
    char* check;
    check = strstr( mitk::CoreObjectFactory::GetInstance()->GetFileExtensions(), fileExtension );
    if( check == NULL )
    {
      std::cout << "ERROR: no readable Fileextension found. " << std::endl;
      return;
    }
    //get Filename and path to save
    char* fileName;
    fileName = strrchr( chiliText.c_str(), '-' );
    //we want no "-" in front of the filename
    fileName++;

    std::string pathAndFile = "/tmp/";
    strcat ( (char*)pathAndFile.c_str(), fileName );

    ipInt4_t error;
    pFetchDataToFile( chiliText.c_str(), pathAndFile.c_str(), &error );
    if( error != 0 )
    {
      std::cout << "ERROR: " << error << ", while reading file (" << fileName << ") from Database." << std::endl;
      return;
    }

    mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();
    try
    {
      factory->SetFileName( pathAndFile );
      factory->Update();
      for ( unsigned int i = 0 ; i < factory->GetNumberOfOutputs( ); ++i )
      {
        mitk::DataTreeNode::Pointer node = factory->GetOutput( i );
        if ( ( node.IsNotNull() ) && ( node->GetData() != NULL )  )
        {
          parentIterator->Add( node );
        }
      }
    }
    catch ( itk::ExceptionObject & ex )
    {
      itkGenericOutputMacro( << "Exception during file open: " << ex );
    }
    if( remove(  pathAndFile.c_str() ) != 0 )
      std::cout << "ERROR: while deleting file: " << pathAndFile << std::endl;
  }
  else std::cout<<"ERROR: no MimeType or ChiliText set."<<std::endl;
}

void mitk::ChiliPluginImpl::UploadViaBuffer( DataTreeNode* node )
{
//UNDER CONSTRUCTION
}

mitk::DataTreeNode* mitk::ChiliPluginImpl::DownloadViaBuffer()
{
//UNDER CONSTRUCTION
  return NULL;
}

#endif

