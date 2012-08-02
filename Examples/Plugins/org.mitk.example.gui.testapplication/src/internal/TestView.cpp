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

#include "TestView.h"

TestView::TestView()
{
}

//void TestView::CreateQtPartControl(QWidget *parent)
//{
//  // create GUI widgets
//  m_Parent = parent;
//  m_Controls.setupUi(parent);
//
//  connect(m_Controls.selectButton, SIGNAL(clicked()), this, SLOT(ToggleRadioMethod()));
//  
//  // register selection listener
//  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);
//
//  m_Parent->setEnabled(true);
//}