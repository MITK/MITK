/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryNewEditorHandler.h"

#include <berryHandlerUtil.h>
#include <berryXMLMemento.h>

#include "internal/berryWorkbenchConstants.h"
#include "internal/berryWorkbenchPage.h"

#include <berryIPersistableEditor.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

#include <QApplication>
#include <QMessageBox>

namespace berry {

Object::Pointer NewEditorHandler::Execute(const SmartPointer<const ExecutionEvent> &event)
{
  IWorkbenchWindow::Pointer activeWorkbenchWindow = HandlerUtil::GetActiveWorkbenchWindow(event);
  IWorkbenchPage::Pointer page = activeWorkbenchWindow->GetActivePage();
  if (page.IsNull())
  {
    return Object::Pointer();
  }
  IEditorPart::Pointer editor = page->GetActiveEditor();
  if (editor.IsNull())
  {
    return Object::Pointer();
  }
  QString editorId = editor->GetSite()->GetId();
  if (editorId.isEmpty())
  {
    return Object::Pointer();
  }
  try
  {
    if (IPersistableEditor* persistableEditor = dynamic_cast<IPersistableEditor*>(editor.GetPointer()))
    {
      IMemento::Pointer editorState(XMLMemento::CreateWriteRoot(WorkbenchConstants::TAG_EDITOR_STATE).GetPointer());
      persistableEditor->SaveState(editorState);
      page.Cast<WorkbenchPage>()->OpenEditor(editor->GetEditorInput(),
                                             editorId, true,
                                             IWorkbenchPage::MATCH_NONE, editorState);
    }
    else
    {
      page->OpenEditor(editor->GetEditorInput(), editorId, true,
                       IWorkbenchPage::MATCH_NONE);
    }
  }
  catch (const PartInitException& e)
  {
    QMessageBox::critical(QApplication::activeWindow(), "Error", e.message());
  }
  return Object::Pointer();
}

}
