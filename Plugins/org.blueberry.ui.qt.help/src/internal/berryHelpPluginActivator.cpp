/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHelpPluginActivator.h"

#include "berryHelpContentView.h"
#include "berryHelpIndexView.h"
#include "berryHelpSearchView.h"
#include "berryHelpEditor.h"
#include "berryHelpEditorInput.h"
#include "berryHelpEditorInputFactory.h"
#include "berryHelpPerspective.h"

#include "berryQHelpEngineConfiguration.h"
#include "berryQHelpEngineWrapper.h"

#include <berryPlatformUI.h>

#include <service/event/ctkEventConstants.h>

#include <QDir>
#include <QDateTime>

namespace berry {

class HelpPerspectiveListener : public IPerspectiveListener
{
public:

  Events::Types GetPerspectiveEventTypes() const override;

  using IPerspectiveListener::PerspectiveChanged;

  void PerspectiveOpened(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective) override;
  void PerspectiveChanged(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective, const QString &changeId) override;
};

class HelpWindowListener : public IWindowListener
{
public:

  HelpWindowListener();
  ~HelpWindowListener() override;

  void WindowClosed(const IWorkbenchWindow::Pointer& window) override;
  void WindowOpened(const IWorkbenchWindow::Pointer& window) override;

private:

  // We use the same perspective listener for every window
  QScopedPointer<IPerspectiveListener> perspListener;
};


HelpPluginActivator* HelpPluginActivator::instance = nullptr;

HelpPluginActivator::HelpPluginActivator()
  : pluginListener(nullptr)
{
  this->instance = this;
}

HelpPluginActivator::~HelpPluginActivator()
{
  instance = nullptr;
}

void
HelpPluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(berry::HelpContentView, context)
  BERRY_REGISTER_EXTENSION_CLASS(berry::HelpIndexView, context)
  BERRY_REGISTER_EXTENSION_CLASS(berry::HelpSearchView, context)
  BERRY_REGISTER_EXTENSION_CLASS(berry::HelpEditor, context)
  BERRY_REGISTER_EXTENSION_CLASS(berry::HelpEditorInputFactory, context)
  BERRY_REGISTER_EXTENSION_CLASS(berry::HelpPerspective, context)

  QFileInfo qhcInfo = context->getDataFile("qthelpcollection.qhc");
  helpEngine.reset(new QHelpEngineWrapper(qhcInfo.absoluteFilePath()));
  if (!helpEngine->setupData())
  {
    BERRY_ERROR << "QHelpEngine set-up failed: " << helpEngine->error().toStdString();
    return;
  }

  helpEngineConfiguration.reset(new QHelpEngineConfiguration(context, *helpEngine.data()));

  delete pluginListener;
  pluginListener = new QCHPluginListener(context, helpEngine.data());
  context->connectPluginListener(pluginListener, SLOT(pluginChanged(ctkPluginEvent)));

  // register all QCH files from all the currently installed plugins
  pluginListener->processPlugins();

  helpEngine->initialDocSetupDone();

  // Register a wnd listener which registers a perspective listener for each
  // new window. The perspective listener opens the help home page in the window
  // if no other help page is opened yet.
  wndListener.reset(new HelpWindowListener());
  PlatformUI::GetWorkbench()->AddWindowListener(wndListener.data());

  // Register an event handler for CONTEXTHELP_REQUESTED events
  helpContextHandler.reset(new HelpContextHandler);
  ctkDictionary helpHandlerProps;
  helpHandlerProps.insert(ctkEventConstants::EVENT_TOPIC, "org/blueberry/ui/help/CONTEXTHELP_REQUESTED");
  context->registerService<ctkEventHandler>(helpContextHandler.data(), helpHandlerProps);
}

