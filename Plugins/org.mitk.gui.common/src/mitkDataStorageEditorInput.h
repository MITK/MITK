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

#ifndef MITKDATASTORAGEEDITORINPUT_H_
#define MITKDATASTORAGEEDITORINPUT_H_

#include <berryIEditorInput.h>
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
class MITK_GUI_COMMON_PLUGIN DataStorageEditorInput : public berry::IEditorInput
{
public:
  berryObjectMacro(DataStorageEditorInput);

  DataStorageEditorInput();
  DataStorageEditorInput(IDataStorageReference::Pointer ref);

  bool Exists() const;
  QString GetName() const;
  QString GetToolTipText() const;

  IDataStorageReference::Pointer GetDataStorageReference();

  bool operator==(const berry::Object*) const;

private:

  IDataStorageReference::Pointer m_DataStorageRef;
};

}

#endif /*MITKDATASTORAGEEDITORINPUT_H_*/
