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
#include "mitkRenderingManager.h"

// vtk includes
#include "vtkSphereSource.h"
#include <vtkConeSource.h>

const std::string QmitkNavigationToolCreationAdvancedWidget::VIEW_ID = "org.mitk.views.navigationtoolcreationadvancedwidget";

QmitkNavigationToolCreationAdvancedWidget::QmitkNavigationToolCreationAdvancedWidget(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f), m_Controls(NULL), m_DataStorage(NULL), m_ToolTipSurface(NULL), m_ManipulatedToolTip(NULL),
  m_SurfaceNodeName("")
{
  CreateQtPartControl(this);
  CreateConnections();
  m_Controls->m_InteractiveTransformation->hide();

  m_Controls->m_ToolTypeChooser->setCurrentIndex(0);
  m_ToolType = Instrument;   // initialize according to GUI setting;
}

QmitkNavigationToolCreationAdvancedWidget::~QmitkNavigationToolCreationAdvancedWidget()
{
 //clean the data storage
 if(m_DataStorage.IsNotNull() && m_DataStorage->Exists(m_DataStorage->GetNamedNode("ManipulatedToolTip")))
    {
    m_DataStorage->Remove(m_DataStorage->GetNamedNode("ManipulatedToolTip"));
    }
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
      connect( (QObject*)(m_Controls->m_ToolTypeChooser), SIGNAL(stateChanged(int)), this, SLOT(OnToolTypeChanged(int)) );
      connect( (QObject*)(m_Controls->m_ManipulateTooltipCB), SIGNAL(stateChanged(int)), this, SLOT(OnManipulateTooltip(int)) );
      connect( (QObject*)(this), SIGNAL(finished(int)), this, SLOT(OnClose()));
      connect( (QObject*)(m_Controls->m_InteractiveTransformation), SIGNAL(ApplyManipulatedToolTip()), this, SLOT(OnApplyManipulatedToolTip()));
    }
  }

void QmitkNavigationToolCreationAdvancedWidget::OnClose()
{
  emit DialogCloseRequested();
}

void QmitkNavigationToolCreationAdvancedWidget::OnToolTypeChanged(int state)
{
  switch (state)
  {
  case 0:
    m_ToolType = Instrument;
      break;
  case 1:
    m_ToolType = Fiducial;
    break;
  case 2:
    m_ToolType = Skinmarker;
    break;
  default:
    m_ToolType = Unknown;
  }
}

QmitkNavigationToolCreationAdvancedWidget::ToolType
QmitkNavigationToolCreationAdvancedWidget::GetToolType()
{
  return m_ToolType;
}

void QmitkNavigationToolCreationAdvancedWidget::SetToolType( int type )
{
  m_ToolType = (ToolType) type;
  m_Controls->m_ToolTypeChooser->setCurrentIndex(type);
}

std::string QmitkNavigationToolCreationAdvancedWidget::GetToolIdentifier()
{
  return m_Controls->m_IdentifierEdit->text().toStdString();
}

std::string QmitkNavigationToolCreationAdvancedWidget::GetSerialNumber()
{
  return m_Controls->m_SerialNumberEdit->text().toStdString();
}

void QmitkNavigationToolCreationAdvancedWidget::SetToolIdentifier( std::string _arg )
{
  m_Controls->m_IdentifierEdit->setText(QString(_arg.c_str()));
}

void QmitkNavigationToolCreationAdvancedWidget::SetSerialNumber( std::string _arg )
{
  m_Controls->m_SerialNumberEdit->setText(QString(_arg.c_str()));
}

void QmitkNavigationToolCreationAdvancedWidget::SetDataStorage( mitk::DataStorage::Pointer dataStorage )
{
  m_DataStorage = dataStorage;
}

void QmitkNavigationToolCreationAdvancedWidget::OnManipulateTooltip(int state)
{
  if(state == Qt::Checked)
  {
    this->RetrieveAndInitializeDataForTooltipManipulation();
    m_Controls->m_InteractiveTransformation->show();
  }
  else
  {
    m_Controls->m_InteractiveTransformation->hide();

    // clean the data storage
    if(m_DataStorage->Exists(m_DataStorage->GetNamedNode("StandardToolTip")))
    {
      m_DataStorage->Remove(m_DataStorage->GetNamedNode("StandardToolTip"));
    }

    if(m_DataStorage->Exists(m_DataStorage->GetNamedNode("ManipulatedToolTip")))
    {
      m_DataStorage->Remove(m_DataStorage->GetNamedNode("ManipulatedToolTip"));
    }
  }
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
    //m_ManipulatedToolTip->GetGeometry()->SetIndexToWorldTransform(this->m_DefaultToolTip);

    mitk::Geometry3D::Pointer defaultGeo = mitk::Geometry3D::New();
    defaultGeo->SetIndexToWorldTransform(m_DefaultToolTip);

    m_Controls->m_InteractiveTransformation->SetGeometry(m_ManipulatedToolTip->GetGeometry(),defaultGeo);

    // reinit the views with the new nodes
    mitk::DataStorage::SetOfObjects::ConstPointer rs = m_DataStorage->GetAll();
    mitk::TimeSlicedGeometry::Pointer bounds = m_DataStorage->ComputeBoundingGeometry3D(rs, "visible");    // initialize the views to the bounding geometry
    mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  }
  else
  {
    this->OnManipulateTooltip(0);
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
}

mitk::AffineTransform3D::Pointer QmitkNavigationToolCreationAdvancedWidget::GetManipulatedToolTip()
{
  mitk::AffineTransform3D::Pointer returnValue;
  if (m_ManipulatedToolTip.IsNotNull()) returnValue = m_ManipulatedToolTip->GetGeometry()->GetIndexToWorldTransform();
  else returnValue->SetIdentity();
  return returnValue;
}