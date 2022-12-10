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
#include "IterativeClosestPointRegistration.h"

// Qt
#include <QMessageBox>

// mitk image
#include <mitkImage.h>

const std::string IterativeClosestPointRegistration::VIEW_ID = "org.mitk.views.iterativeclosestpointregistration";

void IterativeClosestPointRegistration::SetFocus()
{
}

void IterativeClosestPointRegistration::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.m_performICP, &QPushButton::clicked, this, &IterativeClosestPointRegistration::PerformICP);
}

void IterativeClosestPointRegistration::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{

}

void IterativeClosestPointRegistration::PerformICP()
{
MITK_INFO << "Perform ICP";
}
