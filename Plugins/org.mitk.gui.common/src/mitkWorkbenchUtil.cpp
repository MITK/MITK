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

#include "mitkWorkbenchUtil.h"

#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIEditorRegistry.h>
#include <berryCoreException.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>

#include "mitkIDataStorageService.h"
#include "mitkDataStorageEditorInput.h"
#include "mitkRenderingManager.h"
#include "mitkIRenderingManager.h"

#include "mitkProperties.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkCoreObjectFactory.h"

#include "QmitkIOUtil.h"

#include <QMessageBox>
#include <QApplication>
#include <QDateTime>

#include "internal/org_mitk_gui_common_Activator.h"

namespace mitk {

  struct WorkbenchUtilPrivate {

    /**
     * Get the editor descriptor for a given name using the editorDescriptor
     * passed in as a default as a starting point.
     *
     * @param name
     *            The name of the element to open.
     * @param editorReg
     *            The editor registry to do the lookups from.
     * @param defaultDescriptor
     *            IEditorDescriptor or <code>null</code>
     * @return IEditorDescriptor
     * @throws PartInitException
     *             if no valid editor can be found
     */
    static berry::IEditorDescriptor::Pointer GetEditorDescriptor(const QString& name, berry::IEditorRegistry* editorReg, berry::IEditorDescriptor::Pointer defaultDescriptor)
    {
      if (defaultDescriptor.IsNotNull())
      {
        return defaultDescriptor;
      }

      berry::IEditorDescriptor::Pointer editorDesc = defaultDescriptor;

      // next check the OS for in-place editor (OLE on Win32)
      if (editorReg->IsSystemInPlaceEditorAvailable(name))
      {
        editorDesc = editorReg->FindEditor(berry::IEditorRegistry::SYSTEM_INPLACE_EDITOR_ID);
      }

      // next check with the OS for an external editor
      if (editorDesc.IsNull() && editorReg->IsSystemExternalEditorAvailable(name))
      {
        editorDesc = editorReg->FindEditor(berry::IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID);
      }

      // if no valid editor found, bail out
      if (editorDesc.IsNull())
      {
        throw berry::PartInitException("No editor found");
      }

      return editorDesc;
    }

    static mitk::IDataStorageReference::Pointer GetDataStorageReference()
    {
      ctkPluginContext* context = mitk::PluginActivator::GetContext();
      mitk::IDataStorageService* dss = nullptr;
      ctkServiceReference dsRef = context->getServiceReference<mitk::IDataStorageService>();
      if (dsRef)
      {
        dss = context->getService<mitk::IDataStorageService>(dsRef);
      }

      if (nullptr == dss)
      {
        QString msg = "IDataStorageService service not available. Unable to open files.";
        MITK_WARN << msg.toStdString();
        QMessageBox::warning(QApplication::activeWindow(), "Unable to open files", msg);
        return mitk::IDataStorageReference::Pointer(nullptr);
      }

      // Get the active data storage (or the default one, if none is active)
      mitk::IDataStorageReference::Pointer dataStorageRef = dss->GetDataStorage();
      context->ungetService(dsRef);

      return dataStorageRef;
    }

  }; // end struct WorkbenchUtilPrivate

  void WorkbenchUtil::LoadFiles(const QStringList &fileNames, berry::IWorkbenchWindow::Pointer window, bool openEditor)
  {
    if (fileNames.empty())
    {
      return;
    }

    mitk::IDataStorageReference::Pointer dataStorageReference = WorkbenchUtilPrivate::GetDataStorageReference();
    if (nullptr == dataStorageReference)
    {
      return;
    }
    mitk::DataStorage::Pointer dataStorage = dataStorageReference->GetDataStorage();

    // Turn off ASSERT
#if defined(_MSC_VER) && !defined(NDEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
    int lastCrtReportType = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

    // Do the actual work of loading the data into the data storage
    DataStorage::SetOfObjects::Pointer data;
    try
    {
      data = QmitkIOUtil::Load(fileNames, *dataStorage);
    }
    catch (const mitk::Exception& e)
    {
      MITK_INFO << e;
      return;
    }
    const bool dsmodified = !data->empty();

    // Set ASSERT status back to previous status.
#if defined(_MSC_VER) && !defined(NDEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
    if (lastCrtReportType)
      _CrtSetReportMode(_CRT_ASSERT, lastCrtReportType);
#endif

    // Check if there is an open perspective. If not, open the default perspective.
    if (window->GetActivePage().IsNull())
    {
      QString defaultPerspId = window->GetWorkbench()->GetPerspectiveRegistry()->GetDefaultPerspective();
      window->GetWorkbench()->ShowPerspective(defaultPerspId, window);
    }

    bool globalReinitOnNodeAdded = true;
    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
    if (prefService != nullptr)
    {
      berry::IPreferences::Pointer prefs
        = prefService->GetSystemPreferences()->Node("org.mitk.views.datamanager");
      if (prefs.IsNotNull())
      {
        globalReinitOnNodeAdded = prefs->GetBool("Call global reinit if node is added", true);
      }
    }

    if (openEditor && globalReinitOnNodeAdded)
    {
      try
      {
        // Activate the editor using the same data storage or open the default editor
        mitk::DataStorageEditorInput::Pointer input(new mitk::DataStorageEditorInput(dataStorageReference));
        berry::IEditorPart::Pointer editor = mitk::WorkbenchUtil::OpenEditor(window->GetActivePage(), input, true);
        mitk::IRenderWindowPart* renderEditor = dynamic_cast<mitk::IRenderWindowPart*>(editor.GetPointer());
        mitk::IRenderingManager* renderingManager = renderEditor == nullptr ? nullptr : renderEditor->GetRenderingManager();

        if (dsmodified && renderingManager)
        {
          mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
        }
      }
      catch (const berry::PartInitException& e)
      {
        QString msg = "An error occurred when displaying the file(s): %1";
        QMessageBox::warning(QApplication::activeWindow(), "Error displaying file",
          msg.arg(e.message()));
      }
    }
  }

