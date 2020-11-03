/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMITKSurfaceMaterialEditorView.h"

#include "mitkBaseRenderer.h"
#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkIDataStorageService.h"
#include "mitkDataNodeObject.h"

#include "berryIEditorPart.h"
#include "berryIWorkbenchPage.h"

#include "QmitkDataStorageComboBox.h"

#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkRenderer.h>
#include <vtkTextProperty.h>
#include <vtkCoordinate.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include <mitkVtkPropRenderer.h>
#include <mitkVtkLayerController.h>

#include <qmessagebox.h>

#include "mitkStandaloneDataStorage.h"



const std::string QmitkMITKSurfaceMaterialEditorView::VIEW_ID = "org.mitk.views.mitksurfacematerialeditor";

QmitkMITKSurfaceMaterialEditorView::QmitkMITKSurfaceMaterialEditorView()
: QmitkAbstractView(),
  m_Controls(nullptr)
{
  fixedProperties.push_back( "shader" );
  fixedProperties.push_back( "material.representation" );
  fixedProperties.push_back( "color" );
  fixedProperties.push_back( "opacity" );
  fixedProperties.push_back( "material.wireframeLineWidth" );

  fixedProperties.push_back( "material.ambientCoefficient" );
  fixedProperties.push_back( "material.diffuseCoefficient" );
  fixedProperties.push_back( "material.ambientColor" );
  fixedProperties.push_back( "material.diffuseColor" );
  fixedProperties.push_back( "material.specularColor" );
  fixedProperties.push_back( "material.specularCoefficient" );
  fixedProperties.push_back( "material.specularPower" );
  fixedProperties.push_back( "material.interpolation" );

  shaderProperties.push_back( "shader" );
  shaderProperties.push_back( "material.representation" );
  shaderProperties.push_back( "color" );
  shaderProperties.push_back( "opacity" );
  shaderProperties.push_back( "material.wireframeLineWidth" );

  observerAllocated = false;
}

QmitkMITKSurfaceMaterialEditorView::~QmitkMITKSurfaceMaterialEditorView()
{
}

void QmitkMITKSurfaceMaterialEditorView::InitPreviewWindow()
{
  usedTimer=0;

  vtkSphereSource* sphereSource = vtkSphereSource::New();
  sphereSource->SetThetaResolution(25);
  sphereSource->SetPhiResolution(25);
  sphereSource->Update();

  vtkPolyData* sphere = sphereSource->GetOutput();

  m_Surface = mitk::Surface::New();
  m_Surface->SetVtkPolyData( sphere );

  m_DataNode = mitk::DataNode::New();
  m_DataNode->SetData( m_Surface );

  m_DataTree = mitk::StandaloneDataStorage::New();

  m_DataTree->Add( m_DataNode , (mitk::DataNode *)nullptr );

  m_Controls->m_PreviewRenderWindow->GetRenderer()->SetDataStorage( m_DataTree );
  m_Controls->m_PreviewRenderWindow->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );

  sphereSource->Delete();
}


void QmitkMITKSurfaceMaterialEditorView::RefreshPropertiesList()
{
  mitk::DataNode* SrcND = m_SelectedDataNode;
  mitk::DataNode* DstND = m_DataNode;

  mitk::PropertyList* DstPL = DstND->GetPropertyList();


  m_Controls->m_ShaderPropertyList->SetPropertyList( nullptr );

  DstPL->Clear();

  if(observerAllocated)
  {
    observerAllocated=false;
  }

  if(SrcND)
  {
    mitk::PropertyList* SrcPL = SrcND->GetPropertyList();

    std::string shaderState = "fixed";

//    if(shaderEnum.IsNotNull())
//    {
//      shaderState = shaderEnum->GetValueAsString();

//      itk::MemberCommand<QmitkMITKSurfaceMaterialEditorView>::Pointer propertyModifiedCommand = itk::MemberCommand<QmitkMITKSurfaceMaterialEditorView>::New();
//      propertyModifiedCommand->SetCallbackFunction(this, &QmitkMITKSurfaceMaterialEditorView::shaderEnumChange);
//      observerIndex = shaderEnum->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);
//      observedProperty = shaderEnum;
//      observerAllocated=true;
//    }

    MITK_INFO << "PROPERTIES SCAN BEGIN";

    for(mitk::PropertyList::PropertyMap::const_iterator it=SrcPL->GetMap()->begin(); it!=SrcPL->GetMap()->end(); it++)
    {
      std::string name=it->first;
      mitk::BaseProperty *p=it->second;

      // MITK_INFO << "property '" << name << "' found";

      if(shaderState.compare("fixed")==0)
      {
        if(std::find(fixedProperties.begin(), fixedProperties.end(), name) != fixedProperties.end())
        {
          DstPL->SetProperty(name,p);
        }
      }
      else
      {
        //if(std::find(shaderProperties.begin(), shaderProperties.end(), name) != shaderProperties.end())
        {
          DstPL->SetProperty(name,p);
        }
      }
    }

    MITK_INFO << "PROPERTIES SCAN END";
  }

  m_Controls->m_ShaderPropertyList->SetPropertyList( DstPL );
  //m_Controls->m_PreviewRenderWindow->GetRenderer()->GetVtkRenderer()->ResetCameraClippingRange();
}

void QmitkMITKSurfaceMaterialEditorView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkMITKSurfaceMaterialEditorViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    InitPreviewWindow();
    RefreshPropertiesList();

  }
}

void QmitkMITKSurfaceMaterialEditorView::SetFocus()
{
  m_Controls->m_ShaderPropertyList->setFocus();
}

void QmitkMITKSurfaceMaterialEditorView::CreateConnections()
{
}

void QmitkMITKSurfaceMaterialEditorView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  if(!nodes.empty())
  {
    m_SelectedDataNode = nodes.at(0);
    MITK_INFO << "Node '" << m_SelectedDataNode->GetName() << "' selected";
    SurfaceSelected();
  }
}

void QmitkMITKSurfaceMaterialEditorView::SurfaceSelected()
{
  postRefresh();
}

void QmitkMITKSurfaceMaterialEditorView::shaderEnumChange(const itk::Object * /*caller*/, const itk::EventObject & /*event*/)
{
  postRefresh();
}

void QmitkMITKSurfaceMaterialEditorView::postRefresh()
{
  if(usedTimer)
    return;

  usedTimer=startTimer(0);
}

void QmitkMITKSurfaceMaterialEditorView::timerEvent( QTimerEvent *e )
{
  if(usedTimer!=e->timerId())
  {
    MITK_ERROR << "INTERNAL ERROR: usedTimer[" << usedTimer << "] != timerId[" << e->timerId() << "]";
  }

  if(usedTimer)
  {
    killTimer(usedTimer);
    usedTimer=0;
  }

  RefreshPropertiesList();
}
