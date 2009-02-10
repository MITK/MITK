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

#ifndef MITKDATASTORAGEEDITORINPUT_H_
#define MITKDATASTORAGEEDITORINPUT_H_

#include <cherryIEditorInput.h>
#include <mitkIDataStorageReference.h>

#include "mitkGUICommonPluginDll.h"

namespace mitk
{

class MITK_GUI_COMMON_PLUGIN DataStorageEditorInput : public cherry::IEditorInput
{
public:
  cherryObjectMacro(DataStorageEditorInput);

  bool Exists() const;
  std::string GetName() const;
  std::string GetToolTipText() const;

  IDataStorageReference::Pointer GetDataStorageReference();

  bool operator==(const cherry::Object*) const;

private:

  IDataStorageReference::Pointer m_DataStorageRef;
};

}

#endif /*MITKDATASTORAGEEDITORINPUT_H_*/
