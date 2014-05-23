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

#include "QmitkNavigationToolCreationAdvancedWidget.h"
#include "QmitkNavigationToolCreationWidget.h"

// mitk includes
#include <mitkRenderingManager.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

// vtk includes
#include <vtkSphereSource.h>
#include <vtkConeSource.h>

const std::string QmitkNavigationToolCreationAdvancedWidget::VIEW_ID = "org.mitk.views.navigationtoolcreationadvancedwidget";

QmitkNavigationToolCreationAdvancedWidget::QmitkNavigationToolCreationAdvancedWidget(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f), m_Controls(NULL), m_DataStorage(NULL), m_ToolTipSurface(NULL), m_ManipulatedToolTip(NULL),
  m_SurfaceNodeName("")
{
  CreateQtPartControl(this);
  CreateConnections();
}

QmitkNavigationToolCreationAdvancedWidget::~QmitkNavigationToolCreationAdvancedWidget()
{
}

void QmitkNavigationToolCreationAdvancedWidget::CreateQtPartControl(QWidget *parent)
  {
    if (!m_Controls)
    {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationToolCreationAdvancedWidgetControls;
    m_Controls->setupUi(parent);
    }
  }

void QmitkNavigationToolCreationAdvancedWidget::CreateConnections()
  {
    if ( m_Controls )
    {
      connect( (QObject*)(this), SIGNAL(finished(int)), this, SLOT(OnClose()));
      connect( (QObject*)(m_Controls->m_InteractiveTransformation), SIGNAL(ApplyManipulatedToolTip()), this, SLOT(OnApplyManipulatedToolTip()));
    }
  }

void QmitkNavigationToolCreationAdvancedWidget::OnClose()
{
  //clean the data storage
 if(m_DataStorage->Exists(m_DataStorage->GetNamedNode("StandardToolTip")))
    {
      m_DataStorage->Remove(m_DataStorage->GetNamedNode("StandardToolTip"));
    }

 if(m_DataStorage.IsNotNull() && m_DataStorage->Exists(m_DataStorage->GetNamedNode("ManipulatedToolTip")))
    {
    m_DataStorage->Remove(m_DataStorage->GetNamedNode("ManipulatedToolTip"));
    }

  emit DialogCloseRequested();
}

void QmitkNavigationToolCreationAdvancedWidget::SetDataStorage( mitk::DataStorage::Pointer dataStorage )
{
  m_DataStorage = dataStorage;
}

void QmitkNavigationToolCreationAdvancedWidget::ReInitialize()
{
   if (m_DataStorage.IsNull()) return;

   //reset some variables
   m_ManipulatedToolTip = NULL;
   m_ToolTipSurface = NULL;
   m_SurfaceNodeName = "";
   mitk::Geometry3D::Pointer geo3d = mitk::Geometry3D::New();
   mitk::BaseGeometry::Pointer emptyGeo = dynamic_cast<mitk::BaseGeometry*>(geo3d.GetPointer());
   m_Controls->m_InteractiveTransformation->SetGeometry(emptyGeo,emptyGeo);

   //call initialization method
   this->RetrieveAndInitializeDataForTooltipManipulation();

   //care for new data nodes
   mitk::DataNode::Pointer manipulatedTipNode = NULL;
   if(!m_DataStorage->Exists(m_DataStorage->GetNamedNode("ManipulatedToolTip")))
    {
      manipulatedTipNode = mitk::DataNode::New();
      manipulatedTipNode->SetData(m_ManipulatedToolTip);
      manipulatedTipNode->SetName("ManipulatedToolTip");
      manipulatedTipNode->SetColor(1.0, 0.0, 0.0);
      manipulatedTipNode->SetOpacity(0.5);
      m_DataStorage->Add(manipulatedTipNode);
    }
    else
    {
      manipulatedTipNode = m_DataStorage->GetNamedNode("ManipulatedToolTip");
      manipulatedTipNode->SetData(m_ManipulatedToolTip);
    }

   // reinit the views with the new nodes
   mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
}

void QmitkNavigationToolCreationAdvancedWidget::RetrieveAndInitializeDataForTooltipManipulation()
{
  // we need the tooltip surface (point or stl)
  emit RetrieveDataForManualToolTipManipulation();

  if(m_ToolTipSurface.IsNotNull())
  {
    mitk::DataNode::Pointer toolTipNode = NULL;

    if(m_SurfaceNodeName.empty())
    {
      m_SurfaceNodeName = "StandardToolTip";
    }

    if(!m_DataStorage->Exists(m_DataStorage->GetNamedNode(m_SurfaceNodeName)))
    {
      toolTipNode = mitk::DataNode::New();
      toolTipNode->SetName(m_SurfaceNodeName);
      toolTipNode->SetData(m_ToolTipSurface);
      m_DataStorage->Add(toolTipNode);

    }
    else
    {
      toolTipNode = m_DataStorage->GetNamedNode(m_SurfaceNodeName);
      toolTipNode->SetData(m_ToolTipSurface);
    }

    m_ManipulatedToolTip = m_ToolTipSurface->Clone();

    mitk::BaseGeometry::Pointer defaultGeo = dynamic_cast<mitk::BaseGeometry*> (mitk::Geometry3D::New().GetPointer());
    defaultGeo->SetIndexToWorldTransform(m_DefaultToolTip);

    m_Controls->m_InteractiveTransformation->SetGeometry(m_ManipulatedToolTip->GetGeometry(),defaultGeo);
  }
  else
  {
    mitkThrow() << "No tooltip surface specified, operation aborted";
  }
}

void QmitkNavigationToolCreationAdvancedWidget::SetDefaultTooltip(mitk::AffineTransform3D::Pointer defaultToolTip)
{
  m_DefaultToolTip = defaultToolTip;
}

void QmitkNavigationToolCreationAdvancedWidget::SetToolTipSurface( bool cone, mitk::DataNode::Pointer node /*= NULL*/ )
{
  m_ToolTipSurface = NULL;
  if(cone)
  {
    m_ToolTipSurface = mitk::Surface::New();
    vtkConeSource *vtkData = vtkConeSource::New();
    vtkData->SetAngle(5.0);
    vtkData->SetResolution(50);
    vtkData->SetHeight(6.0f);
    vtkData->SetRadius(2.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);

    vtkData->Update();
    m_ToolTipSurface->SetVtkPolyData(vtkData->GetOutput());

    vtkData->Delete();
    m_SurfaceNodeName ="";
  }
  else if(!cone && node.IsNotNull())
  {
    m_ToolTipSurface = dynamic_cast<mitk::Surface*>(node->GetData());
    m_SurfaceNodeName = node->GetName();
  }
}

void QmitkNavigationToolCreationAdvancedWidget::OnApplyManipulatedToolTip()
{
  //save manipulated surface object, which holds the tooltip as geometry
  m_ManipulatedToolTip = dynamic_cast<mitk::Surface*>(m_DataStorage->GetNamedNode("ManipulatedToolTip")->GetData()->Clone().GetPointer());
  //then close the window
  OnClose();
}

mitk::AffineTransform3D::Pointer QmitkNavigationToolCreationAdvancedWidget::GetManipulatedToolTip()
{
  mitk::AffineTransform3D::Pointer returnValue = mitk::AffineTransform3D::New();
  if (m_ManipulatedToolTip.IsNotNull()) returnValue = m_ManipulatedToolTip->GetGeometry()->GetIndexToWorldTransform();
  else returnValue->SetIdentity();
  return returnValue;
}
