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
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <berryIEditorReference.h>
#include <berryIEditorInput.h>

#include <ctkPluginContext.h>

#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>

#include <mitkLogMacros.h>

#include <QLabel>
#include <QMessageBox>
#include <QtCore/qconfig.h>

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QChar>
#include <QByteArray>
#include <QDesktopServices>

#include "QmitkExtApplicationPlugin.h"
#include "mitkDataStorageEditorInput.h"
#include <string>

#ifdef MITK_USE_Qt5_WebEngine
  #include <QWebEngineView>
  #include <QWebEnginePage>
  #include <QUrlQuery>
#endif

class QmitkMitkWorkbenchIntroPart::Impl
{
public:
  Impl()
#ifdef MITK_USE_Qt5_WebEngine
    : View(nullptr)
#endif
  {
  }

  ~Impl()
  {
  }

#ifdef MITK_USE_Qt5_WebEngine
  QWebEngineView* View;
#endif

private:
  Impl(const Impl&);
  Impl& operator=(const Impl&);
};

#ifdef MITK_USE_Qt5_WebEngine
namespace
{
  class QmitkWebEnginePage final : public QWebEnginePage
  {
  public:
    explicit QmitkWebEnginePage(QmitkMitkWorkbenchIntroPart* introPart, QObject* parent = nullptr);
    ~QmitkWebEnginePage();

  private:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;

    QmitkMitkWorkbenchIntroPart* m_IntroPart;
  };

  QmitkWebEnginePage::QmitkWebEnginePage(QmitkMitkWorkbenchIntroPart* introPart, QObject* parent)
    : QWebEnginePage(parent),
      m_IntroPart(introPart)
  {
  }

  QmitkWebEnginePage::~QmitkWebEnginePage()
  {
  }

  bool QmitkWebEnginePage::acceptNavigationRequest(const QUrl& url, NavigationType, bool)
  {
    QString scheme = url.scheme();

    if (scheme.contains("mitk"))
    {
      if (url.path().isEmpty())
        return false;

      if (url.host().contains("perspectives"))
      {
        QString id = url.path().simplified().replace("/", "");

        auto introSite = m_IntroPart->GetIntroSite();
        auto workbenchWindow = introSite->GetWorkbenchWindow();
        auto workbench = workbenchWindow->GetWorkbench();

        workbench->ShowPerspective(id, workbenchWindow);

        auto context = QmitkExtApplicationPlugin::GetDefault()->GetPluginContext();
        auto serviceReference = context->getServiceReference<mitk::IDataStorageService>();

        mitk::IDataStorageService* service = serviceReference
          ? context->getService<mitk::IDataStorageService>(serviceReference)
          : nullptr;

        if (service)
        {
          berry::IEditorInput::Pointer editorInput(new mitk::DataStorageEditorInput(service->GetActiveDataStorage()));

          auto page = introSite->GetPage();
          auto editorPart = page->FindEditor(editorInput);

          if (editorPart.IsNotNull())
            page->Activate(editorPart);
        }
      }
    }
    else if (scheme.contains("http"))
    {
      QDesktopServices::openUrl(url);
    }
    else
    {
      return true;
    }

    return false;
  }
}
#endif

QmitkMitkWorkbenchIntroPart::QmitkMitkWorkbenchIntroPart()
  : m_Controls(nullptr),
    m_Impl(new Impl)
{
  berry::IPreferences::Pointer workbenchPrefs = QmitkExtApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
#ifdef MITK_USE_Qt5_WebEngine
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
#else
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, false);
#endif
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
#ifdef MITK_USE_Qt5_WebEngine
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
#else
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, false);
#endif
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

  delete m_Impl;
}

void QmitkMitkWorkbenchIntroPart::CreateQtPartControl(QWidget* parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkWelcomeScreenViewControls;
    m_Controls->setupUi(parent);

#ifdef MITK_USE_Qt5_WebEngine
    // create a QWebView as well as a QWebPage and QWebFrame within the QWebview
    m_Impl->View = new QWebEngineView(parent);

    auto page = new QmitkWebEnginePage(this, parent);
    m_Impl->View->setPage(page);

    QUrl urlQtResource(QString("qrc:/org.mitk.gui.qt.welcomescreen/mitkworkbenchwelcomeview.html"),  QUrl::TolerantMode );
    m_Impl->View->load( urlQtResource );

    // adds the webview as a widget
    parent->layout()->addWidget(m_Impl->View);
#endif

    this->CreateConnections();
  }
}

void QmitkMitkWorkbenchIntroPart::CreateConnections()
{
}

void QmitkMitkWorkbenchIntroPart::StandbyStateChanged(bool /*standby*/)
{
}

void QmitkMitkWorkbenchIntroPart::SetFocus()
{
}
