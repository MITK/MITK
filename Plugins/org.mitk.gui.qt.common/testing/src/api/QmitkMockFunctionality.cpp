/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
