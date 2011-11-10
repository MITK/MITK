/*=========================================================================

 Program:   BlueBerry Platform
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

#include "QmitkDiffusionImagingAppIntroPart.h"

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
#include <QWebView>
#include <QWebPage>
#endif
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QChar>
#include <QByteArray>
#include <QDesktopServices>


#include "QmitkStdMultiWidgetEditor.h"
#include "mitkPluginActivator.h"
#include "mitkDataStorageEditorInput.h"

#include "mitkBaseDataIOFactory.h"
#include "mitkSceneIO.h"
#include "mitkProgressBar.h"
#include "mitkDataNodeFactory.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"

QmitkDiffusionImagingAppIntroPart::QmitkDiffusionImagingAppIntroPart()
  : m_Controls(NULL)
{
  berry::IPreferences::Pointer workbenchPrefs = mitkPluginActivator::GetDefault()->GetPreferencesService()->GetSystemPreferences();
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
  workbenchPrefs->Flush();
}

QmitkDiffusionImagingAppIntroPart::~QmitkDiffusionImagingAppIntroPart()
{
  // if the workbench is not closing (that means, welcome screen was closed explicitly), set "Show_intro" false
  if (!this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
  {
    berry::IPreferences::Pointer workbenchPrefs = mitkPluginActivator::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, false);
    workbenchPrefs->Flush();
  }
  else
  {
    berry::IPreferences::Pointer workbenchPrefs = mitkPluginActivator::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
    workbenchPrefs->Flush();
  }

  // if workbench is not closing (Just welcome screen closing), open last used perspective
  if (this->GetIntroSite()->GetPage()->GetPerspective()->GetId()
    == "org.mitk.diffusionimagingapp.perspectives.welcome" && !this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
    {
    berry::IPerspectiveDescriptor::Pointer perspective = this->GetIntroSite()->GetWorkbenchWindow()->GetWorkbench()->GetPerspectiveRegistry()->FindPerspectiveWithId("org.mitk.diffusionimagingapp.perspectives.diffusionimagingapp");
    if (perspective)
    {
      this->GetIntroSite()->GetPage()->SetPerspective(perspective);
    }
  }

}


void QmitkDiffusionImagingAppIntroPart::CreateQtPartControl(QWidget* parent)
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

    QUrl urlQtResource(QString("qrc:/org.mitk.gui.qt.welcomescreen/mitkdiffusionimagingappwelcomeview.html"),  QUrl::TolerantMode );
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
void QmitkDiffusionImagingAppIntroPart::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_view->page()), SIGNAL(linkClicked(const QUrl& )), this, SLOT(DelegateMeTo(const QUrl& )) );
  }
}


void QmitkDiffusionImagingAppIntroPart::DelegateMeTo(const QUrl& showMeNext)
{
  QString scheme          = showMeNext.scheme();
  QByteArray urlHostname  = showMeNext.encodedHost();
  QByteArray urlPath      = showMeNext.encodedPath();
  QByteArray dataset      = showMeNext.encodedQueryItemValue("dataset");
  QByteArray clear        = showMeNext.encodedQueryItemValue("clear");

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
          mitk::TimeSlicedGeometry::Pointer bounds = dataStorage->ComputeBoundingGeometry3D(rs);
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

void QmitkDiffusionImagingAppIntroPart::StandbyStateChanged(bool standby)
{

}


void QmitkDiffusionImagingAppIntroPart::SetFocus()
{

}
