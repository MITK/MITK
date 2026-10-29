/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkWorkbenchUtil.h"

#include <berryPlatformUI.h>
#include <berryIEditorRegistry.h>
#include <berryIViewRegistry.h>
#include <berryCoreException.h>

#include "mitkDataStorageEditorInput.h"
#include <mitkRenderingManager.h>
#include "mitkIRenderingManager.h"
#include "mitkRecentData.h"

#include <mitkProperties.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkCoreServices.h>
#include <mitkDataStorageReference.h>
#include <mitkIDataStorageService.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkIOMimeTypes.h>
#include <mitkUndoController.h>

#include <QmitkIOUtil.h>

#include <QMessageBox>
#include <QPushButton>
#include <QApplication>
#include <QDateTime>

#include <algorithm>

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

    static mitk::DataStorageReference GetActiveDataStorageReference()
    {
      mitk::CoreServicePointer<mitk::IDataStorageService> dsService(mitk::CoreServices::GetDataStorageService());

      if (!dsService)
      {
        QString msg = "IDataStorageService service not available. Unable to open files.";
        MITK_WARN << msg.toStdString();
        QMessageBox::warning(QApplication::activeWindow(), "Unable to open files", msg);
        return mitk::DataStorageReference();
      }

      // Get the active data storage reference (or the default one, if none is active)
      return dsService->GetActiveDataStorageReference();
    }

    enum class CurrentProjectChoice
    {
      Close,
      Add,
      Cancel
    };

    static DataStorage::SetOfObjects::ConstPointer GetProjectNodes(const mitk::DataStorage& dataStorage)
    {
      auto isNoHelperObject = NodePredicateNot::New(NodePredicateProperty::New("helper object", BoolProperty::New(true)));
      return dataStorage.GetSubset(isNoHelperObject);
    }

    /**
     * Asks whether to close the current project before scenes are loaded into a data storage
     * that already contains data other than helper objects. Without scenes or such data, the
     * files are added without asking.
     */
    static CurrentProjectChoice AskToCloseCurrentProject(const QStringList& fileNames, const mitk::DataStorage& dataStorage)
    {
      if (!std::ranges::any_of(fileNames, WorkbenchUtil::IsSceneFile))
        return CurrentProjectChoice::Add;

      if (GetProjectNodes(dataStorage)->empty())
        return CurrentProjectChoice::Add;

      QMessageBox messageBox(QApplication::activeWindow());
      messageBox.setIcon(QMessageBox::Question);
      messageBox.setWindowTitle("Open Project");
      messageBox.setText("Close the current project before opening another?");
      messageBox.setInformativeText("Unsaved changes will be lost if you close it.");
      auto* closeButton = messageBox.addButton(QMessageBox::Yes);
      auto* addButton = messageBox.addButton("No, add to current project", QMessageBox::NoRole);
      messageBox.addButton(QMessageBox::Cancel);
      messageBox.setDefaultButton(addButton);
      messageBox.exec();

      if (messageBox.clickedButton() == closeButton)
        return CurrentProjectChoice::Close;

      if (messageBox.clickedButton() == addButton)
        return CurrentProjectChoice::Add;

      return CurrentProjectChoice::Cancel;
    }

    /**
     * Removes the given nodes and all nodes derived from them, such as their helper objects.
     */
    static void RemoveNodesAndDerivations(mitk::DataStorage& dataStorage, const DataStorage::SetOfObjects& nodes)
    {
      auto nodesToRemove = DataStorage::SetOfObjects::New();

      const auto addOnce = [&nodesToRemove](const DataNode::Pointer& node) {
        if (std::ranges::find(*nodesToRemove, node) == nodesToRemove->end())
          nodesToRemove->push_back(node);
      };

      for (const auto& node : nodes)
      {
        addOnce(node);

        const auto derivations = dataStorage.GetDerivations(node, nullptr, false);

        for (const auto& derivation : *derivations)
          addOnce(derivation);
      }

      dataStorage.Remove(nodesToRemove);
    }

  }; // end struct WorkbenchUtilPrivate

  //! [UtilLoadFiles]
  QStringList WorkbenchUtil::LoadFiles(const QStringList &fileNames, berry::IWorkbenchWindow::Pointer window, bool openEditor)
  //! [UtilLoadFiles]
  {
    if (fileNames.empty())
    {
      return {};
    }

    mitk::DataStorageReference dsRef = WorkbenchUtilPrivate::GetActiveDataStorageReference();
    if (!dsRef.IsValid())
    {
      return {};
    }
    mitk::DataStorage::Pointer dataStorage = dsRef.GetStorage();

    const auto choice = WorkbenchUtilPrivate::AskToCloseCurrentProject(fileNames, *dataStorage);

    if (choice == WorkbenchUtilPrivate::CurrentProjectChoice::Cancel)
    {
      return {};
    }

    // Taken before loading, so that the loaded files are not closed with the current project.
    DataStorage::SetOfObjects::ConstPointer projectToClose;

    if (choice == WorkbenchUtilPrivate::CurrentProjectChoice::Close)
    {
      projectToClose = WorkbenchUtilPrivate::GetProjectNodes(*dataStorage);
    }

    // Turn off ASSERT
#if defined(_MSC_VER) && !defined(NDEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
    int lastCrtReportType = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

    if (fileNames.size() > 1)
    {
      auto dicomMimeType = mitk::IOMimeTypes::DICOM_MIMETYPE();
      bool isDICOM = false;
      //Save guard for avoiding unintended DICOM stack loading
      for (const auto& file : fileNames)
      {
        isDICOM = dicomMimeType.AppliesTo(file.toStdString());
        if (isDICOM)
          break;
      }
      if (isDICOM)
      {
        if (QMessageBox::Cancel == QMessageBox::warning(
          QApplication::activeWindow(),
          "Confirm multiple DICOM series loading",
          "<p>You have selected multiple DICOM files to load.</p>"
          "<p>Please note that you only need to select a single DICOM file. MITK will automatically load all files "
          "that belong to the same DICOM series.</p>"
          "<p>Select multiple DICOM files only if they belong to different series. Otherwise, the <b>same volume will "
          "be loaded multiple times</b>.</p>"
          "<p>Hint: To load all DICOM series in a folder, simply select the folder. MITK will handle the rest for you.</p>"
          "<p>Do you want to continue?</p>", QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel))
          return {};
      }
    }

    std::vector<IOUtil::LoadInfo> loadInfos;

    for (const auto& fileName : fileNames)
      loadInfos.push_back(IOUtil::LoadInfo(fileName.toLocal8Bit().constData()));

    // Do the actual work of loading the data into the data storage
    DataStorage::SetOfObjects::Pointer data;
    try
    {
      data = QmitkIOUtil::Load(loadInfos, *dataStorage);
    }
    catch (const mitk::Exception& e)
    {
      MITK_INFO << e;
      return {};
    }
    const bool dsmodified = !data->empty();

    QStringList loadedFileNames;
    bool sceneLoaded = false;

    for (int i = 0; i < fileNames.size(); ++i)
    {
      if (!loadInfos[i].m_Output.empty())
      {
        loadedFileNames.append(fileNames[i]);
        sceneLoaded = sceneLoaded || IsSceneFile(fileNames[i]);
      }
    }

    // Other files may have loaded although the scene failed, so dsmodified is not enough.
    if (sceneLoaded && projectToClose.IsNotNull())
    {
      WorkbenchUtilPrivate::RemoveNodesAndDerivations(*dataStorage, *projectToClose);

      // Explicitly trigger undo model again. It is also done in the data storage itself, but
      // as long as projectToClose exists, it has no effect, as the smart pointer keeps the
      // removed nodes alive.
      projectToClose = nullptr;

      if (auto undoModel = mitk::UndoController::GetCurrentUndoModel(); nullptr != undoModel)
      {
        undoModel->RemoveInvalidOperations();
      }
    }

    RecentData::Add(loadedFileNames);

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
    auto* prefService = mitk::CoreServices::GetPreferencesService();
    if (prefService != nullptr)
    {
      auto* prefs = prefService->GetSystemPreferences()->Node("org.mitk.views.datamanager");

      if (prefs != nullptr)
        globalReinitOnNodeAdded = prefs->GetBool("Call global reinit if node is added", true);
    }

    if (openEditor && globalReinitOnNodeAdded)
    {
      try
      {
        // Activate the editor using the same data storage or open the default editor
        mitk::DataStorageEditorInput::Pointer input(new mitk::DataStorageEditorInput(dsRef));
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

    return loadedFileNames;
  }

  bool WorkbenchUtil::IsSceneFile(const QString& fileName)
  {
    return fileName.endsWith(".mitk", Qt::CaseInsensitive) || fileName.endsWith(".mitkscene.json", Qt::CaseInsensitive);
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

    mitk::DataStorageReference dsRef = WorkbenchUtilPrivate::GetActiveDataStorageReference();
    if (!dsRef.IsValid())
    {
      return nullptr;
    }

    mitk::DataStorageEditorInput::Pointer input(new mitk::DataStorageEditorInput(dsRef));

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
      auto* prefService = mitk::CoreServices::GetPreferencesService();

      if (prefService != nullptr)
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

  bool WorkbenchUtil::IsViewAvailable(const QString& viewId)
  {
    if (!berry::PlatformUI::IsWorkbenchRunning())
      return false;

    return berry::PlatformUI::GetWorkbench()->GetViewRegistry()->Find(viewId).IsNotNull();
  }

  QStringList WorkbenchUtil::FilterAvailableViewIds(const QStringList& viewIds)
  {
    QStringList availableViewIds;

    for (const auto& viewId : viewIds)
    {
      if (IsViewAvailable(viewId))
        availableViewIds.append(viewId);
    }

    return availableViewIds;
  }

} // namespace mitk
