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

#include "EmptyView2.h"

const std::string EmptyView2::VIEW_ID = "org.mitk.views.emptyview2";

EmptyView2::EmptyView2() : m_Parent(0)
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

void EmptyView2::SetFocus ()
{
}
