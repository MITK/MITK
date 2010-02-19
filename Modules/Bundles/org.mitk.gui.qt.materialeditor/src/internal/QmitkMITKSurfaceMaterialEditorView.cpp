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
#include "mitkIDataStorageService.h"
#include "mitkDataTreeNodeObject.h"

#include "berryIEditorPart.h"
#include "berryIWorkbenchPage.h"

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
  
  
  mitk::ShaderRepository::GetGlobalShaderRepository();
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
  
  m_DataTreeNode = mitk::DataTreeNode::New();
  m_DataTreeNode->SetData( m_Surface );
    
  m_DataTree = mitk::StandaloneDataStorage::New();
  
  m_DataTree->Add( m_DataTreeNode , (mitk::DataTreeNode *)0 );
  
  m_Controls->m_PreviewRenderWindow->GetRenderer()->SetDataStorage( m_DataTree );
  m_Controls->m_PreviewRenderWindow->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );
  
  sphereSource->Delete();
}


void QmitkMITKSurfaceMaterialEditorView::RefreshPropertiesList()
{ 
  mitk::DataTreeNode* SrcND = m_SelectedDataTreeNode;
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
  //m_Controls->m_PreviewRenderWindow->GetRenderer()->GetVtkRenderer()->ResetCameraClippingRange();
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
   /* m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );
    m_Controls->m_ImageSelector->SetPredicate( mitk::NodePredicateDataType::New("Surface") );
  
	  InitPreviewWindow();
	  
	  connect( m_Controls->m_ImageSelector, SIGNAL( OnSelectionChanged(const mitk::DataTreeNode::Pointer) ), this, SLOT( SurfaceSelected() ) );
	  
    RefreshPropertiesList(); */

    InitPreviewWindow();

    // listener for selected entry in the data manager
    m_SelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkMITKSurfaceMaterialEditorView>(this, &QmitkMITKSurfaceMaterialEditorView::SelectionChanged));
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelectionListener);
    berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
    m_CurrentSelection = selection.Cast<const berry::IStructuredSelection>();
    this->SelectionChanged(berry::SmartPointer<IWorkbenchPart>(NULL), selection);
    
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


void QmitkMITKSurfaceMaterialEditorView::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection)
{
  if ( sourcepart == this ||  // prevents being notified by own selection events
	       !selection.Cast<const berry::IStructuredSelection>() ) // checks that the selection is a IStructuredSelection and not NULL
	  {
	    return; // otherwise we get "null selection" events each time the view is activated/focussed
	  }

	  MITK_INFO << "selection changed";
	  // save current selection in member variable
	  m_CurrentSelection = selection.Cast<const berry::IStructuredSelection>();

	  //bool newReferenceImageSelected(false);

	  // TODO warning when two images are selected
	  // do something with the selected items
	  if(m_CurrentSelection)
	  {
	    // iterate selection
	    for (berry::IStructuredSelection::iterator i = m_CurrentSelection->Begin(); i != m_CurrentSelection->End(); ++i)
	    {
	      // extract datatree node
	      if (mitk::DataTreeNodeObject::Pointer nodeObj = i->Cast<mitk::DataTreeNodeObject>())
	      {
	        m_SelectedDataTreeNode = nodeObj->GetDataTreeNode();
          MITK_INFO << "Node '" << m_SelectedDataTreeNode->GetName() << "' selected";

          SurfaceSelected();
          return;
	      }
	    }
	  }
	  //TODO: warning if m_InputImageNode is NULL
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