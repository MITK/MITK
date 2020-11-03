/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkToolGUI.h"

#include <iostream>

QmitkToolGUI::~QmitkToolGUI()
{
  m_ReferenceCount = 0; // otherwise ITK will complain in LightObject's destructor
}

void QmitkToolGUI::Register() const
{
  // empty on purpose, just don't let ITK handle calls to Register()
}

void QmitkToolGUI::UnRegister() const ITK_NOEXCEPT
{
  // empty on purpose, just don't let ITK handle calls to UnRegister()
}

void QmitkToolGUI::SetReferenceCount(int)
{
  // empty on purpose, just don't let ITK handle calls to SetReferenceCount()
}

void QmitkToolGUI::SetTool(mitk::Tool *tool)
{
  m_Tool = tool;

  emit(NewToolAssociated(tool));
}
