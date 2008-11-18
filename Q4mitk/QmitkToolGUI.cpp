/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2008-04-24 18:31:42 +0200 (Do, 24 Apr 2008) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkToolGUI.h"

#include <iostream>

QmitkToolGUI::QmitkToolGUI()
:QWidget()
{
}

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

