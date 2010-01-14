/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkMITKSurfaceMaterialEditorView.h"

#include "mitkBaseRenderer.h"
#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"

#include "mitkShaderEnumProperty.h"
#include "mitkShaderRepository.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

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
#include <vtkXMLMaterial.h>
#include <vtkXMLShader.h>
#include <vtkXMLDataElement.h>

#include <mitkVtkPropRenderer.h>
#include <mitkVtkLayerController.h>

#include <qmessagebox.h>

#include "mitkStandaloneDataStorage.h"



const std::string QmitkMITKSurfaceMaterialEditorView::VIEW_ID = "org.mitk.views.mitksurfacematerialeditor";

QmitkMITKSurfaceMaterialEditorView::QmitkMITKSurfaceMaterialEditorView()
: QmitkFunctionality(), 
  m_Controls(NULL),
  m_MultiWidget(NULL)
{
  fixedProperties.push_back( "shader" );
  fixedProperties.push_back( "representation" );
  fixedProperties.push_back( "color" );
  fixedProperties.push_back( "opacity" );
  fixedProperties.push_back( "wireframe line width" );

  fixedProperties.push_back( "color_coefficient" );
  fixedProperties.push_back( "specular_color" );
  fixedProperties.push_back( "specular_coefficient" );
  fixedProperties.push_back( "specular_power" );
  fixedProperties.push_back( "interpolation" );

  shaderProperties.push_back( "shader" );
  shaderProperties.push_back( "representation" );
  shaderProperties.push_back( "color" );
  shaderProperties.push_back( "opacity" );
  shaderProperties.push_back( "wireframe line width" );
  
  observerAllocated = false;
  
  
  mitk::ShaderRepository::GetGlobalShaderRepository();
}

QmitkMITKSurfaceMaterialEditorView::~QmitkMITKSurfaceMaterialEditorView()
{
}

void QmitkMITKSurfaceMaterialEditorView::InitPreviewWindow()
{ 
  
         

  usedTimer=0;
  
  m_MaterialProperty = mitk::MaterialProperty::New();

  vtkSphereSource* sphereSource = vtkSphereSource::New();
  sphereSource->SetThetaResolution(25);
  sphereSource->SetPhiResolution(25);
  sphereSource->Update();
  
  vtkPolyData* sphere = sphereSource->GetOutput();
  
  m_Surface = mitk::Surface::New();
  m_Surface->SetVtkPolyData( sphere );
  
  m_DataTreeNode = mitk::DataTreeNode::New();
  m_DataTreeNode->SetData( m_Surface );
    
  m_DataTree = mitk::StandaloneDataStorage::New();
  
  m_DataTree->Add( m_DataTreeNode , (mitk::DataTreeNode *)0 );
  
  m_Controls->m_PreviewRenderWindow->GetRenderer()->SetDataStorage( m_DataTree );
  m_Controls->m_PreviewRenderWindow->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );
  
  sphereSource->Delete();

  //m_DataTreeNode->GetPropertyList();

  //m_Controls->m_ShaderPropertyList->setNode( m_DataTreeNode );
}


