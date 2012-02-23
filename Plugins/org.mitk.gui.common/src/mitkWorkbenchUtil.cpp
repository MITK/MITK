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


#include "mitkWorkbenchUtil.h"

#include <berryPlatformUI.h>
#include <berryIEditorRegistry.h>
#include <berryUIException.h>

#include <mitkDataNodeFactory.h>
#include "mitkIDataStorageService.h"
#include "mitkDataStorageEditorInput.h"
#include "mitkRenderingManager.h"
#include "mitkIRenderWindowPart.h"
#include "mitkIRenderingManager.h"

#include "mitkProperties.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkIOUtil.h"
#include "mitkWorkbenchUtil.h"

#include <ctkUtils.h>

#include <QMessageBox>
#include <QApplication>

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
  static berry::IEditorDescriptor::Pointer GetEditorDescriptor(const QString& name,
                                                               berry::IEditorRegistry* editorReg,
                                                               berry::IEditorDescriptor::Pointer defaultDescriptor)
  {

    if (defaultDescriptor.IsNotNull())
    {
      return defaultDescriptor;
    }

    berry::IEditorDescriptor::Pointer editorDesc = defaultDescriptor;

    // next check the OS for in-place editor (OLE on Win32)
    if (editorReg->IsSystemInPlaceEditorAvailable(name.toStdString()))
    {
      editorDesc = editorReg->FindEditor(berry::IEditorRegistry::SYSTEM_INPLACE_EDITOR_ID);
    }

    // next check with the OS for an external editor
    if (editorDesc.IsNull() && editorReg->IsSystemExternalEditorAvailable(name.toStdString()))
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
};

void WorkbenchUtil::LoadFiles(const QStringList &fileNames, berry::IWorkbenchWindow::Pointer window)
{
  if (fileNames.empty())
     return;

  mitk::IDataStorageReference::Pointer dataStorageRef;

  {
    ctkPluginContext* context = mitk::PluginActivator::GetContext();
    mitk::IDataStorageService* dss = 0;
    ctkServiceReference dsRef = context->getServiceReference<mitk::IDataStorageService>();
    if (dsRef)
    {
      dss = context->getService<mitk::IDataStorageService>(dsRef);
    }

    if (!dss)
    {
      QString msg = "IDataStorageService service not available. Unable to open files.";
      MITK_WARN << msg.toStdString();
      QMessageBox::warning(QApplication::activeWindow(), "Unable to open files", msg);
      return;
    }

    // Get the active data storage (or the default one, if none is active)
    dataStorageRef = dss->GetDataStorage();
    context->ungetService(dsRef);
  }

  mitk::DataStorage::Pointer dataStorage = dataStorageRef->GetDataStorage();

  // Do the actual work of loading the data into the data storage
  std::vector<std::string> fileNames2;
  ctk::qListToSTLVector(fileNames, fileNames2);
  const bool dsmodified = mitk::IOUtil::LoadFiles(fileNames2, *dataStorage);

  // Check if there is an open perspective. If not, open the default perspective.
  if (window->GetActivePage().IsNull())
  {
    std::string defaultPerspId = window->GetWorkbench()->GetPerspectiveRegistry()->GetDefaultPerspective();
    window->GetWorkbench()->ShowPerspective(defaultPerspId, window);
  }

  try
  {
    // Activate the editor using the same data storage or open the default editor
    mitk::DataStorageEditorInput::Pointer input(new mitk::DataStorageEditorInput(dataStorageRef));
    berry::IEditorPart::Pointer editor = mitk::WorkbenchUtil::OpenEditor(window->GetActivePage(), input, true);
    mitk::IRenderWindowPart* renderEditor = dynamic_cast<mitk::IRenderWindowPart*>(editor.GetPointer());
    mitk::IRenderingManager* renderingManager = renderEditor == 0 ? 0 : renderEditor->GetRenderingManager();

    if(dsmodified && renderingManager)
    {
      // get all nodes that have not set "includeInBoundingBox" to false
      mitk::NodePredicateNot::Pointer pred
          = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
                                                                         , mitk::BoolProperty::New(false)));

      mitk::DataStorage::SetOfObjects::ConstPointer rs = dataStorage->GetSubset(pred);
      // calculate bounding geometry of these nodes
      mitk::TimeSlicedGeometry::Pointer bounds = dataStorage->ComputeBoundingGeometry3D(rs);
      // initialize the views to the bounding geometry
      renderingManager->InitializeViews(bounds);
    }
  }
  catch (const berry::PartInitException& e)
  {
    QString msg = "An error occurred when displaying the file(s): %1";
    QMessageBox::warning(QApplication::activeWindow(), "Error displaying file",
                         msg.arg(QString::fromStdString(e.message())));
  }
}

berry::IEditorPart::Pointer WorkbenchUtil::OpenEditor(berry::IWorkbenchPage::Pointer page,
                                                      berry::IEditorInput::Pointer input,
                                                      const QString &editorId, bool activate)
{
  // sanity checks
  if (page.IsNull())
  {
    throw std::invalid_argument("page argument must not be NULL");
  }

  // open the editor on the input
  return page->OpenEditor(input, editorId.toStdString(), activate);
}

berry::IEditorPart::Pointer WorkbenchUtil::OpenEditor(berry::IWorkbenchPage::Pointer page,
                                                      mitk::DataStorageEditorInput::Pointer input,
                                                      bool activate,
                                                      bool determineContentType)
{
  // sanity checks
  if (page.IsNull())
  {
    throw std::invalid_argument("page argument must not be NULL");
  }

  // open the editor on the data storage
  QString name = QString::fromStdString(input->GetName()) + ".mitk";
  berry::IEditorDescriptor::Pointer editorDesc =
      WorkbenchUtilPrivate::GetEditorDescriptor(name,
                                                berry::PlatformUI::GetWorkbench()->GetEditorRegistry(),
                                                GetDefaultEditor(name, determineContentType));
  return page->OpenEditor(input, editorDesc->GetId(), activate);
}

berry::IEditorDescriptor::Pointer WorkbenchUtil::GetEditorDescriptor(
    const QString& name, bool /*inferContentType*/)
{
  if (name.isEmpty())
  {
    throw std::invalid_argument("name argument must not be empty");
  }

  // no used for now
  //IContentType contentType = inferContentType ? Platform
  //                                              .getContentTypeManager().findContentTypeFor(name) : null;

  berry::IEditorRegistry* editorReg = berry::PlatformUI::GetWorkbench()->GetEditorRegistry();

  return WorkbenchUtilPrivate::GetEditorDescriptor(name, editorReg,
                                                   editorReg->GetDefaultEditor(name.toStdString() /*, contentType*/));
}

berry::IEditorDescriptor::Pointer WorkbenchUtil::GetDefaultEditor(const QString& name,
                                                                  bool /*determineContentType*/)
{
  // Try file specific editor.
  berry::IEditorRegistry* editorReg = berry::PlatformUI::GetWorkbench()->GetEditorRegistry();
  try
  {
    QString editorID; // = file.getPersistentProperty(EDITOR_KEY);
    if (!editorID.isEmpty())
    {
      berry::IEditorDescriptor::Pointer desc = editorReg->FindEditor(editorID.toStdString());
      if (desc.IsNotNull())
      {
        return desc;
      }
    }
  }
  catch (const berry::CoreException& e)
  {
    // do nothing
  }

//  IContentType contentType = null;
//  if (determineContentType)
//  {
//    contentType = getContentType(file);
//  }

  // Try lookup with filename
  return editorReg->GetDefaultEditor(name.toStdString()); //, contentType);
}

} // namespace mitk
