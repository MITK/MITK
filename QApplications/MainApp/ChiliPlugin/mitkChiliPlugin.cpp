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

#define NUMBER_OF_THINKABLE_LIGHTBOXES 4

#include "mitkChiliPlugin.h"

//CHILI
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
#include <chili/qclightboxmanager.h>  //get newLightbox, currentLightbox

//MITK-Plugin
#include "mitkPACSPluginEvents.h"
#include "mitkChiliPluginFactory.h"

//teleconference
#include "mitkConferenceToken.h"
#include "mitkConferenceEventMapper.h"
#include "QmitkChili3ConferenceKitFactory.h"

// Qmitk
#include "QmitkStdMultiWidget.h"
#include "QmitkEventCollector.h"
#include "QcMITKTask.h"

// MITK
#include "SampleApp.h"
#include "mitkProgressBar.h"
#include "mitkRenderingManager.h"
#include "mitkIpPicUnmangle.h"

// Qt
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
#include <qsplitter.h>
#include <qpopupmenu.h>

//XPM images
#include "mitk_chili_plugin.xpm"
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
  m_ChiliInformation( CHILIInformation::New() ),
  m_LoadFromCHILI( LoadFromCHILI::New() ),
  m_SaveToCHILI( SaveToCHILI::New() ),
  m_MITKMainApp( NULL ),
  m_LightBoxCount( 0 ),
  m_InImporting (false)
{
  // contains the visible worktask button
  m_Task = new QcMITKTask( xpm(), s_Parent, name() );

  // somehow needed for the conference part
  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  m_tempDirectory = GetTempDirectory();

  if( m_tempDirectory.empty() )
  {
    QMessageBox::information( 0, 
                              "MITK", 
                              "MITK was not able to create a temporary directory.\n"
                              "You can only load data from CHILI using the yellow light box import buttons." );
  }
  else
  {
    std::cout << "CHILIPlugin: Create and use directory "<< m_tempDirectory << std::endl;
  }
}


