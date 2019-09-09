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
#include <mitkAffineBaseDataInteractor3D.h>

//micro services
#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

const std::string QmitkGeometryToolsView::VIEW_ID = "org.mitk.views.geometrytools";
const char* QmitkGeometryToolsView::PART_NAME = QT_TRANSLATE_NOOP("Plugin Title", "Geometry Tools");

QmitkGeometryToolsView::~QmitkGeometryToolsView()
{
  auto ds = this->GetDataStorage();
  auto allNodes = ds->GetAll();
  for (auto node : *allNodes) {
    bool isActive(false);
    if (node->GetBoolProperty("AffineBaseDataInteractor3D", isActive) && isActive) {
      node->SetBoolProperty("AffineBaseDataInteractor3D", false);
      node->SetDataInteractor(nullptr);
    }
  }
}

QmitkGeometryToolsView::QmitkGeometryToolsView() :
  m_CurrentGeometry(nullptr)
{
}

void QmitkGeometryToolsView::SetFocus()
{
  m_Controls.m_AddInteractor->setFocus();
}

void QmitkGeometryToolsView::CreateQtPartControl( QWidget *parent )
{
  SetPartName(QApplication::translate("Plugin Title", PART_NAME));

  m_Controls.setupUi( parent );
  connect( m_Controls.m_AddInteractor, SIGNAL(clicked()), this, SLOT(AddInteractor()) );
  connect( m_Controls.m_RemoveInteractor, SIGNAL(clicked()), this, SLOT(RemoveInteractor()) );
  connect( m_Controls.m_TranslationStep, SIGNAL(valueChanged(double)), this, SLOT(OnTranslationSpinBoxChanged(double)) );
  connect( m_Controls.m_RotationStep, SIGNAL(valueChanged(double)), this, SLOT(OnRotationSpinBoxChanged(double)) );
  connect( m_Controls.m_ScaleFactor, SIGNAL(valueChanged(double)), this, SLOT(OnScaleSpinBoxChanged(double)) );
  connect( m_Controls.m_AnchorPointX, SIGNAL(valueChanged(double)), this, SLOT(OnAnchorPointChanged(double)) );
  connect( m_Controls.m_AnchorPointY, SIGNAL(valueChanged(double)), this, SLOT(OnAnchorPointChanged(double)) );
  connect( m_Controls.m_AnchorPointZ, SIGNAL(valueChanged(double)), this, SLOT(OnAnchorPointChanged(double)) );
  connect( m_Controls.m_UsageInfoCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnUsageInfoBoxChanged(bool)) );

  connect( m_Controls.m_CustomAnchorPointRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnCustomPointRadioButtonToggled(bool)) );

  connect( m_Controls.m_OriginPointRadioButton, SIGNAL(clicked(bool)), this, SLOT(OnOriginPointRadioButton(bool)) );
  connect( m_Controls.m_CenterPointRadioButton, SIGNAL(clicked(bool)), this, SLOT(OnCenterPointRadioButton(bool)) );

  m_Controls.m_UsageInfo->hide();
  m_Controls.m_CustomAnchorPoint->hide();
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
      mitk::BaseData::Pointer basedata = node->GetData();
      if (basedata.IsNotNull() && basedata->GetTimeGeometry()->IsValid() && m_CurrentGeometry != basedata->GetTimeGeometry())
      {
        m_CurrentGeometry = basedata->GetTimeGeometry();
        mitk::RenderingManager::GetInstance()->InitializeViews(m_CurrentGeometry, mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
      }
      m_Controls.m_AddInteractor->setEnabled( true );
      return;
    }
  }
  m_Controls.m_AddInteractor->setEnabled( false );
}

void QmitkGeometryToolsView::OnCustomPointRadioButtonToggled(bool status)
{
  m_Controls.m_CustomAnchorPoint->setVisible(status);
  //change the anchor point to be the custom point
  OnAnchorPointChanged(0.0);
}

void QmitkGeometryToolsView::OnCenterPointRadioButton(bool)
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && (node->GetDataInteractor().IsNotNull()) )
    {
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", node->GetData()->GetGeometry()->GetCenter()[0]);
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", node->GetData()->GetGeometry()->GetCenter()[1]);
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", node->GetData()->GetGeometry()->GetCenter()[2]);
    }
  }
}

void QmitkGeometryToolsView::OnOriginPointRadioButton(bool)
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && (node->GetDataInteractor().IsNotNull()) )
    {
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", node->GetData()->GetGeometry()->GetOrigin()[0]);
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", node->GetData()->GetGeometry()->GetOrigin()[1]);
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", node->GetData()->GetGeometry()->GetOrigin()[2]);
    }
  }
}


void QmitkGeometryToolsView::AddInteractor()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() )
    {

      mitk::AffineBaseDataInteractor3D::Pointer affineDataInteractor = mitk::AffineBaseDataInteractor3D::New();
      if (m_Controls.m_KeyboardMode->isChecked())
      {
        affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
        affineDataInteractor->SetEventConfig("AffineKeyConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
      }
      else if(m_Controls.m_MouseMode->isChecked())
      {
        affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
        affineDataInteractor->SetEventConfig("AffineMouseConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
      }

      affineDataInteractor->SetDataNode(node);

      node->SetBoolProperty("pickable", true);
      node->SetBoolProperty("AffineBaseDataInteractor3D", true);
      node->SetFloatProperty("AffineBaseDataInteractor3D.Translation Step Size", m_Controls.m_TranslationStep->value());
      node->SetFloatProperty("AffineBaseDataInteractor3D.Rotation Step Size", m_Controls.m_RotationStep->value());
      node->SetFloatProperty("AffineBaseDataInteractor3D.Scale Step Size", m_Controls.m_ScaleFactor->value());
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", m_Controls.m_AnchorPointX->value());
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", m_Controls.m_AnchorPointY->value());
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", m_Controls.m_AnchorPointZ->value());
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
      node->SetFloatProperty("AffineBaseDataInteractor3D.Translation Step Size", step);
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
      node->SetFloatProperty("AffineBaseDataInteractor3D.Rotation Step Size", step);
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
      node->SetFloatProperty("AffineBaseDataInteractor3D.Scale Step Size", factor);
    }
  }
}

void QmitkGeometryToolsView::OnAnchorPointChanged(double /*value*/)
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && (node->GetDataInteractor().IsNotNull()) )
    {
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point X", m_Controls.m_AnchorPointX->value());
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Y", m_Controls.m_AnchorPointY->value());
      node->SetFloatProperty("AffineBaseDataInteractor3D.Anchor Point Z", m_Controls.m_AnchorPointZ->value());
    }
  }
}
