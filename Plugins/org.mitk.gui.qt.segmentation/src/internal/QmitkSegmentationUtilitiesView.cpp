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
#include "QmitkSegmentationUtilitiesView.h"

// Qt
#include <QMessageBox>


const std::string QmitkSegmentationUtilitiesView::VIEW_ID = "org.mitk.views.segmentationutilities";

void QmitkSegmentationUtilitiesView::SetFocus()
{
//  m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkSegmentationUtilitiesView::CreateQtPartControl( QWidget *parent )
{
  m_Controls.setupUi( parent );
}

void QmitkSegmentationUtilitiesView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      return;
    }
  }
}