void
HelpPluginActivator::stop(ctkPluginContext* /*context*/)
{
  delete pluginListener;
  pluginListener = nullptr;

  if (PlatformUI::IsWorkbenchRunning())
  {
    PlatformUI::GetWorkbench()->RemoveWindowListener(wndListener.data());
  }
  wndListener.reset();

  helpEngineConfiguration.reset();
  helpEngine.reset();
}

HelpPluginActivator *HelpPluginActivator::getInstance()
{
  return instance;
}

QHelpEngineWrapper& HelpPluginActivator::getQHelpEngine()
{
  return *helpEngine;
}

void HelpPluginActivator::linkActivated(IWorkbenchPage::Pointer page, const QUrl &link)
{
  IEditorInput::Pointer input(new HelpEditorInput(link));

  // see if an editor with the same input is already open
  IEditorPart::Pointer reuseEditor = page->FindEditor(input);
  if (reuseEditor)
  {
    // just activate it
    page->Activate(reuseEditor);
  }
  else
  {
    // reuse the currently active editor, if it is a HelpEditor
    reuseEditor = page->GetActiveEditor();
    if (reuseEditor.IsNotNull() && page->GetReference(reuseEditor)->GetId() == HelpEditor::EDITOR_ID)
    {
      page->ReuseEditor(reuseEditor.Cast<IReusableEditor>(), input);
      page->Activate(reuseEditor);
    }
    else
    {
      // get the last used HelpEditor instance
      QList<IEditorReference::Pointer> editors =
          page->FindEditors(IEditorInput::Pointer(nullptr), HelpEditor::EDITOR_ID, IWorkbenchPage::MATCH_ID);
      if (editors.empty())
      {
        // no HelpEditor is currently open, create a new one
        page->OpenEditor(input, HelpEditor::EDITOR_ID);
      }
      else
      {
        // reuse an existing editor
        reuseEditor = editors.front()->GetEditor(false);
        page->ReuseEditor(reuseEditor.Cast<IReusableEditor>(), input);
        page->Activate(reuseEditor);
      }
    }
  }
}

QCHPluginListener::QCHPluginListener(ctkPluginContext* context, QHelpEngine* helpEngine)
  : delayRegistration(true), context(context), helpEngine(helpEngine)
{}

void QCHPluginListener::processPlugins()
{
  QMutexLocker lock(&mutex);
  processPlugins_unlocked();
}

void QCHPluginListener::pluginChanged(const ctkPluginEvent& event)
{
  QMutexLocker lock(&mutex);
  if (delayRegistration)
  {
    this->processPlugins_unlocked();
    return;
  }

  /* Only should listen for RESOLVED and UNRESOLVED events.
   *
   * When a plugin is updated the Framework will publish an UNRESOLVED and
   * then a RESOLVED event which should cause the plugin to be removed
   * and then added back into the registry.
   *
   * When a plugin is uninstalled the Framework should publish an UNRESOLVED
   * event and then an UNINSTALLED event so the plugin will have been removed
   * by the UNRESOLVED event before the UNINSTALLED event is published.
   */
  QSharedPointer<ctkPlugin> plugin = event.getPlugin();
  switch (event.getType())
  {
  case ctkPluginEvent::RESOLVED :
    addPlugin(plugin);
    break;
  case ctkPluginEvent::UNRESOLVED :
    removePlugin(plugin);
    break;
  default:
    break;
  }
}

void QCHPluginListener::processPlugins_unlocked()
{
  if (!delayRegistration) return;

  foreach (QSharedPointer<ctkPlugin> plugin, context->getPlugins())
  {
    if (isPluginResolved(plugin))
      addPlugin(plugin);
    else
      removePlugin(plugin);
  }

  delayRegistration = false;
}

bool QCHPluginListener::isPluginResolved(QSharedPointer<ctkPlugin> plugin)
{
  return (plugin->getState() & (ctkPlugin::RESOLVED | ctkPlugin::ACTIVE | ctkPlugin::STARTING | ctkPlugin::STOPPING)) != 0;
}

