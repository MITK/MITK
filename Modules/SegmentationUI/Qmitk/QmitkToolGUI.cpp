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

#include "QmitkToolGUI.h"

#include <iostream>

QmitkToolGUI::~QmitkToolGUI()
{
  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0; // otherwise ITK will complain in LightObject's destructor
  m_ReferenceCountLock.Unlock();
}

void QmitkToolGUI::Register() const
{
  // empty on purpose, just don't let ITK handle calls to Register()
}

void QmitkToolGUI::UnRegister() const
{
  // empty on purpose, just don't let ITK handle calls to UnRegister()
}

void QmitkToolGUI::SetReferenceCount(int)
{
  // empty on purpose, just don't let ITK handle calls to SetReferenceCount()
}

void QmitkToolGUI::SetTool( mitk::Tool* tool )
{
  m_Tool = tool;

  emit( NewToolAssociated(tool) );
}