/** Destructor */
mitk::ChiliPlugin::~ChiliPlugin()
{
  if( !m_tempDirectory.empty() )
  {
    std::string removeDirectory = "";

    #ifdef WIN32
      removeDirectory = "rmdir /S /Q " + m_tempDirectory;
      std::cout << "CHILIPlugin: Delete directory "<< m_tempDirectory << std::endl;
    #else
      removeDirectory = "rm -r " + m_tempDirectory;
      std::cout << "CHILIPlugin: Delete directory "<< m_tempDirectory << std::endl;
    #endif

    if ( system( removeDirectory.c_str() ) == -1 )
      std::cerr << "Couldn't delete " << m_tempDirectory << std::endl;
  }

  //m_Task->deleteLater();

  // tricky tricky...
  m_ReferenceCountLock.Lock();
  ++m_ReferenceCount; // increase count, so the following notice to mitk::ChiliPlugin won't trigger another delete on "this"
  m_ReferenceCountLock.Unlock();

  // remove ITK smart pointer references to "this" object (without actually deleting ourselves)
  PACSPlugin::GetInstance(true); // set internal s_Instance = NULL, so ITK won't have another SmartPointer to us (hopefully nobody else has)

/*  desparate tries to fix DataStorage related crash on exit
  // play deaf (concentrate on killing this, don't react to messages)
  for ( std::vector<QObject*>::iterator iter = m_ObjectsConnectedTo.begin();
        iter != m_ObjectsConnectedTo.end();
        ++iter )
  {
    disconnect( *iter, 0, this, 0 );
  }

  QcLightboxManager* manager = lightboxManager();
  QPtrList<QcLightbox>& lightboxes = manager->getLightboxes();
  QcLightbox* current;
  unsigned int activeLightboxIndex = 0;
  for ( current = lightboxes.first(); current; current = lightboxes.next(), ++activeLightboxIndex )
  {
    disconnect( current, 0, this, 0 );
  }
 
  // try to delete MainApp (and erase it from CHILI's knowledge)
  qApp->removePostedEvents(m_MITKMainApp);
  m_MITKMainApp->parent()->removeChild( m_MITKMainApp );
  m_MITKMainApp->reparent(NULL, 0, QPoint(0,0));
  delete m_MITKMainApp;
  
  // delete DataStorage singleton
  mitk::DataStorage::GetInstance()->ShutdownSingleton();
*/

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
    m_ImportButtonLayout = new QHBoxLayout( m_Task ); // member
    QVBoxLayout* vertlayout = new QVBoxLayout( m_ImportButtonLayout );

    // 4 lightbox import buttons
    for (unsigned int row = 0; row < NUMBER_OF_THINKABLE_LIGHTBOXES; ++row)
    {
      QIDToolButton* toolbutton = new QIDToolButton( row, m_Task );
      toolbutton->setUsesTextLabel( true );
      QToolTip::add( toolbutton, "Import this lightbox to MITK");
      toolbutton->setTextPosition( QToolButton::BelowIcon );
      toolbutton->setPixmap( QPixmap(chili_lightbox_import_xpm) );

      QSizePolicy policy = toolbutton->sizePolicy();
      policy.setVerStretch( 2 );
      toolbutton->setSizePolicy( policy );

      connect( toolbutton, SIGNAL(clicked(int)), this, SLOT(lightBoxImportButtonClicked(int)));
      m_ObjectsConnectedTo.push_back( toolbutton );

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

    // menu for plugin options
    m_PopupMenu = new QPopupMenu(m_Task);
    m_PopupMenu->setCheckable(true);
    m_PopupMenu->insertItem("Import using 'Image number' filter (default)", 0);
    m_PopupMenu->setItemChecked(0, true);
    m_PopupMenu->insertItem("Import using 'Single spacing' filter", 1);
    m_PopupMenu->insertItem("Import using 'Spacing set' filter", 2);
    m_PopupMenu->insertSeparator();
    m_PopupMenu->insertItem("Separate by acquisition number", this, SLOT(OnMenuSeparateByAcquisitionNumberClicked()), 0, 4);
    m_PopupMenu->setItemChecked(4, false);
    connect( m_PopupMenu, SIGNAL(activated(int)), this, SLOT(OnMenuImportFilterSelected(int)) );
    m_ObjectsConnectedTo.push_back( m_PopupMenu );

    // button for plugin options
    m_LightBoxImportToggleButton = new QToolButton( m_Task );
    m_LightBoxImportToggleButton->setText(" "); // just some space that generates one short empty line of text
    m_LightBoxImportToggleButton->setPixmap( QPixmap(chili_restart_mitk_xpm) );
    m_LightBoxImportToggleButton->setAutoRaise(true);
    m_LightBoxImportToggleButton->setEnabled(true); //!! secret button to reinit application
    //connect( m_LightBoxImportToggleButton, SIGNAL(clicked()), this, SLOT(ReinitMITKApplication()) );
    QSizePolicy policy = m_LightBoxImportToggleButton->sizePolicy();
    policy.setVerStretch( 1 );
    m_LightBoxImportToggleButton->setSizePolicy( m_LightBoxImportToggleButton->sizePolicy());
    m_LightBoxImportToggleButton->setPopup( m_PopupMenu );
    m_LightBoxImportToggleButton->setPopupDelay( 1 );
    vertlayout->addWidget( m_LightBoxImportToggleButton );
    
    done = true;
  }

  if (m_MITKMainApp)
  {
    // delete old application
    m_ImportButtonLayout->remove(m_MITKMainApp);
    delete m_MITKMainApp;
  }

  // create one instance of SampleApp
  m_MITKMainApp = new SampleApp( m_Task, "sample", 0 );
  m_ImportButtonLayout->addWidget( m_MITKMainApp );
  m_ImportButtonLayout->activate();
  m_MITKMainApp->show();
  m_MITKMainApp->SetDefaultWidgetSize();
  m_MITKMainApp->viewReinitMultiWidget();

  SendStudySelectedEvent();
}


