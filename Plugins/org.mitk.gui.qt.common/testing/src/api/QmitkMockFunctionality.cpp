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

#include "QmitkMockFunctionality.h"

const std::string QmitkMockFunctionality::ID = "org.mitk.gui.qt.common.tests.api.MockFunctionality";

QmitkMockFunctionality::QmitkMockFunctionality()
: callTrace(new berry::CallHistory())
{

}

QmitkMockFunctionality::QmitkMockFunctionality(const QmitkMockFunctionality& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
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
