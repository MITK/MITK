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

#include "mitkChiliPlugin.h"

//CHILI
#include <chili/cdbTypes.h>  //series_t*, study_t*, ...
#include <chili/qclightboxmanager.h>  //get newLightbox, currentLightbox
//MITK-Plugin
#include "mitkChiliMacros.h"
#include "mitkChiliPluginEvents.h"
#include "mitkChiliPluginFactory.h"
#include "mitkLightBoxImageReader.h"
//teleconference
#include <mitkConferenceToken.h>
#include <mitkConferenceEventMapper.h>
#include <QmitkChili3ConferenceKitFactory.h>
//MITK
#include <QmitkStdMultiWidget.h>
#include <QmitkEventCollector.h>
#include <QcMITKTask.h>
#include <SampleApp.h>
#include <mitkProgressBar.h>
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
#include <qsplitter.h>
//XPM
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
  m_LightBoxCount( 0 ),
  m_InImporting (false)
{
  task = new QcMITKTask( xpm(), s_Parent, name() );

  EventCollector* logger = new EventCollector();
  qApp->installEventFilter(logger);

  m_tempDirectory = GetTempDirectory();

  if( m_tempDirectory == "" )
    QMessageBox::information( 0, "MITK", "MITK was not able to create a temporary directory.\nYou can only load data from CHILI using the yellow light box import buttons." );
  else
    std::cout << "CHILIPlugin: Create and use directory "<< m_tempDirectory << std::endl;

#ifdef CHILI_PLUGIN_VERSION_CODE
  m_ChiliInformation = CHILIInformation::New();
  m_LoadFromCHILI = LoadFromCHILI::New();
  m_SaveToCHILI = SaveToCHILI::New();
#endif
}

/** Destructor */
mitk::ChiliPlugin::~ChiliPlugin()
{
  if( m_tempDirectory != "" )
  {
    std::string removeDirectory = "";

    #ifdef WIN32
      removeDirectory = "rmdir /S /Q " + m_tempDirectory;
      std::cout << "CHILIPlugin: Delete directory "<< m_tempDirectory << std::endl;
    #else
      removeDirectory = "rm -r " + m_tempDirectory;
      std::cout << "CHILIPlugin: Delete directory "<< m_tempDirectory << std::endl;
    #endif

    system( removeDirectory.c_str() );
  }

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
  app->SetDefaultWidgetSize();
  app->viewReinitMultiWidget();

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
    timer->start(600, true);

    done = true;
  }

  return realPluginInstance.GetPointer();
}

/** Return the ConferenceID. */
int mitk::ChiliPlugin::GetConferenceID()
{
  return m_QmitkChiliPluginConferenceID;
}