/** Return the plugininstance. */
QcPlugin* mitk::ChiliPlugin::GetQcPluginInstance()
{
  // give Chili the single instance of mitk::ChiliPlugin
  PACSPlugin::Pointer pluginInstance = PACSPlugin::GetInstance();
  ChiliPlugin::Pointer realPluginInstance = dynamic_cast<ChiliPlugin*>( pluginInstance.GetPointer() );

  // create MainApp only after a small delay
  // This was to avoid errors related to Qt translation attempts
  static bool done = false;
  if (!done)
  {
    QTimer* timer = new QTimer(realPluginInstance);
    connect( timer, SIGNAL(timeout()), realPluginInstance, SLOT(CreateSampleApp()) );
    timer->start(600, true);

    done = true;
  }

  return realPluginInstance.GetPointer();
}


mitk::ChiliPlugin* mitk::ChiliPlugin::GetChiliPluginInstance()
{
  return static_cast<ChiliPlugin*>( GetQcPluginInstance() );
}


int mitk::ChiliPlugin::GetConferenceID()
{
  return m_QmitkChiliPluginConferenceID;
}


mitk::PACSPlugin::PACSPluginCapability mitk::ChiliPlugin::GetPluginCapabilities()
{
  PACSPluginCapability result;
  result.IsPACSFunctional = true;
  result.HasLoadCapability = true;
  result.HasSaveCapability = true;
  
  #if CHILI_PLUGIN_VERSION_CODE <= CHILI_VERSION_CODE( 1, 1, 3 ) // min CHILI 3.12 (plugin interface version 1.1.3)
    result.HasSaveCapability = false;
  #endif

  return result;
}


unsigned int mitk::ChiliPlugin::GetLightboxCount()
{
  // set by lightboxTiles()
  return m_LightBoxCount;
}


unsigned int mitk::ChiliPlugin::GetActiveLightbox()
{
  QcLightboxManager* manager = lightboxManager();

  QcLightbox* activeLightbox = manager->getActiveLightbox();

  QPtrList<QcLightbox>& lightboxes = manager->getLightboxes();
  QcLightbox* current;
  unsigned int activeLightboxIndex = 0;
  for ( current = lightboxes.first(); current; current = lightboxes.next(), ++activeLightboxIndex )
  {
    if (current == activeLightbox) return activeLightboxIndex;
  }

  return 0;
}


void mitk::ChiliPlugin::lightBoxImportButtonClicked(int row)
{
  if( m_InImporting ) return; // wait until completely loaded

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
    //dont use "LoadCompleteSeries( ... )", the Buttons called LightboxImportButtons, so use the LoadImagesFromLightbox-Function
    std::vector<DataTreeNode::Pointer> resultNodes = LoadImagesFromLightbox( selectedLightbox );

    if( resultNodes.size() < 8 )
    {
      SetRelationsToDataStorage( resultNodes );
    }
    else 
    {
      int answer = QMessageBox::question( 0, tr("MITK"), 
             QString("MITK detected %1 distinct image volumes.\n"
                     "Do you want to load all of them (might slow down MITK)?").arg(resultNodes.size()),
             QString("Load all %1 image volumes").arg(resultNodes.size()),
             //QString("Ignore only single slices"),
             QString("Cancel image import") );
      if ( answer == 0 )
      {
        SetRelationsToDataStorage( resultNodes );
      }
    }

    if( selectedLightbox->currentSeries() )
      SetRelationsToDataStorage( LoadTextsFromSeries( selectedLightbox->currentSeries()->oid ) );  //load all texts

    // stupid, that this is still necessary
    DataTreePreOrderIterator treeiter( m_MITKMainApp->GetTree() );
    RenderingManager::GetInstance()->InitializeViews( &treeiter  );
    RenderingManager::GetInstance()->RequestUpdateAll();
    qApp->processEvents();
    RenderingManager::GetInstance()->RequestUpdateAll();

    m_InImporting = false;
  }
}


