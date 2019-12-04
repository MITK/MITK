/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "EmptyView2.h"

const std::string EmptyView2::VIEW_ID = "org.mitk.views.emptyview2";

EmptyView2::EmptyView2() : m_Parent(nullptr)
{
}

EmptyView2::~EmptyView2()
{
}

void EmptyView2::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  m_Parent->setEnabled(true);
}

void EmptyView2::SetFocus()
{
}