/** Return the current CHILI-Plugin-Capabilities. */
mitk::PACSPlugin::PACSPluginCapability mitk::ChiliPlugin::GetPluginCapabilities()
{
  PACSPluginCapability result;
  result.isPlugin = true;
  #ifndef CHILI_PLUGIN_VERSION_CODE
    result.canLoad = false;
    result.canSave = false;
  #else
    result.canLoad = true;
    #if CHILI_VERSION_CODE( 1, 1, 3 ) < CHILI_PLUGIN_VERSION_CODE //min. CHILI 3.12
      result.canSave = true;
    #else
      result.canSave = false;
    #endif
  #endif
  return result;
}

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
    //dont use "LoadCompleteSeries( ... )", the Buttons called LightboxImportButtons, so use the LoadImagesFromLightbox-Function
    std::vector<DataTreeNode::Pointer> resultNodes = LoadImagesFromLightbox( selectedLightbox );

    if( resultNodes.size() > 8 )
    {
      if( QMessageBox::question( 0, tr("MITK"), QString("MITK detected %1 distinct image volumes.\nDo you want to load all of them (might slow down MITK)?").arg(resultNodes.size()), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
      {
        SetRelationsToDataStorage( resultNodes );
      }
      else
      {
        for( unsigned int n = 0; n < resultNodes.size(); n++ )
        {
          BaseProperty::Pointer propertyImageNumber = resultNodes[n]->GetProperty( "NumberOfSlices" );
          if( propertyImageNumber.IsNull() || ( propertyImageNumber.IsNotNull() && propertyImageNumber->GetValueAsString() != "1" ) )
          {
            std::vector<mitk::DataTreeNode::Pointer> tmpVec;
            tmpVec.clear();
            tmpVec.push_back( resultNodes[n] );
            SetRelationsToDataStorage( tmpVec );
          }
        }
      }
    }
    else
      SetRelationsToDataStorage( resultNodes );

#ifdef CHILI_PLUGIN_VERSION_CODE
    if( selectedLightbox->currentSeries() )
      SetRelationsToDataStorage( LoadTextsFromSeries( selectedLightbox->currentSeries()->oid ) );  //load all texts
#endif

    // stupid, that this is still necessary
    DataTreePreOrderIterator treeiter( app->GetTree() );
    RenderingManager::GetInstance()->InitializeViews( &treeiter  );
    RenderingManager::GetInstance()->RequestUpdateAll();
    qApp->processEvents();
    m_InImporting = false;
  }
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

/** Event if a new study get selected (from QcPlugin). */
void mitk::ChiliPlugin::studySelected( study_t* /*study*/ )
{
  SendStudySelectedEvent();
}

/** Throw an event, to stop the filter. */
void mitk::ChiliPlugin::SendAbortFilterEvent()
{
  InvokeEvent( PluginAbortFilter() );
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
#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllImagesFromSeries)." );
#else
  //es gibt zwei Möglichkeiten:
  // - zum einen alles in die DataStorage hängen und anschließend die Beziehungen zwischen den Daten herstellen
  // - zum anderen kann man während des Einfügens in die DataStorage die Beziehungen berücksichtigen.
    // Das Problem dabei ist, dass dazu die Daten sortiert werden müssen (in der Form "PSRelationInformation"/"GetStudyRelationInformation()"). Diese Sortierung müsste unterscheiden, ob sich die Daten in der DataStorage befinden, oder gerade übergeben werden. Ebenfalls müssten die Daten die in der Beziehungen vorkommen, aber nicht übergeben werden bzw. nicht in der DataStorage sind, beachtet werden. Beim Einfügen der Daten müsste dann noch beachtet werden, ob die Beziehung beim Einfügen ("ADD()") erstellt werden kann, oder die Daten eingefügt und dann die Beziehung erstellt werden muss (Parent schon in DataStorage).
  // -> Variante eins wird verwendet
  if( !inputNodes.empty() )
  {
    //add nodes
    for( unsigned int x = 0; x < inputNodes.size(); x++ )
      if( inputNodes[x].IsNotNull() )
        DataStorage::GetInstance()->Add( inputNodes[x] );

    //create relations
    PACSPlugin::PSRelationInformationList relationList = GetStudyRelationInformation();  //get information of current study
    for( PACSPlugin::PSRelationInformationList::iterator relationIter = relationList.begin(); relationIter != relationList.end(); relationIter++ )
    {
      //search for current Label and OID at DataStorage
      DataStorage::SetOfObjects::ConstPointer allNodes = DataStorage::GetInstance()->GetAll();
      DataStorage::SetOfObjects::const_iterator parentNodeIter;

      for( parentNodeIter = allNodes->begin(); parentNodeIter != allNodes->end(); parentNodeIter++ )  //search at "DataStorage"
      {
        if( ( (*parentNodeIter)->GetProperty( "SeriesOID" ) && (*parentNodeIter)->GetProperty( "VolumeLabel" ) ) && ( (*parentNodeIter)->GetProperty( "SeriesOID" )->GetValueAsString() == relationIter->OID && (*parentNodeIter)->GetProperty( "VolumeLabel" )->GetValueAsString() == relationIter->Label ) )
          break;
      }

      if( parentNodeIter != allNodes->end() )
      {
        for( std::list<std::string>::iterator currentChildren = relationIter->ChildLabel.begin(); currentChildren != relationIter->ChildLabel.end(); currentChildren++ )  //use all children
        {
          //first we need the seriesOID additinaly
          PACSPlugin::PSRelationInformationList::iterator tmpIter;
          for( tmpIter = relationList.begin(); tmpIter != relationList.end(); tmpIter++ )
            if( tmpIter->Label == (*currentChildren) )
              break;
          if( tmpIter != relationList.end() )
          {
            //search for child-Label and OID at DataStorage
            DataStorage::SetOfObjects::const_iterator childNodeIter;

            for( childNodeIter = allNodes->begin(); childNodeIter != allNodes->end(); childNodeIter++ )  //search at "DataStorage"
            {
              if( ( (*childNodeIter)->GetProperty( "SeriesOID" ) && (*childNodeIter)->GetProperty( "VolumeLabel" ) ) && ( (*childNodeIter)->GetProperty( "SeriesOID" )->GetValueAsString() == tmpIter->OID && (*childNodeIter)->GetProperty( "VolumeLabel" )->GetValueAsString() == tmpIter->Label ) )
                break;
            }

            if( childNodeIter != allNodes->end() )  //add relation to DataStorage
            {
              //DataStorage::GetInstance()->???( parentIter, childIter ); //its possible, that the relation always exist and the function have to check for circle
            }
          }
        }
      }
    }
  }
#endif
}

/** ---- CHILIInformation ---- */

/** Return the studyinformation. If you dont set the seriesOID, you get the current selected study. If you want a specific study, set the OID. If no study could found, this function return StudyInformation.OID == "". */
mitk::PACSPlugin::StudyInformation mitk::ChiliPlugin::GetStudyInformation( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  StudyInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your CHILI version does not support this function (GetStudyInformation)." );
#else
  resultInformation = m_ChiliInformation->GetStudyInformation( this, seriesOID );
#endif
  return resultInformation;
}

/** Return the patientinformation. If you dont set the seriesOID, you get the current selected patient. If you want a specific patient, set the OID. If no patient could found, this function return PatientInformation.OID == "". */
mitk::PACSPlugin::PatientInformation mitk::ChiliPlugin::GetPatientInformation( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  PatientInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetPatientInformation)." );
#else
  resultInformation = m_ChiliInformation->GetPatientInformation( this, seriesOID );
#endif
  return resultInformation;
}

