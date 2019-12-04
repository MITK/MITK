/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEditorHistoryItem.h"

#include "berryIEditorInput.h"
#include "berryIEditorDescriptor.h"
#include "berryIElementFactory.h"
#include "berryIMemento.h"
#include "berryIPersistableElement.h"

#include "berryStatus.h"
#include "berryWorkbenchConstants.h"
#include "berryWorkbenchPlugin.h"

namespace berry {

EditorHistoryItem::EditorHistoryItem(const SmartPointer<IEditorInput>& input,
                                     const SmartPointer<IEditorDescriptor>& descriptor)
  : input(input)
  , descriptor(descriptor)
{
}

EditorHistoryItem::EditorHistoryItem(const SmartPointer<IMemento>& memento)
  : memento(memento)
{
}

SmartPointer<IEditorDescriptor> EditorHistoryItem::GetDescriptor() const
{
  return descriptor;
}

SmartPointer<IEditorInput> EditorHistoryItem::GetInput() const
{
  return input;
}

bool EditorHistoryItem::IsRestored() const
{
  return memento.IsNull();
}

QString EditorHistoryItem::GetName() const
{
  QString result;
  if (IsRestored() && GetInput().IsNotNull())
  {
    result = GetInput()->GetName();
  }
  else if (memento.IsNotNull())
  {
    memento->GetString(WorkbenchConstants::TAG_NAME, result);
  }
  return result;
}

QString EditorHistoryItem::GetToolTipText() const
{
  QString result;
  if (IsRestored() && GetInput().IsNotNull())
  {
    result = GetInput()->GetToolTipText();
  }
  else if (memento.IsNotNull())
  {
    memento->GetString(WorkbenchConstants::TAG_TOOLTIP, result);
  }
  return result;
}

bool EditorHistoryItem::Matches(const SmartPointer<IEditorInput>& input) const
{
  if (IsRestored())
  {
    return input == GetInput();
  }
  // if not restored, compare name, tool tip text and factory id,
  // avoiding as much work as possible
  if (GetName() != input->GetName())
  {
    return false;
  }
  if (GetToolTipText() != input->GetToolTipText())
  {
    return false;
  }

  const IPersistableElement* persistable = input->GetPersistable();
  QString inputId = persistable ? QString::null : persistable->GetFactoryId();
  QString myId = GetFactoryId();
  return myId.isEmpty() ? inputId.isEmpty() : myId == inputId;
}

QString EditorHistoryItem::GetFactoryId() const
{
  QString result;
  if (IsRestored())
  {
    if (input.IsNotNull())
    {
      const IPersistableElement* persistable = input->GetPersistable();
      if (persistable != nullptr)
      {
        result = persistable->GetFactoryId();
      }
    }
  }
  else if (memento.IsNotNull())
  {
    memento->GetString(WorkbenchConstants::TAG_FACTORY_ID, result);
  }
  return result;
}

SmartPointer<const IStatus> EditorHistoryItem::RestoreState()
{
  Q_ASSERT_X(!IsRestored(), "RestoreState", "already restored");

  IStatus::ConstPointer result = Status::OK_STATUS(BERRY_STATUS_LOC);
  IMemento::Pointer memento = this->memento;
  this->memento = nullptr;

  QString factoryId;
  memento->GetString(WorkbenchConstants::TAG_FACTORY_ID, factoryId);
  if (factoryId.isEmpty())
  {
    WorkbenchPlugin::Log("Unable to restore mru list - no input factory ID.");
    return result;
  }
  QScopedPointer<IElementFactory> factory(
        PlatformUI::GetWorkbench()->GetElementFactory(factoryId));
  if (!factory)
  {
    return result;
  }
  IMemento::Pointer persistableMemento = memento->GetChild(WorkbenchConstants::TAG_PERSISTABLE);
  if (persistableMemento.IsNull())
  {
    WorkbenchPlugin::Log("Unable to restore mru list - no input element state: " + factoryId);
    return result;
  }
  QScopedPointer<IAdaptable> adaptable(factory->CreateElement(persistableMemento));
  if (adaptable == nullptr || dynamic_cast<IEditorInput*>(adaptable.data()) == nullptr)
  {
    return result;
  }
  input = dynamic_cast<IEditorInput*>(adaptable.data());
  // Get the editor descriptor.
  QString editorId;
  memento->GetString(WorkbenchConstants::TAG_ID, editorId);
  if (!editorId.isEmpty())
  {
    IEditorRegistry* registry = WorkbenchPlugin::GetDefault()->GetEditorRegistry();
    descriptor = registry->FindEditor(editorId);
  }
  return result;
}

bool EditorHistoryItem::CanSave() const
{
  return !IsRestored()
      || (GetInput().IsNotNull() && GetInput()->GetPersistable() != nullptr);
}

SmartPointer<const IStatus> EditorHistoryItem::SaveState(const SmartPointer<IMemento>& memento)
{
  if (!IsRestored())
  {
    memento->PutMemento(this->memento);
  }
  else if (input.IsNotNull())
  {

    const IPersistableElement* persistable = input->GetPersistable();
    if (persistable != nullptr)
    {
      /*
       * Store IPersistable of the IEditorInput in a separate section
       * since it could potentially use a tag already used in the parent
       * memento and thus overwrite data.
       */
      IMemento::Pointer persistableMemento = memento->CreateChild(WorkbenchConstants::TAG_PERSISTABLE);
      persistable->SaveState(persistableMemento);
      memento->PutString(WorkbenchConstants::TAG_FACTORY_ID,
                         persistable->GetFactoryId());
      if (descriptor.IsNotNull() && !descriptor->GetId().isEmpty())
      {
        memento->PutString(WorkbenchConstants::TAG_ID, descriptor->GetId());
      }
      // save the name and tooltip separately so they can be restored
      // without having to instantiate the input, which can activate plugins
      memento->PutString(WorkbenchConstants::TAG_NAME, input->GetName());
      memento->PutString(WorkbenchConstants::TAG_TOOLTIP, input->GetToolTipText());
    }
  }
  return Status::OK_STATUS(BERRY_STATUS_LOC);
}


}
