/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKDATASTORAGEEDITORINPUT_H_
#define MITKDATASTORAGEEDITORINPUT_H_

#include <berryIEditorInput.h>
#include <berryIPersistableElement.h>

#include <mitkIDataStorageReference.h>

#include <org_mitk_gui_common_Export.h>

namespace mitk
{

/**
 * \ingroup org_mitk_gui_common
 *
 * \brief An editor input based on a mitk::DataStorage.
 *
 * This editor input is usually used in render window editors inheriting from
 * QmitkAbstractRenderEditor.
 */
class MITK_GUI_COMMON_PLUGIN DataStorageEditorInput : public berry::IEditorInput //, private berry::IPersistableElement
{
public:
  berryObjectMacro(DataStorageEditorInput);

  DataStorageEditorInput();
  DataStorageEditorInput(IDataStorageReference::Pointer ref);

  bool Exists() const override;
  QString GetName() const override;
  QString GetToolTipText() const override;
  QIcon GetIcon() const override;

  const berry::IPersistableElement* GetPersistable() const override;
  Object* GetAdapter(const QString &adapterType) const override;

  IDataStorageReference::Pointer GetDataStorageReference();

  bool operator==(const berry::Object*) const override;

private:

  //QString GetFactoryId() const;
  //void SaveState(const berry::SmartPointer<berry::IMemento>& memento) const;

  IDataStorageReference::Pointer m_DataStorageRef;
};

}

#endif /*MITKDATASTORAGEEDITORINPUT_H_*/