/** Return the seriesinformation. If you dont set the seriesOID, you get the current selected series. If you want a specific series, set the OID. If no series could found, this function return SeriesInformation.OID == "". */
mitk::PACSPlugin::SeriesInformation mitk::ChiliPlugin::GetSeriesInformation( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  SeriesInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetSeriesInformation)." );
#else
  resultInformation = m_ChiliInformation->GetSeriesInformation( this, seriesOID );
#endif
  return resultInformation;
}

/** Return the seriesinformationlist. If you dont set the studyOID, you get the seriesList of the current selected study. If you want a list of a specific study, set the OID. If no series could found, this function returns an empty list. */
mitk::PACSPlugin::SeriesInformationList mitk::ChiliPlugin::GetSeriesInformationList( const std::string& mitkHideIfNoVersionCode( studyOID ) )
{
  SeriesInformationList resultInformation;
  resultInformation.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetSeriesInformationList)." );
#else
  resultInformation = m_ChiliInformation->GetSeriesInformationList( this, studyOID );
#endif
  return resultInformation;
}

/** Return the textinformation. You have to set the textOID to get the information. If no text could found, this function return TextInformation.OID == "". */
mitk::PACSPlugin::TextInformation mitk::ChiliPlugin::GetTextInformation( const std::string& mitkHideIfNoVersionCode( textOID ) )
{
  TextInformation resultInformation;
  resultInformation.OID = "";

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetTextInformation)." );
#else
  resultInformation = m_ChiliInformation->GetTextInformation( this, textOID );
#endif
  return resultInformation;
}

/** Return a list of all textinformation. You have to set the seriesOID to get the textList. If no texts could found, this function returns an empty list. This function dont return the text which used to save and load the parent-child-relationship. */
mitk::PACSPlugin::TextInformationList mitk::ChiliPlugin::GetTextInformationList( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  TextInformationList resultInformation;
  resultInformation.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (GetTextInformation)." );
#else
  resultInformation = m_ChiliInformation->GetTextInformationList( this, seriesOID );
#endif
  return resultInformation;
}

/** ---- LoadFromCHILI ---- */

/** In the Parent-Child-XML-File are volumes saved. This function return all volumes to a given seriesOID */
mitk::PACSPlugin::PSRelationInformationList mitk::ChiliPlugin::GetSeriesRelationInformation( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  PSRelationInformationList resultInformation;
  resultInformation.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your CHILI version does not support this function (GetSeriesRelationInformation)." );
#else
  if( seriesOID != "" && m_tempDirectory != "" )
    resultInformation = m_LoadFromCHILI->GetSeriesRelationInformation( this, seriesOID, m_tempDirectory );
#endif
  return resultInformation;
}