void QCHPluginListener::removePlugin(QSharedPointer<ctkPlugin> plugin)
{
  // bail out if system plugin
  if (plugin->getPluginId() == 0) return;

  QFileInfo qchDirInfo = context->getDataFile("qch_files/" + QString::number(plugin->getPluginId()));
  if (qchDirInfo.exists())
  {
    QDir qchDir(qchDirInfo.absoluteFilePath());
    QStringList qchEntries = qchDir.entryList(QStringList("*.qch"));
    QStringList qchFiles;
    foreach(QString qchEntry, qchEntries)
    {
      qchFiles << qchDir.absoluteFilePath(qchEntry);
    }
    // unregister the cached qch files
    foreach(QString qchFile, qchFiles)
    {
      QString namespaceName = QHelpEngineCore::namespaceName(qchFile);
      if (namespaceName.isEmpty())
      {
        BERRY_ERROR << "Could not get the namespace for qch file " << qchFile.toStdString();
        continue;
      }
      else
      {
        if (!helpEngine->unregisterDocumentation(namespaceName))
        {
          BERRY_ERROR << "Unregistering qch namespace " << namespaceName.toStdString() << " failed: " << helpEngine->error().toStdString();
        }
      }
    }
    // clean the directory
    foreach(QString qchEntry, qchEntries)
    {
      qchDir.remove(qchEntry);
    }
  }
}

void QCHPluginListener::addPlugin(QSharedPointer<ctkPlugin> plugin)
{
  // bail out if system plugin
  if (plugin->getPluginId() == 0) return;

  QFileInfo qchDirInfo = context->getDataFile("qch_files/" + QString::number(plugin->getPluginId()));
  QUrl location(plugin->getLocation());
  QFileInfo pluginFileInfo(location.toLocalFile());

  if (!qchDirInfo.exists() || qchDirInfo.lastModified() < pluginFileInfo.lastModified())
  {
    removePlugin(plugin);

    if (!qchDirInfo.exists())
    {
      QDir().mkpath(qchDirInfo.absoluteFilePath());
    }

    QStringList localQCHFiles;
    QStringList resourceList = plugin->findResources("/", "*.qch", true);
    foreach(QString resource, resourceList)
    {
      QByteArray content = plugin->getResource(resource);
      QFile localFile(qchDirInfo.absoluteFilePath() + "/" + resource.section('/', -1));
      localFile.open(QIODevice::WriteOnly);
      localFile.write(content);
      localFile.close();
      if (localFile.error() != QFile::NoError)
      {
        BERRY_WARN << "Error writing " << localFile.fileName().toStdString()
                   << ": " << localFile.errorString().toStdString();
      }
      else
      {
        localQCHFiles << localFile.fileName();
      }
    }

    foreach(QString qchFile, localQCHFiles)
    {
      if (!helpEngine->registerDocumentation(qchFile))
      {
        BERRY_ERROR << "Registering qch file " << qchFile.toStdString() << " failed: " << helpEngine->error().toStdString();
      }
    }
  }

}

IPerspectiveListener::Events::Types HelpPerspectiveListener::GetPerspectiveEventTypes() const
{
  return Events::OPENED | Events::CHANGED;
}

void HelpPerspectiveListener::PerspectiveOpened(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective)
{
  // if no help editor is opened, open one showing the home page
  if (perspective->GetId() == HelpPerspective::ID &&
      page->FindEditors(IEditorInput::Pointer(nullptr), HelpEditor::EDITOR_ID, IWorkbenchPage::MATCH_ID).empty())
  {
    IEditorInput::Pointer input(new HelpEditorInput());
    page->OpenEditor(input, HelpEditor::EDITOR_ID);
  }
}

void HelpPerspectiveListener::PerspectiveChanged(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective, const QString &changeId)
{
  if (perspective->GetId() == HelpPerspective::ID && changeId == IWorkbenchPage::CHANGE_RESET)
  {
    PerspectiveOpened(page, perspective);
  }
}

