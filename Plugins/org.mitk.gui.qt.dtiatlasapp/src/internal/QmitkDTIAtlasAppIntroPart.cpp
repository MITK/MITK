/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkDTIAtlasAppIntroPart.h"

#include "mitkNodePredicateDataType.h"
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveRegistry.h>
#include <berryWorkbenchPreferenceConstants.h>
#include <berryIPreferences.h>
#include <berryIEditorReference.h>

#include <mitkLogMacros.h>

#include <QLabel>
#include <QMessageBox>
#include <QtCore/qconfig.h>
#ifdef QT_WEBKIT
#  include <QWebView>
#  include <QWebPage>
#  if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#    include <QUrlQuery>
#  endif
#endif
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QChar>
#include <QByteArray>
#include <QDesktopServices>

#include "QmitkStdMultiWidget.h"
#include "QmitkStdMultiWidgetEditor.h"
#include "QmitkDTIAtlasAppApplicationPlugin.h"
#include "mitkDataStorageEditorInput.h"

#include "mitkBaseDataIOFactory.h"
#include "mitkSceneIO.h"
#include "mitkProgressBar.h"
#include "mitkDataNodeFactory.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"

QmitkDTIAtlasAppIntroPart::QmitkDTIAtlasAppIntroPart()
  : m_Controls(NULL)
{
  berry::IPreferences::Pointer workbenchPrefs = QmitkDTIAtlasAppApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
  workbenchPrefs->Flush();
}

QmitkDTIAtlasAppIntroPart::~QmitkDTIAtlasAppIntroPart()
{
  // if the workbench is not closing (that means, welcome screen was closed explicitly), set "Show_intro" false
  if (!this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
  {
    berry::IPreferences::Pointer workbenchPrefs = QmitkDTIAtlasAppApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, false);
    workbenchPrefs->Flush();
  }
  else
  {
    berry::IPreferences::Pointer workbenchPrefs = QmitkDTIAtlasAppApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
    workbenchPrefs->Flush();
  }

  // if workbench is not closing (Just welcome screen closing), open last used perspective
  if (this->GetIntroSite()->GetPage()->GetPerspective()->GetId()
    == "org.mitk.dtiatlasapp.perspectives.welcome" && !this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
    {
    berry::IPerspectiveDescriptor::Pointer perspective = this->GetIntroSite()->GetWorkbenchWindow()->GetWorkbench()->GetPerspectiveRegistry()->FindPerspectiveWithId("org.mitk.dtiatlasapp.perspectives.dtiatlasapp");
    if (perspective)
    {
      this->GetIntroSite()->GetPage()->SetPerspective(perspective);
    }
  }

}


void QmitkDTIAtlasAppIntroPart::CreateQtPartControl(QWidget* parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkWelcomeScreenViewControls;
    m_Controls->setupUi(parent);
#ifdef QT_WEBKIT

    // create a QWebView as well as a QWebPage and QWebFrame within the QWebview
    m_view = new QWebView(parent);
    m_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    QUrl urlQtResource(QString("qrc:/org.mitk.gui.qt.welcomescreen/mitkdtiatlasappwelcomeview.html"),  QUrl::TolerantMode );
    m_view->load( urlQtResource );

    // adds the webview as a widget
    parent->layout()->addWidget(m_view);
    this->CreateConnections();
#else
    parent->layout()->addWidget(new QLabel("<h1><center>Please install Qt with the WebKit option to see cool pictures!</center></h1>"));
#endif
  }
}

#ifdef QT_WEBKIT
void QmitkDTIAtlasAppIntroPart::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_view->page()), SIGNAL(linkClicked(const QUrl& )), this, SLOT(DelegateMeTo(const QUrl& )) );
  }
}


