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

#include "QmitkPAUSViewerView.h"

const std::string QmitkPAUSViewerView::VIEW_ID = "org.mitk.views.photoacoustics.pausviewer";

QmitkPAUSViewerView::QmitkPAUSViewerView()
{
}

QmitkPAUSViewerView::~QmitkPAUSViewerView()
{
}

void QmitkPAUSViewerView::InitWindows()
{
}

void QmitkPAUSViewerView::SetFocus()
{
}

void QmitkPAUSViewerView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer>& nodes)
{
}

void QmitkPAUSViewerView::CreateQtPartControl(QWidget *parent)
{
  m_Controls.setupUi(parent);
}