/** In the Parent-Child-XML-File are volumes saved. This function return all volumes to a given studyOID */
mitk::PACSPlugin::PSRelationInformationList mitk::ChiliPlugin::GetStudyRelationInformation( const std::string& mitkHideIfNoVersionCode( studyOID ) )
{
  PSRelationInformationList resultInformation;
  resultInformation.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your CHILI version does not support this function (GetStudyInformation)." );
#else
  if( m_tempDirectory != "" )
    resultInformation = m_LoadFromCHILI->GetStudyRelationInformation( this, studyOID, m_tempDirectory );
#endif
  return resultInformation;
}

/** Set the internal reader which used to combine slices.
0 = ImageNumberFilter; 1 = SpacingSetFilter; 2 = SingleSpacingFilter */
void mitk::ChiliPlugin::SetReaderType( unsigned int mitkHideIfNoVersionCode( readerType ) )
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  m_LoadFromCHILI->SetReaderType( readerType );
#endif
}

/** Load all images from the given lightbox. If no lightbox set, the current lightbox get used. Chili dont support text-access via lightbox. If you want to load them, use LoadAllTextsFromSeries(...). The slices get combined with the internal set ReaderType. */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromLightbox( QcLightbox* lightbox )
{
  std::vector<DataTreeNode::Pointer> resultVector;
  resultVector.clear();
  if( lightbox != NULL && m_tempDirectory != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

#ifndef CHILI_PLUGIN_VERSION_CODE

    //use LightboxImageReader
    LightBoxImageReader::Pointer reader = LightBoxImageReader::New();
    reader->SetLightBox( lightbox );
    Image::Pointer image = reader->GetOutput();
    image->Update();
    if( image->IsInitialized() )  //create node
    {
      DataTreeNode::Pointer node = DataTreeNode::New();
      node->SetData( image );
      DataTreeNodeFactory::SetDefaultImageProperties( node );
      PropertyList::Pointer tempPropertyList = reader->GetImageTagsAsPropertyList();
      for( PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
        node->SetProperty( iter->first.c_str(), iter->second.first );
      resultVector.push_back( node );
    }

#else
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    resultVector = m_LoadFromCHILI->LoadImagesFromLightbox( this, lightbox, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
#endif

    QApplication::restoreOverrideCursor();
  }
  return resultVector;
}

/** Load all image- and text-files from series. This function use LoadAllImagesFromSeries(...) and LoadAllTextsFromSeries(...). */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadFromSeries( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadCompleteSeries)." );
#else

  if( m_tempDirectory != "" && seriesOID != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    resultNodes = m_LoadFromCHILI->LoadFromSeries( this, seriesOID, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
  return resultNodes;
}

/** This function load all images from series, therefore the FileDownload get used. Warning: CHILI use the original server-file-format. That mean that *.pic or *.dcm are possible. Dicomfiles get transformed to pic. The slices get combined with the internal set ReaderType. Should other file-formats saved, they get load with the DataTreeNodeFactory. */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadImagesFromSeries( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllImagesFromSeries)." );
#else

  if( m_tempDirectory != "" && seriesOID != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    resultNodes = m_LoadFromCHILI->LoadImagesFromSeries( this, seriesOID, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
  return resultNodes;
}

/** This function load all text-files from the series. Chili combine the filename, the OID, MimeType, ... to create the databasedirectory, so different files can be saved with the same filename. The filename from database is used to save the files. So we have to work sequently, otherwise we override the files ( twice filenames ). */
std::vector<mitk::DataTreeNode::Pointer> mitk::ChiliPlugin::LoadTextsFromSeries( const std::string& mitkHideIfNoVersionCode( seriesOID ) )
{
  std::vector<DataTreeNode::Pointer> resultNodes;
  resultNodes.clear();

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllImagesFromSeries)." );
#else

  if( m_tempDirectory != "" && seriesOID != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    resultNodes = m_LoadFromCHILI->LoadTextsFromSeries( this, seriesOID, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
  return resultNodes;
}

/** To load a single text-file, you need more than the textOID, this function search for the missing attributes and use LoadOneText( const std::string& seriesOID, const std::string& textOID, const std::string& textPath ). */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadSingleText( const std::string& mitkHideIfNoVersionCode( textOID ) )
{
  DataTreeNode::Pointer resultNode = NULL;

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllImagesFromSeries)." );
#else

  if( m_tempDirectory != "" && textOID != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    resultNode = m_LoadFromCHILI->LoadSingleText( this, textOID, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
  return resultNode;
}

/** This function load a single text-file. The file get saved to harddisk, get readed via factory ( current: mitkImageWriterFactory, mitkPointSetWriterFactory, mitkSurfaceVtkWriterFactory ) to mitk and deleted from harddisk. */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadSingleText( const std::string& mitkHideIfNoVersionCode(seriesOID), const std::string& mitkHideIfNoVersionCode(textOID), const std::string& mitkHideIfNoVersionCode(textPath) )
{
  DataTreeNode::Pointer resultNode = NULL;

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllImagesFromSeries)." );
#else

  if( m_tempDirectory != "" && seriesOID != "" && textOID != "" && textPath != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    resultNode = m_LoadFromCHILI->LoadSingleText( this, seriesOID, textOID, textPath, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
  return resultNode;
}

/** This function load single elements from the parent-child-xml-file. */
mitk::DataTreeNode::Pointer mitk::ChiliPlugin::LoadParentChildElement( const std::string& mitkHideIfNoVersionCode( seriesOID ), const std::string& mitkHideIfNoVersionCode( label ) )
{
  DataTreeNode::Pointer resultNode = NULL;

#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (LoadAllImagesFromSeries)." );
#else

  if( seriesOID != "" && label != "" && m_tempDirectory != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    resultNode = m_LoadFromCHILI->LoadParentChildElement( this, seriesOID, label, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
  return resultNode;
}

/** ---- SaveToCHILI ---- */

/** This function provides a dialog where the user can decide if he want to create a new series, save to series, override, ... . Then SaveAsNewSeries(...) or SaveToSeries(...) get used. */
void mitk::ChiliPlugin::SaveToChili( DataStorage::SetOfObjects::ConstPointer mitkHideIfNoVersionCode( inputNodes ) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToChili)." );
#elif CHILI_VERSION_CODE( 1, 1, 4 ) > CHILI_PLUGIN_VERSION_CODE  //CHILI < 3.12
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToChili)." );
#else
  if( m_tempDirectory != "" && inputNodes->begin() != inputNodes->end() )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    m_SaveToCHILI->SaveToChili( this, inputNodes, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
}

/** This function create a new series and use the function SaveToSeries() . No dialog is used. */
void mitk::ChiliPlugin::SaveAsNewSeries( DataStorage::SetOfObjects::ConstPointer mitkHideIfNoVersionCode( inputNodes ), const std::string& mitkHideIfNoVersionCode( studyOID ), int mitkHideIfNoVersionCode( seriesNumber), const std::string& mitkHideIfNoVersionCode( seriesDescription ) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToChili)." );
#elif CHILI_VERSION_CODE( 1, 1, 4 ) > CHILI_PLUGIN_VERSION_CODE  //CHILI < 3.12
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToChili)." );
#else

  if( m_tempDirectory != "" && inputNodes->begin() != inputNodes->end() && studyOID != "" && seriesDescription != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    m_SaveToCHILI->SaveAsNewSeries( this, inputNodes, studyOID, seriesNumber, seriesDescription, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
}

/** This function save the nodes to via FileUpload to chili. */
void mitk::ChiliPlugin::SaveToSeries( DataStorage::SetOfObjects::ConstPointer mitkHideIfNoVersionCode( inputNodes ), const std::string& mitkHideIfNoVersionCode( seriesOID ), bool mitkHideIfNoVersionCode( overrideExistingSeries ) )
{
#ifndef CHILI_PLUGIN_VERSION_CODE
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToChili)." );
#elif CHILI_VERSION_CODE( 1, 1, 4 ) > CHILI_PLUGIN_VERSION_CODE  //CHILI < 3.12
  QMessageBox::information( 0, "MITK", "Sorry, your current CHILI version does not support this function (SaveToChili)." );
#else

  if( m_tempDirectory != "" && inputNodes->begin() != inputNodes->end() && seriesOID != "" )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    ProgressBar::GetInstance()->AddStepsToDo( 2 );
    m_SaveToCHILI->SaveToSeries( this, inputNodes, seriesOID, overrideExistingSeries, m_tempDirectory );
    ProgressBar::GetInstance()->Progress( 2 );
    QApplication::restoreOverrideCursor();
  }

#endif
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