/** Reinitialize the application. */
void mitk::ChiliPlugin::ReinitMITKApplication()
{
  if ( QMessageBox::question( NULL, tr("MITK"), 
                                QString("Do you want to restart MITK and lose all work results?"),
                                QMessageBox::Yes | QMessageBox::Default, 
                                QMessageBox::No  | QMessageBox::Escape
                              ) == QMessageBox::Yes )
  {
    CreateSampleApp();
  }
}


#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
// TODO somebody check if there is a better way to create a temp dir with windows
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
  if ( system( tmpName.c_str() ) == -1 )
    std::cerr << "Couldn't delete " << tmpName << std::endl;

  //use the random created name
  std::string fileName = TempName;
  fileName = fileName.substr( 0, fileName.find_first_of(".") );
  fileName = fileName + "\\";
  std::string makeTmpDirectory = "mkdir " + fileName;

  //create directory
  if ( system( makeTmpDirectory.c_str() ) == -1 )
    std::cerr << "Couldn't create " << makeTmpDirectory << std::endl;

  return fileName;
}
#else
#include <stdlib.h>
/** Create a temporary directory for linux. */
std::string mitk::ChiliPlugin::GetTempDirectory()
{
  char *tmpDirectory = getenv( "TMPDIR" ); //return NULL if the TempDirectory is "/tmp/"
  std::string newTmpDirectory;
  if( !tmpDirectory )
  {
    newTmpDirectory = "/tmp/ChiliTempXXXXXX"; //mkdtemp need 6-10 dummies
  }
  else
  {
    newTmpDirectory = tmpDirectory;
    newTmpDirectory = newTmpDirectory + "/ChiliTempXXXXXX";
  }

  tmpDirectory = mkdtemp( (char*)newTmpDirectory.c_str() ); //create a new unique directory
  if( tmpDirectory)
  {
    newTmpDirectory = tmpDirectory;
    newTmpDirectory = newTmpDirectory + "/";
    return newTmpDirectory;
  }
  else
  {
    return "";
  }
}
#endif