  berry::IEditorPart::Pointer WorkbenchUtil::OpenEditor(berry::IWorkbenchPage::Pointer page, berry::IEditorInput::Pointer input, const QString &editorId, bool activate)
  {
    // sanity checks
    if (page.IsNull())
    {
      throw std::invalid_argument("page argument must not be nullptr");
    }

    // open the editor on the input
    return page->OpenEditor(input, editorId, activate);
  }

  berry::IEditorPart::Pointer WorkbenchUtil::OpenEditor(berry::IWorkbenchPage::Pointer page, mitk::DataStorageEditorInput::Pointer input, bool activate, bool determineContentType)
  {
    // sanity checks
    if (page.IsNull())
    {
      throw std::invalid_argument("page argument must not be nullptr");
    }

    // open the editor on the data storage
    QString name = input->GetName() + ".mitk";
    berry::IEditorDescriptor::Pointer editorDesc = WorkbenchUtilPrivate::GetEditorDescriptor(name,
        berry::PlatformUI::GetWorkbench()->GetEditorRegistry(),
        GetDefaultEditor(name, determineContentType));

    return page->OpenEditor(input, editorDesc->GetId(), activate);
  }

  berry::IEditorDescriptor::Pointer WorkbenchUtil::GetEditorDescriptor(const QString& name, bool /*inferContentType*/)
  {
    if (name.isEmpty())
    {
      throw std::invalid_argument("name argument must not be empty");
    }

    // no used for now
    //IContentType contentType = inferContentType ? Platform
    //                                              .getContentTypeManager().findContentTypeFor(name) : null;

    berry::IEditorRegistry* editorReg = berry::PlatformUI::GetWorkbench()->GetEditorRegistry();

    return WorkbenchUtilPrivate::GetEditorDescriptor(name, editorReg, editorReg->GetDefaultEditor(name /*, contentType*/));
  }

  berry::IEditorDescriptor::Pointer WorkbenchUtil::GetDefaultEditor(const QString& name, bool /*determineContentType*/)
  {
    // Try file specific editor.
    berry::IEditorRegistry* editorReg = berry::PlatformUI::GetWorkbench()->GetEditorRegistry();
    try
    {
      QString editorID; // = file.getPersistentProperty(EDITOR_KEY);
      if (!editorID.isEmpty())
      {
        berry::IEditorDescriptor::Pointer desc = editorReg->FindEditor(editorID);
        if (desc.IsNotNull())
        {
          return desc;
        }
      }
    }
    catch (const berry::CoreException&)
    {
      // do nothing
    }

    //  IContentType contentType = null;
    //  if (determineContentType)
    //  {
    //    contentType = getContentType(file);
    //  }

      // Try lookup with filename
    return editorReg->GetDefaultEditor(name); //, contentType);
  }

