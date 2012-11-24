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

#include "EmptyView1.h"

const std::string EmptyView1::VIEW_ID = "org.mitk.views.emptyview1";

EmptyView1::EmptyView1() : m_Parent(0)
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

void EmptyView1::SetFocus ()
{
}