QObject* mitk::ChiliPlugin::findProgressBar(QObject* object)
{
  // traverses the CHILI object hierarchy, looking for the
  // progressbar that pops up when a series is loaded into
  // a lightbox
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

/** Event from QcPlugin. */
void mitk::ChiliPlugin::lightboxFilled( QcLightbox* /*lbm*/ )
{
  GetStudyInformation();
  GetSeriesInformation();
}

/** Event from QcPlugin. */
void mitk::ChiliPlugin::lightboxTiles( QcLightboxManager* /*lbm*/, int tiles )
{
  m_LightBoxCount = tiles;

  for( unsigned int row = 0; row < m_LightBoxCount; ++row )
  {
    try
    {
      QIDToolButton* button = m_LightBoxImportButtons.at(row);
      button->show();
    }
    catch(...) 
    {
    }
  }
  for( unsigned int row = tiles; row < NUMBER_OF_THINKABLE_LIGHTBOXES; ++row )
  {
    try
    {
      QIDToolButton* button = m_LightBoxImportButtons.at(row);
      button->hide();
    }
    catch(...) 
    {
    }
  }

  SendLightBoxCountChangedEvent(); // tell the application
}


/** Event if a new study get selected (from QcPlugin). */
void mitk::ChiliPlugin::studySelected( study_t* /*study*/ )
{
  GetStudyInformation();   // just find out all information, remember OIDs for instance UIDs
  GetSeriesInformationList(); 
  SendStudySelectedEvent();
}


/** Throw an event, to stop the filter. */
void mitk::ChiliPlugin::AbortPACSImport()
{
  InvokeEvent( PluginAbortPACSImport() );
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


void mitk::ChiliPlugin::SetRelationsToDataStorage( std::vector<DataTreeNode::Pointer> inputNodes )
{
  for ( std::vector<DataTreeNode::Pointer>::iterator iter = inputNodes.begin();
        iter != inputNodes.end();
        ++iter )
  {
    DataStorage::GetInstance()->Add( *iter );
  }
}


mitk::PACSPlugin::StudyInformation mitk::ChiliPlugin::GetStudyInformation( const std::string& seriesInstanceUID )
{
  return m_ChiliInformation->GetStudyInformation( this, seriesInstanceUID );
}


mitk::PACSPlugin::PatientInformation mitk::ChiliPlugin::GetPatientInformation( const std::string& seriesInstanceUID )
{
  return m_ChiliInformation->GetPatientInformation( this, seriesInstanceUID );
}


mitk::PACSPlugin::SeriesInformation mitk::ChiliPlugin::GetSeriesInformation( const std::string& seriesInstanceUID )
{
  return m_ChiliInformation->GetSeriesInformation( this, seriesInstanceUID );
}

mitk::PACSPlugin::PatientInformationList mitk::ChiliPlugin::GetPatientInformationList()
{
  PatientInformationList resultInformation;
  resultInformation.clear();

  PatientInformation patient = GetPatientInformation(); // CHILI Workstation has only one active study --> one active patient
  resultInformation.push_back( patient );

  return resultInformation;
}
    
mitk::PACSPlugin::StudyInformationList mitk::ChiliPlugin::GetStudyInformationList( const PatientInformation& /* patient */ )
{
  StudyInformationList resultInformation;
  resultInformation.clear();

  StudyInformation study = GetStudyInformation(); // CHILI Workstation has only one active study
  resultInformation.push_back( study );
  
  return resultInformation;
}


mitk::PACSPlugin::SeriesInformationList mitk::ChiliPlugin::GetSeriesInformationList( const std::string& studyInstanceUID )
{
  SeriesInformationList resultInformation;
  resultInformation.clear();

  const std::string studyOID = this->GetStudyOIDFromInstanceUID( studyInstanceUID );
  resultInformation = m_ChiliInformation->GetSeriesInformationList( this, studyOID );

  return resultInformation;
}


mitk::PACSPlugin::DocumentInformation mitk::ChiliPlugin::GetDocumentInformation( const std::string& seriesInstanceUID, 
                                                                                 unsigned int instanceNumber )
{
  return m_ChiliInformation->GetDocumentInformation( this, seriesInstanceUID, instanceNumber );
}


mitk::PACSPlugin::DocumentInformationList mitk::ChiliPlugin::GetDocumentInformationList( const std::string& seriesInstanceUID )
{
  return m_ChiliInformation->GetDocumentInformationList( this, seriesInstanceUID );
}


void mitk::ChiliPlugin::SetReaderType( unsigned int readerType )
{
  m_LoadFromCHILI->SetReaderType( readerType );
}


std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromLightbox( unsigned int lightboxIndex )
{
  QcLightboxManager* manager = lightboxManager();
  QPtrList<QcLightbox>& lightboxes = manager->getLightboxes();
  QcLightbox* current;
  unsigned int currentLightboxIndex = 0;
  for ( current = lightboxes.first(); current; current = lightboxes.next(), ++currentLightboxIndex )
  {
    if ( lightboxIndex == currentLightboxIndex )
    {
      return this->LoadImagesFromLightbox( current );
    }
  }

  std::vector<DataTreeNode::Pointer> empty;
  empty.clear();
  return empty;
}


std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromLightbox( QcLightbox* lightbox )
{
  std::vector<DataTreeNode::Pointer> resultVector;
  resultVector.clear();

  if( lightbox && !m_tempDirectory.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );

    resultVector = m_LoadFromCHILI->LoadImagesFromLightbox( this, lightbox, m_tempDirectory );

    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

  return resultVector;
}

std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromSeries( std::vector<std::string> seriesInstanceUIDs )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

  if( !m_tempDirectory.empty() && !seriesInstanceUIDs.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );

    resultNodes = m_LoadFromCHILI->LoadImagesFromSeries( this, seriesInstanceUIDs, m_tempDirectory );

    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

  return resultNodes;
}

// TODO liste rein
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromSeries( const std::string& seriesInstanceUID )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

  if( !m_tempDirectory.empty() && !seriesInstanceUID.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );

    resultNodes = m_LoadFromCHILI->LoadImagesFromSeries( this, seriesInstanceUID, m_tempDirectory );

    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

  return resultNodes;
}