  mitk::IRenderWindowPart* WorkbenchUtil::GetRenderWindowPart(berry::IWorkbenchPage::Pointer page, IRenderWindowPartStrategies strategies)
  {
    // Return the active editor if it implements mitk::IRenderWindowPart
    mitk::IRenderWindowPart* renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(page->GetActiveEditor().GetPointer());
    if (renderWindowPart)
    {
      return renderWindowPart;
    }

    // No suitable active editor found, check visible editors
    QList<berry::IEditorReference::Pointer> editors = page->GetEditorReferences();
    for (QList<berry::IEditorReference::Pointer>::iterator i = editors.begin(); i != editors.end(); ++i)
    {
      berry::IWorkbenchPart::Pointer part = (*i)->GetPart(false);
      if (page->IsPartVisible(part))
      {
        renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(part.GetPointer());
        if (renderWindowPart)
        {
          return renderWindowPart;
        }
      }
    }

    // No suitable visible editor found, check visible views
    QList<berry::IViewReference::Pointer> views = page->GetViewReferences();
    for (QList<berry::IViewReference::Pointer>::iterator i = views.begin(); i != views.end(); ++i)
    {
      berry::IWorkbenchPart::Pointer part = (*i)->GetPart(false);
      if (page->IsPartVisible(part))
      {
        renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(part.GetPointer());
        if (renderWindowPart)
        {
          return renderWindowPart;
        }
      }
    }

    // No strategies given
    if (strategies == NONE)
    {
      return nullptr;
    }

    mitk::IDataStorageReference::Pointer dataStorageReference = WorkbenchUtilPrivate::GetDataStorageReference();
    if (nullptr == dataStorageReference)
    {
      return nullptr;
    }

    mitk::DataStorageEditorInput::Pointer input(new mitk::DataStorageEditorInput(dataStorageReference));

    bool activate = false;
    if (strategies & ACTIVATE)
    {
      activate = true;
    }

    berry::IEditorPart::Pointer editorPart;
    if (strategies & OPEN)
    {
      // This will create a default editor for the given input. If an editor
      // with that input is already open, the editor is brought to the front.
      try
      {
        editorPart = mitk::WorkbenchUtil::OpenEditor(page, input, activate);
      }
      catch (const berry::PartInitException&)
      {
        // There is no editor registered which can handle the given input.
      }
    }
    else if (activate || (strategies & BRING_TO_FRONT))
    {
      // check if a suitable editor is already opened
      editorPart = page->FindEditor(input);
      if (editorPart)
      {
        if (activate)
        {
          page->Activate(editorPart);
        }
        else
        {
          page->BringToTop(editorPart);
        }
      }
    }

    return dynamic_cast<mitk::IRenderWindowPart*>(editorPart.GetPointer());
  }

  mitk::IRenderWindowPart* WorkbenchUtil::OpenRenderWindowPart(berry::IWorkbenchPage::Pointer page, bool activatedEditor/* = true*/)
  {
    if (activatedEditor)
    {
      return GetRenderWindowPart(page, ACTIVATE | OPEN);
    }
    else
    {
      return GetRenderWindowPart(page, BRING_TO_FRONT | OPEN);
    }
  }

  bool WorkbenchUtil::SetDepartmentLogoPreference(const QString &logoResource, ctkPluginContext *context)
  {
    if (context == nullptr)
    {
      BERRY_WARN << "Plugin context invalid, unable to set custom logo.";
      return false;
    }

    // The logo must be available in the local filesystem. We check if we have not already extracted the
    // logo from the plug-in or if this plug-ins timestamp is newer then the already extracted logo timestamp.
    // If one of the conditions is true, extract it and write it to the plug-in specific storage location.
    const QString logoFileName = logoResource.mid(logoResource.lastIndexOf('/') + 1);

    if (logoFileName.isEmpty())
    {
      BERRY_WARN << "Logo file name empty, unable to set custom logo.";
      return false;
    }

    const QString logoPath = context->getDataFile("").absoluteFilePath();

    bool extractLogo = true;
    QFileInfo logoFileInfo(logoPath + "/" + logoFileName);

    if (logoFileInfo.exists())
    {
      // The logo has been extracted previously. Check if the plugin timestamp is newer, which
      // means it might contain an updated logo.
      QString pluginLocation = QUrl(context->getPlugin()->getLocation()).toLocalFile();
      if (!pluginLocation.isEmpty())
      {
        QFileInfo pluginFileInfo(pluginLocation);
        if (logoFileInfo.lastModified() > pluginFileInfo.lastModified())
        {
          extractLogo = false;
        }
      }
    }

    if (extractLogo)
    {
      // Extract the logo from the shared library and write it to disk.
      QFile logo(logoResource);

      if (!logo.exists())
      {
        BERRY_WARN << "Custom logo '" << logoResource << "' does not exist.";
        return false;
      }

      if (logo.open(QIODevice::ReadOnly))
      {
        QFile localLogo(logoPath + "/" + logoFileName);

        if (localLogo.open(QIODevice::WriteOnly))
        {
          localLogo.write(logo.readAll());
          localLogo.flush();
        }
      }
    }

    logoFileInfo.refresh();

    if (logoFileInfo.exists())
    {
      // Get the preferences service
      ctkServiceReference prefServiceRef = context->getServiceReference<berry::IPreferencesService>();
      berry::IPreferencesService* prefService = nullptr;
      if (prefServiceRef)
      {
        prefService = context->getService<berry::IPreferencesService>(prefServiceRef);
      }

      if (prefService)
      {
        prefService->GetSystemPreferences()->Put("DepartmentLogo", qPrintable(logoFileInfo.absoluteFilePath()));
      }
      else
      {
        BERRY_WARN << "Preferences service not available, unable to set custom logo.";
        return false;
      }
    }
    else
    {
      BERRY_WARN << "Custom logo at '" << logoFileInfo.absoluteFilePath().toStdString() << "' does not exist.";
      return false;
    }

    return true;
  }

} // namespace mitk