void QmitkMITKSurfaceMaterialEditorView::RefreshPropertiesList()
{ 
  mitk::DataTreeNode* SrcND = m_Controls->m_ImageSelector->GetSelectedNode();
  mitk::DataTreeNode* DstND = m_DataTreeNode;

  mitk::PropertyList* DstPL = DstND->GetPropertyList();

  m_Controls->m_ShaderPropertyList->SetPropertyList( 0 );

  DstPL->Clear();

  if(observerAllocated)
  {
    observedProperty->RemoveObserver( observerIndex );
    observerAllocated=false;
  }

  if(SrcND)
  {
    mitk::PropertyList* SrcPL = SrcND->GetPropertyList();
    
    mitk::ShaderEnumProperty::Pointer shaderEnum = dynamic_cast<mitk::ShaderEnumProperty*>(SrcPL->GetProperty("shader"));
    
    std::string shaderState = "fixed";
    
    if(shaderEnum.IsNotNull())
    {
      shaderState = shaderEnum->GetValueAsString();
      
      itk::MemberCommand<QmitkMITKSurfaceMaterialEditorView>::Pointer propertyModifiedCommand = itk::MemberCommand<QmitkMITKSurfaceMaterialEditorView>::New();
      propertyModifiedCommand->SetCallbackFunction(this, &QmitkMITKSurfaceMaterialEditorView::shaderEnumChange);
      observerIndex = shaderEnum->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);
      observedProperty = shaderEnum;
      observerAllocated=true;
    } 
    
    MITK_INFO << "PROPERTIES SCAN BEGIN";

    for(mitk::PropertyList::PropertyMap::const_iterator it=SrcPL->GetMap()->begin(); it!=SrcPL->GetMap()->end(); it++)
    {
      std::string name=it->first;
      mitk::BaseProperty *p=it->second.first;

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
}

    /*

      // subscribe for property change events

      itk::MemberCommand<QmitkPropertiesTableModel>::Pointer propertyModifiedCommand =
         itk::MemberCommand<QmitkPropertiesTableModel>::New();
      propertyModifiedCommand->SetCallbackFunction(this, &QmitkPropertiesTableModel::PropertyModified);
 
       m_PropertyModifiedObserverTags[it->first] = it->second.first->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);


            itk::MemberCommand<QmitkDataStorageTableModel>::Pointer propertyModifiedCommand = itk::MemberCommand<QmitkDataStorageTableModel>::New();
        propertyModifiedCommand->SetCallbackFunction(this, &QmitkDataStorageTableModel::PropertyModified);

        mitk::BaseProperty* visibilityProperty = (*it)->GetProperty("visible");
        if(visibilityProperty)
          m_PropertyModifiedObserverTags[visibilityProperty] 
        = visibilityProperty->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);

        mitk::BaseProperty* nameProperty = (*it)->GetProperty("name");
        if(nameProperty)
          m_PropertyModifiedObserverTags[nameProperty] 
        = nameProperty->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);


    for(mitk::PropertyList::PropertyMap::const_iterator it=m_PropertyList->GetMap()->begin()
      ; it!=m_PropertyList->GetMap()->end()
      ; it++)
    {
      // add relevant property column values
      m_PropertyListElements.push_back((*it));

      // subscribe for property change events

      itk::MemberCommand<QmitkPropertiesTableModel>::Pointer propertyModifiedCommand =
         itk::MemberCommand<QmitkPropertiesTableModel>::New();
      propertyModifiedCommand->SetCallbackFunction(this, &QmitkPropertiesTableModel::PropertyModified);
 
       m_PropertyModifiedObserverTags[it->first] = it->second.first->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);*/






void QmitkMITKSurfaceMaterialEditorView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkMITKSurfaceMaterialEditorViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    // define data type for combobox
    m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );
    m_Controls->m_ImageSelector->SetPredicate( mitk::NodePredicateDataType::New("Surface") );
  
	  InitPreviewWindow();
	  
	  connect( m_Controls->m_ImageSelector, SIGNAL( OnSelectionChanged(const mitk::DataTreeNode::Pointer) ), this, SLOT( SurfaceSelected() ) );
	  
    RefreshPropertiesList(); 

  }
}

void QmitkMITKSurfaceMaterialEditorView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkMITKSurfaceMaterialEditorView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkMITKSurfaceMaterialEditorView::CreateConnections()
{
  if ( m_Controls )
  {
//    connect( (QObject*)(m_Controls->m_Button), SIGNAL(clicked()), this, SLOT(DoSomething()) );
  }
}

void QmitkMITKSurfaceMaterialEditorView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkMITKSurfaceMaterialEditorView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkMITKSurfaceMaterialEditorView::SurfaceSelected()
{
  postRefresh();
}

void QmitkMITKSurfaceMaterialEditorView::shaderEnumChange(const itk::Object * /*caller*/, const itk::EventObject & /*event*/)
{
  postRefresh();
}


/*
  mitk::DataTreeNode* node = m_Controls->m_ImageSelector->GetSelectedNode();
  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template functionality", "Please load and select an image before starting some action.");
    return;
  }

  // here we have a valid mitk::DataTreeNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is really an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      std::string name("(no name)");
      node->GetName(name);

      QMessageBox::information( NULL, "Image processing",
                                QString( "Doing something to '%1'" ).arg(name.c_str()) );

      // at this point anything can be done using the mitk::Image image.
    }
  }*/

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