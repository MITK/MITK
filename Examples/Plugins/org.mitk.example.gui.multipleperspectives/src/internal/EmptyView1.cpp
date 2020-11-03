/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "EmptyView1.h"

const std::string EmptyView1::VIEW_ID = "org.mitk.views.emptyview1";

EmptyView1::EmptyView1() : m_Parent(nullptr)
{
}

EmptyView1::~EmptyView1()
{
}

void EmptyView1::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  m_Parent->setEnabled(true);
}

void EmptyView1::SetFocus()
{
}