std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadTextsFromSeries( const std::string& seriesInstanceUID )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

  if( !m_tempDirectory.empty() && !seriesInstanceUID.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );

    resultNodes = m_LoadFromCHILI->LoadTextsFromSeries( this, seriesInstanceUID, m_tempDirectory );

    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

  return resultNodes;
}


mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadSingleText( const std::string& seriesInstanceUID, 
                                                               unsigned int instanceNumber )
{
  DataTreeNode::Pointer resultNode;

  if( !m_tempDirectory.empty() && !seriesInstanceUID.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );

    resultNode = m_LoadFromCHILI->LoadSingleText( this, seriesInstanceUID, instanceNumber, m_tempDirectory );

    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

  return resultNode;
}


void mitk::ChiliPlugin::SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, 
                                         const std::string& studyInstanceUID, 
                                         int seriesNumber, 
                                         const std::string& seriesDescription )
{
#if CHILI_PLUGIN_VERSION_CODE < CHILI_VERSION_CODE( 1, 1, 4 ) // CHILI < 3.12
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI Workstation does not support plugins saving data" );
  return;
#endif

  if( !m_tempDirectory.empty() && 
      inputNodes->begin() != inputNodes->end() && 
      !studyInstanceUID.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );

    m_SaveToCHILI->SaveAsNewSeries( this, inputNodes, studyInstanceUID, seriesNumber, seriesDescription, m_tempDirectory );

    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }
}


