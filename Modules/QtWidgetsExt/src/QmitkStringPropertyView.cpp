/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkStringPropertyView.h"

QmitkStringPropertyView::QmitkStringPropertyView(const mitk::StringProperty *property, QWidget *parent)
  : QLabel(parent), PropertyView(property), m_StringProperty(property)
{
  PropertyChanged();
}

QmitkStringPropertyView::~QmitkStringPropertyView()
{
}

void QmitkStringPropertyView::PropertyChanged()
{
  if (m_Property)
    setText(m_StringProperty->GetValue());
}

void QmitkStringPropertyView::PropertyRemoved()
{
  m_Property = nullptr;
  m_StringProperty = nullptr;
  setText("n/a");
}
