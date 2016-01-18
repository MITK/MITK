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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "OpenIGTLinkPlugin.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

const std::string OpenIGTLinkPlugin::VIEW_ID = "org.mitk.views.openigtlinkplugin";

void OpenIGTLinkPlugin::SetFocus()
{
}

void OpenIGTLinkPlugin::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  //connect(m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()));
}

void OpenIGTLinkPlugin::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer>& nodes)
{
  // iterate all selected objects, adjust warning visibility
}

void OpenIGTLinkPlugin::ConnectButtonClicked()
{
}

void OpenIGTLinkPlugin::ReceivingButtonClicked()
{
}