HelpWindowListener::HelpWindowListener()
  : perspListener(new HelpPerspectiveListener())
{
  // Register perspective listener for already opened windows
  typedef QList<IWorkbenchWindow::Pointer> WndVec;
  WndVec windows = PlatformUI::GetWorkbench()->GetWorkbenchWindows();
  for (WndVec::iterator i = windows.begin(); i != windows.end(); ++i)
  {
    (*i)->AddPerspectiveListener(perspListener.data());
  }
}

HelpWindowListener::~HelpWindowListener()
{
  if (!PlatformUI::IsWorkbenchRunning()) return;

  typedef QList<IWorkbenchWindow::Pointer> WndVec;
  WndVec windows = PlatformUI::GetWorkbench()->GetWorkbenchWindows();
  for (WndVec::iterator i = windows.begin(); i != windows.end(); ++i)
  {
    (*i)->RemovePerspectiveListener(perspListener.data());
  }
}

void HelpWindowListener::WindowClosed(const IWorkbenchWindow::Pointer& window)
{
  window->RemovePerspectiveListener(perspListener.data());
}

void HelpWindowListener::WindowOpened(const IWorkbenchWindow::Pointer& window)
{
  window->AddPerspectiveListener(perspListener.data());
}

void HelpContextHandler::handleEvent(const ctkEvent &event)
{
  struct _runner : public Poco::Runnable
  {
    _runner(const ctkEvent& ev) : ev(ev) {}

    void run() override
    {
      QUrl helpUrl;
      if (ev.containsProperty("url"))
      {
        helpUrl = QUrl(ev.getProperty("url").toString());
      }
      else
      {
        helpUrl = contextUrl();
      }

      HelpPluginActivator::linkActivated(PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage(),
                                         helpUrl);
      delete this;
    }

    QUrl contextUrl() const
    {
      berry::IWorkbench* currentWorkbench = berry::PlatformUI::GetWorkbench();
      if (currentWorkbench)
      {
        berry::IWorkbenchWindow::Pointer currentWorkbenchWindow = currentWorkbench->GetActiveWorkbenchWindow();
        if (currentWorkbenchWindow)
        {
          berry::IWorkbenchPage::Pointer currentPage = currentWorkbenchWindow->GetActivePage();
          if (currentPage)
          {
            berry::IWorkbenchPart::Pointer currentPart = currentPage->GetActivePart();
            if (currentPart)
            {
              QString pluginID = currentPart->GetSite()->GetPluginId();
              QString viewID = currentPart->GetSite()->GetId();
              QString loc = "qthelp://" + pluginID + "/bundle/%1.html";

              QHelpEngineWrapper& helpEngine = HelpPluginActivator::getInstance()->getQHelpEngine();
              // Get view help page if available
              QUrl contextUrl(loc.arg(viewID.replace(".", "_")));
              QUrl url = helpEngine.findFile(contextUrl);
              if (url.isValid()) return url;
              else
              {
                BERRY_INFO << "Context help url invalid: " << contextUrl.toString().toStdString();
              }
              // If no view help exists get plugin help if available
              QUrl pluginContextUrl(loc.arg(pluginID.replace(".", "_")));
              url = helpEngine.findFile(pluginContextUrl);
              if (url.isValid()) return url;
              // Try to get the index.html file of the plug-in contributing the
              // currently active part.
              QUrl pluginIndexUrl(loc.arg("index"));
              url = helpEngine.findFile(pluginIndexUrl);
              if (url != pluginIndexUrl)
              {
                // Use the default page instead of another index.html
                // (merged via the virtual folder property).
                url = QUrl();
              }
              return url;
            }
          }
        }
      }
      return QUrl();
    }

    ctkEvent ev;
  };
  // sync with GUI thread
  Display::GetDefault()->AsyncExec(new _runner(event));
}

}
