/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkBasePropertyView.h"

QmitkBasePropertyView::QmitkBasePropertyView(const mitk::BaseProperty *property, QWidget *parent)
  : QLabel(parent), PropertyView(property)
{
  PropertyChanged();
}

QmitkBasePropertyView::~QmitkBasePropertyView()
{
}

void QmitkBasePropertyView::PropertyChanged()
{
  if (m_Property)
    setText(m_Property->GetValueAsString().c_str());
}

void QmitkBasePropertyView::PropertyRemoved()
{
  m_Property = nullptr;
  setText("n/a");
}
