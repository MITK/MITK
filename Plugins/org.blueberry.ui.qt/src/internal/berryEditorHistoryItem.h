/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEDITORHISTORYITEM_H
#define BERRYEDITORHISTORYITEM_H

#include <berryObject.h>
#include <berrySmartPointer.h>

namespace berry {

struct IEditorInput;
struct IEditorDescriptor;
struct IMemento;
struct IStatus;

/**
 * An item in the editor history.
 */
class EditorHistoryItem : public Object
{

private:

  SmartPointer<IEditorInput> input;

  SmartPointer<IEditorDescriptor> descriptor;

  SmartPointer<IMemento> memento;

public:

  berryObjectMacro(EditorHistoryItem);

  /**
   * Constructs a new item.
   */
  EditorHistoryItem(const SmartPointer<IEditorInput>& input, const SmartPointer<IEditorDescriptor>& descriptor);

  /**
   * Constructs a new item from a memento.
   */
  EditorHistoryItem(const SmartPointer<IMemento>& memento);

  /**
   * Returns the editor descriptor.
   *
   * @return the editor descriptor.
   */
  SmartPointer<IEditorDescriptor> GetDescriptor() const;

  /**
   * Returns the editor input.
   *
   * @return the editor input.
   */
  SmartPointer<IEditorInput> GetInput() const;

  /**
   * Returns whether this item has been restored from the memento.
   */
  bool IsRestored() const;

  /**
   * Returns the name of this item, either from the input if restored,
   * otherwise from the memento.
   */
  QString GetName() const;

  /**
   * Returns the tooltip text of this item, either from the input if restored,
   * otherwise from the memento.
   */
  QString GetToolTipText() const;

  /**
   * Returns whether this item matches the given editor input.
   */
  bool Matches(const SmartPointer<IEditorInput>& input) const;

  /**
   * Returns the factory id of this item, either from the input if restored,
   * otherwise from the memento.
   * Returns <code>null</code> if there is no factory id.
   */
  QString GetFactoryId() const;

  /**
   * Restores the object state from the memento.
   */
  SmartPointer<const IStatus> RestoreState();

  /**
   * Returns whether this history item can be saved.
   */
  bool CanSave() const;

  /**
   * Saves the object state in the given memento.
   *
   * @param memento the memento to save the object state in
   */
  SmartPointer<const IStatus> SaveState(const SmartPointer<IMemento>& memento);

};

}

Q_DECLARE_METATYPE(berry::EditorHistoryItem::Pointer)

#endif // BERRYEDITORHISTORYITEM_H