void mitk::ChiliPlugin::SaveToSeries( DataStorage::SetOfObjects::ConstPointer inputNodes, const std::string& seriesInstanceUID, bool overrideExistingSeries )
{
#if CHILI_PLUGIN_VERSION_CODE < CHILI_VERSION_CODE( 1, 1, 4 ) //CHILI < 3.12
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI Workstation does not support plugins saving data" );
  return;
#endif
  if( !m_tempDirectory.empty() && 
      inputNodes->begin() != inputNodes->end() && 
      !seriesInstanceUID.empty() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );

    m_SaveToCHILI->SaveToSeries( this, inputNodes, seriesInstanceUID, overrideExistingSeries, m_tempDirectory );

    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
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
      if( ! EventCollector::PostEvent(QString(str), m_Task))
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


void mitk::ChiliPlugin::OnMenuSeparateByAcquisitionNumberClicked()
{
  // toggle check mark in menu
  bool checked = m_PopupMenu->isItemChecked(4);
  checked = !checked;
  m_PopupMenu->setItemChecked(4, checked);
  
  // toggle flag in ImageNumberFilter
  m_LoadFromCHILI->SetSeparateByAcquisitionNumber(checked);
}


void mitk::ChiliPlugin::OnMenuImportFilterSelected(int id)
{
  switch (id)
  {
    case 0:
      // Image Number filter
      m_LoadFromCHILI->SetReaderType(0);
      m_PopupMenu->setItemChecked(0, true);
      m_PopupMenu->setItemChecked(1, false);
      m_PopupMenu->setItemChecked(2, false);
      break;
    case 1:
      // Single Spacing filter
      m_LoadFromCHILI->SetReaderType(2); // NO typo! This IS a little strange.
      m_PopupMenu->setItemChecked(0, false);
      m_PopupMenu->setItemChecked(1, true);
      m_PopupMenu->setItemChecked(2, false);
      break;
    case 2:
      // Spacing Set filter
      m_LoadFromCHILI->SetReaderType(1); // NO typo! This IS a little strange.
      m_PopupMenu->setItemChecked(0, false);
      m_PopupMenu->setItemChecked(1, false);
      m_PopupMenu->setItemChecked(2, true);
      break;
  }
}


const std::string mitk::ChiliPlugin::GetStudyOIDFromInstanceUID( const std::string& studyInstanceUID )
{
  std::string returnValue("");
  UID2OIDMap::iterator iter = m_StudyOIDForInstanceUID.find( studyInstanceUID );
  if ( iter != m_StudyOIDForInstanceUID.end() )
  {
    returnValue = iter->second; // OID for given instance UID;
  }
  return returnValue;
}


void mitk::ChiliPlugin::UpdateStudyOIDForInstanceUID( const std::string& studyOID, const std::string& studyInstanceUID )
{
  m_StudyOIDForInstanceUID[ studyInstanceUID ] = studyOID;
  //std::cout << "Remember study OID " << studyOID << " for UID " << studyInstanceUID << std::endl;
}


const std::string mitk::ChiliPlugin::GetSeriesOIDFromInstanceUID( const std::string& seriesInstanceUID )
{
  std::string returnValue("");
  UID2OIDMap::iterator iter = m_SeriesOIDForInstanceUID.find( seriesInstanceUID );
  if ( iter != m_SeriesOIDForInstanceUID.end() )
  {
    returnValue = iter->second; // OID for given instance UID;
  }
  return returnValue;
}


void mitk::ChiliPlugin::UpdateSeriesOIDForInstanceUID( const std::string& seriesOID, const std::string& seriesInstanceUID )
{
  m_SeriesOIDForInstanceUID[ seriesInstanceUID ] = seriesOID;
  //std::cout << "Remember series OID " << seriesOID << " for UID " << seriesInstanceUID << std::endl;
}


const std::string mitk::ChiliPlugin::GetTextOIDFromSeriesInstanceUIDAndInstanceNumber( const std::string& seriesInstanceUID, 
                                                                                       unsigned int textInstanceNumber )
{
  std::string returnValue("");
  UIDAndInstanceNumber2OIDMap::iterator iter = 
    m_TextOIDForSeriesInstanceUIDAndInstanceNumber.find( std::make_pair(seriesInstanceUID, textInstanceNumber) );
  if ( iter != m_TextOIDForSeriesInstanceUIDAndInstanceNumber.end() )
  {
    returnValue = iter->second;
  }
  return returnValue;
}


void mitk::ChiliPlugin::UpdateTextOIDFromSeriesInstanceUIDAndInstanceNumber( const std::string& textOID,
                                                                             const std::string& seriesInstanceUID, 
                                                                             unsigned int textInstanceNumber )
{
  m_TextOIDForSeriesInstanceUIDAndInstanceNumber[ std::make_pair(seriesInstanceUID, textInstanceNumber) ] = textOID;
  //std::cout << "Remember text OID " << textOID << " for UID " << seriesInstanceUID << " and instance number " << textInstanceNumber << std::endl;
}


void mitk::ChiliPlugin::UploadFileAsNewSeries( const std::string& filename,
                                               const std::string& mimeType,
                                               const std::string& studyInstanceUID, 
                                               int seriesNumber, 
                                               const std::string& seriesDescription )
{
#if CHILI_PLUGIN_VERSION_CODE < CHILI_VERSION_CODE( 1, 1, 4 ) //CHILI < 3.12
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI Workstation does not support plugins saving data" );
  return;
#endif
  m_SaveToCHILI->UploadFileAsNewSeries( this, filename, mimeType, studyInstanceUID, seriesNumber, seriesDescription );
}

void mitk::ChiliPlugin::UploadFileToSeries( const std::string& filename,
                                            const std::string& filebasename,
                                            const std::string& mimeType,
                                            const std::string& seriesInstanceUID, 
                                            bool overwriteExistingSeries )
{
#if CHILI_PLUGIN_VERSION_CODE < CHILI_VERSION_CODE( 1, 1, 4 ) //CHILI < 3.12
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI Workstation does not support plugins saving data" );
  return;
#endif
  
  m_SaveToCHILI->UploadFileToSeries( this, filename, filebasename, mimeType, seriesInstanceUID, overwriteExistingSeries );
}

void mitk::ChiliPlugin::DownloadSingleFile( const std::string& seriesInstanceUID, 
                                            unsigned int instanceNumber, 
                                            const std::string& filename )
{
  m_LoadFromCHILI->DownloadSingleFile( this, seriesInstanceUID, instanceNumber, filename );
}
