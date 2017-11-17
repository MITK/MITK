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

#include "QmitkDiffusionImagingAppIntroPart.h"

#include "mitkNodePredicateDataType.h"
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveRegistry.h>
#include <berryWorkbenchPreferenceConstants.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>

#include <berryIEditorReference.h>
#include <berryIEditorInput.h>

#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>

#include <mitkLogMacros.h>

#include <QLabel>
#include <QMessageBox>
#include <QtCore/qconfig.h>

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QUrlQuery>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QChar>
#include <QByteArray>
#include <QDesktopServices>

#include "QmitkDiffusionApplicationPlugin.h"
#include "mitkDataStorageEditorInput.h"
#include <string>

#include "mitkProgressBar.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"

namespace
{
  class QmitkDiffusionWebEnginePage final : public QWebEnginePage
  {
  public:
    explicit QmitkDiffusionWebEnginePage(QmitkDiffusionImagingAppIntroPart* introPart, QObject* parent = nullptr);
    ~QmitkDiffusionWebEnginePage() override;

  private:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;

    QmitkDiffusionImagingAppIntroPart* m_IntroPart;
  };

  QmitkDiffusionWebEnginePage::QmitkDiffusionWebEnginePage(QmitkDiffusionImagingAppIntroPart* introPart, QObject* parent)
    : QWebEnginePage(parent),
    m_IntroPart(introPart)
  {
  }

  QmitkDiffusionWebEnginePage::~QmitkDiffusionWebEnginePage()
  {
  }

  bool QmitkDiffusionWebEnginePage::acceptNavigationRequest(const QUrl& url, NavigationType, bool)
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

        auto context = QmitkDiffusionApplicationPlugin::GetDefault()->GetPluginContext();
        auto serviceReference = context->getServiceReference<mitk::IDataStorageService>();

        mitk::IDataStorageService* service = serviceReference
          ? context->getService<mitk::IDataStorageService>(serviceReference)
          : nullptr;

        if (service)
        {
          berry::IEditorInput::Pointer editorInput(new mitk::DataStorageEditorInput(service->GetActiveDataStorage()));

          auto page = introSite->GetPage();
          auto editors = page->FindEditors(editorInput, "org.mitk.editors.stdmultiwidget", 1);

          if (!editors.isEmpty())
            page->Activate(editors[0]->GetPart(true));
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

QmitkDiffusionImagingAppIntroPart::QmitkDiffusionImagingAppIntroPart()
  : m_Controls(nullptr)
{
  berry::IPreferences::Pointer workbenchPrefs = QmitkDiffusionApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
  workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
  workbenchPrefs->Flush();
}

QmitkDiffusionImagingAppIntroPart::~QmitkDiffusionImagingAppIntroPart()
{
  // if the workbench is not closing (that means, welcome screen was closed explicitly), set "Show_intro" false
  if (!this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
  {
    berry::IPreferences::Pointer workbenchPrefs = QmitkDiffusionApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, false);
    workbenchPrefs->Flush();
  }
  else
  {
    berry::IPreferences::Pointer workbenchPrefs = QmitkDiffusionApplicationPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences();
    workbenchPrefs->PutBool(berry::WorkbenchPreferenceConstants::SHOW_INTRO, true);
    workbenchPrefs->Flush();
  }

  // if workbench is not closing (Just welcome screen closing), open last used perspective
  if (this->GetIntroSite()->GetPage()->GetPerspective()->GetId()
    == "org.mitk.diffusionimagingapp.perspectives.welcome"
      && !this->GetIntroSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->IsClosing())
    {
    berry::IPerspectiveDescriptor::Pointer perspective = this->GetIntroSite()->GetWorkbenchWindow()->GetWorkbench()->GetPerspectiveRegistry()->FindPerspectiveWithId("org.mitk.perspectives.diffusiondefault");
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

    m_view = new QWebEngineView(parent);

    auto page = new QmitkDiffusionWebEnginePage(this, parent);
    m_view->setPage(page);

    QUrl urlQtResource(QString("qrc:/org.mitk.gui.qt.welcomescreen/mitkdiffusionimagingappwelcomeview.html"),  QUrl::TolerantMode );
    m_view->load( urlQtResource );

    // adds the webview as a widget
    parent->layout()->addWidget(m_view);
    this->CreateConnections();
  }
}

void QmitkDiffusionImagingAppIntroPart::CreateConnections()
{
}

void QmitkDiffusionImagingAppIntroPart::StandbyStateChanged(bool)
{
}

void QmitkDiffusionImagingAppIntroPart::SetFocus()
{
}
