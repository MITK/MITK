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


//Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

//Qmitk
#include "QmitkGeometryToolsView.h"

//mitk
#include <mitkImage.h>
#include <mitkAffineDataInteractor3D.h>

//micro services
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

const std::string QmitkGeometryToolsView::VIEW_ID = "org.mitk.views.geometrytools";

void QmitkGeometryToolsView::SetFocus()
{
  m_Controls.m_AddInteractor->setFocus();
}

void QmitkGeometryToolsView::CreateQtPartControl( QWidget *parent )
{
  m_Controls.setupUi( parent );
  connect( m_Controls.m_AddInteractor, SIGNAL(clicked()), this, SLOT(AddInteractor()) );
  connect( m_Controls.m_RemoveInteractor, SIGNAL(clicked()), this, SLOT(RemoveInteractor()) );
  connect( m_Controls.m_TranslationStep, SIGNAL(valueChanged(double)), this, SLOT(OnTranslationSpinBoxChanged(double)) );
  connect( m_Controls.m_RotationStep, SIGNAL(valueChanged(double)), this, SLOT(OnRotationSpinBoxChanged(double)) );
  connect( m_Controls.m_ScaleFactor, SIGNAL(valueChanged(double)), this, SLOT(OnScaleSpinBoxChanged(double)) );
  connect( m_Controls.m_UsageInfoCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnUsageInfoBoxChanged(bool)) );

  m_Controls.m_UsageInfo->hide();
}

void QmitkGeometryToolsView::OnUsageInfoBoxChanged(bool flag)
{
  m_Controls.m_UsageInfo->setVisible(flag);
}

void QmitkGeometryToolsView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() )
    {
      m_Controls.m_AddInteractor->setEnabled( true );
      return;
    }
  }
  m_Controls.m_AddInteractor->setEnabled( false );
}


void QmitkGeometryToolsView::AddInteractor()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() )
    {
      mitk::AffineDataInteractor3D::Pointer affineDataInteractor = mitk::AffineDataInteractor3D::New();
      affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
      affineDataInteractor->SetEventConfig("AffineConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
      affineDataInteractor->SetDataNode(node);
      node->SetBoolProperty("pickable", true);
      node->SetFloatProperty("AffineDataInteractor3D.Translation Step Size", m_Controls.m_TranslationStep->value());
      node->SetFloatProperty("AffineDataInteractor3D.Rotation Step Size", m_Controls.m_RotationStep->value());
      node->SetFloatProperty("AffineDataInteractor3D.Scale Step Size", m_Controls.m_ScaleFactor->value());
    }
  }
}

void QmitkGeometryToolsView::RemoveInteractor()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( (node.IsNotNull()) && (node->GetDataInteractor().IsNotNull()) )
    {
      node->SetDataInteractor(nullptr);
    }
  }
}

void QmitkGeometryToolsView::OnTranslationSpinBoxChanged(double step)
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && (node->GetDataInteractor().IsNotNull()) )
    {
      node->SetFloatProperty("AffineDataInteractor3D.Translation Step Size", step);
    }
  }
}

void QmitkGeometryToolsView::OnRotationSpinBoxChanged(double step)
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && (node->GetDataInteractor().IsNotNull()) )
    {
      node->SetFloatProperty("AffineDataInteractor3D.Rotation Step Size", step);
    }
  }
}

void QmitkGeometryToolsView::OnScaleSpinBoxChanged(double factor)
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && (node->GetDataInteractor().IsNotNull()) )
    {
      node->SetFloatProperty("AffineDataInteractor3D.Scale Step Size", factor);
    }
  }
}
