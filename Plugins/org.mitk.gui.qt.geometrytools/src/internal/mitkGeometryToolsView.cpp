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
#include "mitkGeometryToolsView.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>
#include <mitkAffineDataInteractor3D.h>
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

const std::string mitkGeometryToolsView::VIEW_ID = "org.mitk.views.geometrytools";

void mitkGeometryToolsView::SetFocus()
{
  m_Controls.m_TranslateButton->setFocus();
}

void mitkGeometryToolsView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.m_TranslateButton, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
  connect( m_Controls.m_RotateButton, SIGNAL(clicked()), this, SLOT(AddRotationInteractor()) );
}

void mitkGeometryToolsView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() )
    {
      m_Controls.m_TranslateButton->setEnabled( true );
      return;
    }
  }

  m_Controls.m_TranslateButton->setEnabled( false );
}


void mitkGeometryToolsView::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode::Pointer node = nodes.front();

  mitk::AffineDataInteractor3D::Pointer affineDataInteractor = mitk::AffineDataInteractor3D::New();
  affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
  affineDataInteractor->SetEventConfig("AffineTranslationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
  affineDataInteractor->SetDataNode(node);
  node->SetBoolProperty("pickable", true);
}

void mitkGeometryToolsView::AddRotationInteractor()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode::Pointer node = nodes.front();

  mitk::AffineDataInteractor3D::Pointer affineDataInteractor = mitk::AffineDataInteractor3D::New();
  affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
  affineDataInteractor->SetEventConfig("AffineRotationConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
  affineDataInteractor->SetDataNode(node);
  node->SetBoolProperty("pickable", true);
}
