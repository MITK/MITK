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

#include "QmitkMitkWorkbenchIntroPart.h"

#include <berryIWorkbenchWindow.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveRegistry.h>
#include <berryWorkbenchPreferenceConstants.h>
#include <berryIPreferences.h>

#include <berryIEditorReference.h>
#include <berryIEditorInput.h>

#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>

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

#include "QmitkExtApplicationPlugin.h"
#include "mitkDataStorageEditorInput.h"
#include <string>

QmitkMitkWorkbenchIntroPart::QmitkMitkWorkbenchIntroPart()
  : m_Controls(NULL)
{
  berry::IPreferences::Pointer workbenchPrefs = QmitkExtApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
  workbenchPrefs->Flush();
}

QmitkMitkWorkbenchIntroPart::~QmitkMitkWorkbenchIntroPart()
{
  // if the workbench is not closing (that means, welcome screen was closed explicitly), set "Show_intro" false
  if (!this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
  {
    berry::IPreferences::Pointer workbenchPrefs = QmitkExtApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, false);
    workbenchPrefs->Flush();
  }
  else
  {
    berry::IPreferences::Pointer workbenchPrefs = QmitkExtApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
    workbenchPrefs->Flush();
  }

  // if workbench is not closing (Just welcome screen closing), open last used perspective
  if (this->GetIntroSite()->GetPage()->GetPerspective()->GetId()
    == "org.mitk.mitkworkbench.perspectives.editor" && !this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
    {
    berry::IPerspectiveDescriptor::Pointer perspective = this->GetIntroSite()->GetWorkbenchWindow()->GetWorkbench()->GetPerspectiveRegistry()->FindPerspectiveWithId("org.mitk.mitkworkbench.perspectives.editor");
    if (perspective)
    {
      this->GetIntroSite()->GetPage()->SetPerspective(perspective);
    }
  }

}


void QmitkMitkWorkbenchIntroPart::CreateQtPartControl(QWidget* parent)
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

    QUrl urlQtResource(QString("qrc:/org.mitk.gui.qt.welcomescreen/mitkworkbenchwelcomeview.html"),  QUrl::TolerantMode );
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
void QmitkMitkWorkbenchIntroPart::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_view->page()), SIGNAL(linkClicked(const QUrl& )), this, SLOT(DelegateMeTo(const QUrl& )) );
  }
}


void QmitkMitkWorkbenchIntroPart::DelegateMeTo(const QUrl& showMeNext)
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

      // search the Workbench for opened StdMultiWidgets to ensure the focus does not stay on the welcome screen and is switched to
      // an StdMultiWidget if one available
      mitk::IDataStorageService::Pointer service =
          berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);
      berry::IEditorInput::Pointer editorInput;
      editorInput = new mitk::DataStorageEditorInput( service->GetActiveDataStorage() );

      // the solution is not clean, but the dependency to the StdMultiWidget was removed in order to fix a crash problem
      // as described in Bug #11715
      // This is the correct way : use the static string ID variable
      // berry::IEditorPart::Pointer editor = GetIntroSite()->GetPage()->FindEditors( editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID );
      // QuickFix: we use the same string for an local variable
      const std::string stdEditorID = "org.mitk.editors.stdmultiwidget";

      // search for opened StdMultiWidgetEditors
      std::vector<berry::IEditorReference::Pointer> editorList = GetIntroSite()->GetPage()->FindEditors( editorInput, stdEditorID, 1 );

      // if an StdMultiWidgetEditor open was found, give focus to it
      if(editorList.size())
      {
        GetIntroSite()->GetPage()->Activate( editorList[0]->GetPart(true) );
      }

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

void QmitkMitkWorkbenchIntroPart::StandbyStateChanged(bool standby)
{

}


void QmitkMitkWorkbenchIntroPart::SetFocus()
{

}
