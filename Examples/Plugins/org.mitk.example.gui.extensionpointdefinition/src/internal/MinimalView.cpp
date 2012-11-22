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

#include "MinimalView.h"

#include "org_mitk_example_gui_extensionpointdefinition_Activator.h"

#include <QMessageBox>

const std::string MinimalView::VIEW_ID = "org.mitk.views.minimalview";

MinimalView::MinimalView() : m_Parent(0)
{
}

MinimalView::~MinimalView()
{
}

void MinimalView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  m_Parent->setEnabled(true);

}

void MinimalView::SetFocus ()
{
}

void MinimalView::ChangeExtensionLabelText(const QString& s)
{
  m_Controls.extensionLabel->setText(s);
}