void QmitkDTIAtlasAppIntroPart::DelegateMeTo(const QUrl& showMeNext)
{
  QString scheme          = showMeNext.scheme();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QByteArray urlHostname  = showMeNext.encodedHost();
  QByteArray urlPath      = showMeNext.encodedPath();
  QByteArray dataset      = showMeNext.encodedQueryItemValue("dataset");
  QByteArray clear        = showMeNext.encodedQueryItemValue("clear");
#else
  QByteArray urlHostname  = QUrl::toAce(showMeNext.host());
  QByteArray urlPath      = showMeNext.path().toLatin1();
  QUrlQuery query(showMeNext);
  QByteArray dataset      = query.queryItemValue("dataset").toLatin1();
  QByteArray clear        = query.queryItemValue("clear").toLatin1();//showMeNext.encodedQueryItemValue("clear");
#endif

  if (scheme.isEmpty()) MITK_INFO << " empty scheme of the to be delegated link" ;

  // if the scheme is set to mitk, it is to be tested which action should be applied
  if (scheme.contains(QString("mitk")) )
  {
    if(urlPath.isEmpty() ) MITK_INFO << " mitk path is empty " ;

    // searching for the perspective keyword within the host name
    if(urlHostname.contains(QByteArray("perspectives")) )
    {
      // the simplified method removes every whitespace
      // ( whitespace means any character for which the standard C++ isspace() method returns true)
      urlPath = urlPath.simplified();
      QString tmpPerspectiveId(urlPath.data());
      tmpPerspectiveId.replace(QString("/"), QString("") );
      std::string perspectiveId  = tmpPerspectiveId.toStdString();

      // is working fine as long as the perspective id is valid, if not the application crashes
      GetIntroSite()->GetWorkbenchWindow()->GetWorkbench()->ShowPerspective(perspectiveId, GetIntroSite()->GetWorkbenchWindow() );

      mitk::DataStorageEditorInput::Pointer editorInput;
      editorInput = new mitk::DataStorageEditorInput();
      berry::IEditorPart::Pointer editor = GetIntroSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);


      QmitkStdMultiWidgetEditor::Pointer multiWidgetEditor;
      mitk::DataStorage::Pointer dataStorage;

      if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNull())
      {
        editorInput = new mitk::DataStorageEditorInput();
        dataStorage = editorInput->GetDataStorageReference()->GetDataStorage();
      }
      else
      {
        multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
        multiWidgetEditor->GetStdMultiWidget()->RequestUpdate();
        dataStorage = multiWidgetEditor->GetEditorInput().Cast<mitk::DataStorageEditorInput>()->GetDataStorageReference()->GetDataStorage();
      }

      bool dsmodified = false;

      QString *fileName = new QString(dataset.data());

      if ( fileName->right(5) == ".mitk" )
      {
        mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

        bool clearDataStorageFirst(false);
        QString *sClear = new QString(clear.data());
        if ( sClear->right(4) == "true" )
        {
          clearDataStorageFirst = true;
        }

        mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
        dataStorage = sceneIO->LoadScene( fileName->toLocal8Bit().constData(), dataStorage, clearDataStorageFirst );
        dsmodified = true;
        mitk::ProgressBar::GetInstance()->Progress(2);
      }
      else
      {
        mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();
        try
        {
          nodeReader->SetFileName(fileName->toLocal8Bit().data());
          nodeReader->Update();
          for ( unsigned int i = 0 ; i < nodeReader->GetNumberOfOutputs( ); ++i )
          {
            mitk::DataNode::Pointer node;
            node = nodeReader->GetOutput(i);
            if ( node->GetData() != NULL )
            {
              dataStorage->Add(node);
              dsmodified = true;
            }
          }
        }
        catch(...)
        {
          MITK_INFO << "Could not open file!";
        }
      }



      if(dataStorage.IsNotNull() && dsmodified)
      {
        // get all nodes that have not set "includeInBoundingBox" to false
        mitk::NodePredicateNot::Pointer pred
            = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
                                                                           , mitk::BoolProperty::New(false)));

        mitk::DataStorage::SetOfObjects::ConstPointer rs = dataStorage->GetSubset(pred);

        if(rs->Size() > 0)
        {
          // calculate bounding geometry of these nodes
          mitk::TimeGeometry::Pointer bounds = dataStorage->ComputeBoundingGeometry3D(rs);
          // initialize the views to the bounding geometry
          mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
        }
      }





    }
    // searching for the load
    if(urlHostname.contains(QByteArray("perspectives")) )
    {
      // the simplified method removes every whitespace
      // ( whitespace means any character for which the standard C++ isspace() method returns true)
      urlPath = urlPath.simplified();
      QString tmpPerspectiveId(urlPath.data());
      tmpPerspectiveId.replace(QString("/"), QString("") );
      std::string perspectiveId  = tmpPerspectiveId.toStdString();

      // is working fine as long as the perspective id is valid, if not the application crashes
      GetIntroSite()->GetWorkbenchWindow()->GetWorkbench()->ShowPerspective(perspectiveId, GetIntroSite()->GetWorkbenchWindow() );

      mitk::DataStorageEditorInput::Pointer editorInput;
      editorInput = new mitk::DataStorageEditorInput();
      GetIntroSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
    }
    else
    {
      MITK_INFO << "Unkown mitk action keyword (see documentation for mitk links)" ;
    }
  }
  // if the scheme is set to http, by default no action is performed, if an external webpage needs to be
  // shown it should be implemented below
  else if (scheme.contains(QString("http")) )
  {
    QDesktopServices::openUrl(showMeNext);
//    m_view->load( ) ;
  }
  else if(scheme.contains("qrc"))
  {
    m_view->load(showMeNext);
  }

}

#endif

void QmitkDTIAtlasAppIntroPart::StandbyStateChanged(bool standby)
{

}


void QmitkDTIAtlasAppIntroPart::SetFocus()
{

}
