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

#include "QmitkMockFunctionality.h"

const std::string QmitkMockFunctionality::ID = "org.mitk.gui.qt.common.tests.api.MockFunctionality";

QmitkMockFunctionality::QmitkMockFunctionality()
: callTrace(new berry::CallHistory())
{

}

QmitkMockFunctionality::~QmitkMockFunctionality()
{
  callTrace->Add("FunctionalityDestructor");
}

berry::CallHistory::Pointer QmitkMockFunctionality::GetCallHistory() const
{
  return callTrace;
}

void QmitkMockFunctionality::CreateQtPartControl(QWidget* parent)
{
  callTrace->Add("CreateQtPartControl");
}

void QmitkMockFunctionality::SetFocus()
{
  QmitkFunctionality::SetFocus();
  callTrace->Add("SetFocus");
}
