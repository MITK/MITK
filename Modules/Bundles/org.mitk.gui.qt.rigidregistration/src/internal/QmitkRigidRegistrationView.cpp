/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkRigidRegistrationView.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qradiobutton.h"
#include "qslider.h"
#include "qtooltip.h"

#include <vtkTransform.h>

#include "mitkDataTreeNodeObject.h"

#include "cherryIWorkbenchWindow.h"
#include "cherryISelectionService.h"


const std::string QmitkRigidRegistrationView::VIEW_ID = "org.mitk.views.rigidregistration";

using namespace cherry;

struct SelListenerRigidRegistration : ISelectionListener
{
  cherryObjectMacro(SelListenerRigidRegistration);

  SelListenerRigidRegistration(QmitkRigidRegistrationView* view)
  {
    m_View = view;
  }

  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
    // save current selection in member variable
    m_View->m_CurrentSelection = selection.Cast<const IStructuredSelection>();
    m_View->m_Controls.TextLabelFixed->hide();
    m_View->m_Controls.m_FixedLabel->hide();
    m_View->m_Controls.TextLabelMoving->hide();
    m_View->m_Controls.m_MovingLabel->hide();

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {
      m_View->SetImagesVisible(selection);
      if (m_View->m_CurrentSelection->Size() != 2)
      {
        m_View->m_Controls.m_StatusLabel->setText("You have to select two images from Datamanager for Registration!");
        m_View->m_Controls.m_StatusLabel->show();
        m_View->FixedSelected(NULL);
        m_View->FixedSelected(NULL);
      }
      else
      {
        m_View->m_Controls.m_StatusLabel->hide();
        bool foundFixedImage = false;
        // iterate selection
        for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin(); 
          i != m_View->m_CurrentSelection->End(); ++i)
        {
          // extract datatree node
          if (mitk::DataTreeNodeObject::Pointer nodeObj = i->Cast<mitk::DataTreeNodeObject>())
          {
            mitk::DataTreeNode::Pointer node = nodeObj->GetDataTreeNode();
            // only look at interesting types
            if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
            {
              if (foundFixedImage == false)
              {
                m_View->FixedSelected(node);
                foundFixedImage = true;
              }
              else
                m_View->MovingSelected(node);
            }
            else
            {
              m_View->FixedSelected(NULL);
              m_View->FixedSelected(NULL);
              m_View->m_Controls.m_StatusLabel->setText("You have to select two images from Datamanager for Registration!");
              return;
            }
          }
        }
      }
    }
  }

  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname(part->GetPartName().c_str());
      if(partname.compare("Datamanager")==0)
      {
        // apply selection
        DoSelectionChanged(selection);
      }
    }
  }

  QmitkRigidRegistrationView* m_View;
};

QmitkRigidRegistrationView::QmitkRigidRegistrationView(QObject * /*parent*/, const char * /*name*/)
: QmitkFunctionality(), m_MultiWidget(NULL), /*m_MovingNode(NULL), m_MovingMaskNode(NULL), m_FixedNode(NULL), m_FixedMaskNode(NULL),*/ 
  m_ShowRedGreen(false), m_ShowFixedImage(false), m_ShowMovingImage(false), m_ShowBothImages(true), m_Opacity(0.5), m_OriginalOpacity(1.0), m_OldMovingLayer(0),
  m_NewMovingLayer(0), m_OldMovingLayerSet(false), m_NewMovingLayerSet(false), m_Deactivated(false),m_FixedDimension(0), m_MovingDimension(0)
{
  m_TranslateSliderPos[0] = 0;
  m_TranslateSliderPos[1] = 0;
  m_TranslateSliderPos[2] = 0;

  m_RotateSliderPos[0] = 0;
  m_RotateSliderPos[1] = 0;
  m_RotateSliderPos[2] = 0;
  translationParams = new int[3];
  rotationParams = new int[3];

  m_TimeStepperAdapter = NULL;
}

QmitkRigidRegistrationView::~QmitkRigidRegistrationView()
{
}

void QmitkRigidRegistrationView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);
  m_Controls.m_ManualFrame->hide();
  m_Controls.timeSlider->hide();
  m_Controls.TextLabelFixed->hide();
  m_Controls.m_FixedLabel->hide();
  m_Controls.TextLabelMoving->hide();
  m_Controls.m_MovingLabel->hide();
  m_Controls.m_ManualFrame->setEnabled(false);
  m_Parent->setEnabled(false);
  this->CreateConnections();
  this->CheckCalculateEnabled();
  m_SelListener = cherry::ISelectionListener::Pointer(new SelListenerRigidRegistration(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  cherry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<SelListenerRigidRegistration>()->DoSelectionChanged(sel);
}

void QmitkRigidRegistrationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_Parent->setEnabled(true);
  m_MultiWidget = &stdMultiWidget;
  m_MultiWidget->SetWidgetPlanesVisibility(true);
  connect(this, SIGNAL( reinitFixed(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
  connect(this, SIGNAL( reinitMoving(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
}

void QmitkRigidRegistrationView::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
  m_MultiWidget = NULL;
}

void QmitkRigidRegistrationView::CreateConnections()
{
  connect( m_Controls.m_ManualRegistrationCheckbox, SIGNAL(toggled(bool)), this, SLOT(ShowManualRegistrationFrame(bool)));
  connect(m_Controls.m_ShowRedGreenValues, SIGNAL(toggled(bool)), this, SLOT(ShowRedGreen(bool)));
  connect(m_Controls.m_OpacitySlider, SIGNAL(sliderMoved(int)), this, SLOT(OpacityUpdate(int)));
  connect(m_Controls.m_CalculateTransformation, SIGNAL(clicked()), this, SLOT(Calculate()));
  connect(m_Controls.m_UndoTransformation,SIGNAL(clicked()),this,SLOT(UndoTransformation()));
  connect(m_Controls.m_RedoTransformation,SIGNAL(clicked()),this,SLOT(RedoTransformation()));
  connect(m_Controls.m_AutomaticTranslation,SIGNAL(clicked()),this,SLOT(AlignCenters()));
  connect(m_Controls.m_StopOptimization,SIGNAL(clicked()), this , SLOT(StopOptimizationClicked()));
  connect(m_Controls.m_XTransSlider, SIGNAL(valueChanged(int)), this, SLOT(xTrans_valueChanged(int)));
  connect(m_Controls.m_YTransSlider, SIGNAL(valueChanged(int)), this, SLOT(yTrans_valueChanged(int)));
  connect(m_Controls.m_ZTransSlider, SIGNAL(valueChanged(int)), this, SLOT(zTrans_valueChanged(int)));
  connect(m_Controls.m_XRotSlider, SIGNAL(valueChanged(int)), this, SLOT(xRot_valueChanged(int)));
  connect(m_Controls.m_YRotSlider, SIGNAL(valueChanged(int)), this, SLOT(yRot_valueChanged(int)));
  connect(m_Controls.m_ZRotSlider, SIGNAL(valueChanged(int)), this, SLOT(zRot_valueChanged(int)));
  connect(m_Controls.m_LoadRigidRegistrationParameter, SIGNAL(clicked()), m_Controls.qmitkRigidRegistrationSelector1, SLOT(LoadRigidRegistrationParameter()));
  connect(m_Controls.m_SaveRigidRegistrationParameter, SIGNAL(clicked()), m_Controls.qmitkRigidRegistrationSelector1, SLOT(SaveRigidRegistrationParameter()));
  connect(m_Controls.qmitkRigidRegistrationSelector1,SIGNAL(OptimizerChanged(double)),this,SLOT(SetOptimizerValue( double )));
  connect(m_Controls.qmitkRigidRegistrationSelector1,SIGNAL(TransformChanged()),this,SLOT(CheckCalculateEnabled()));
  connect(m_Controls.qmitkRigidRegistrationSelector1,SIGNAL(AddNewTransformationToUndoList()),this,SLOT(AddNewTransformationToUndoList()));
}

void QmitkRigidRegistrationView::Activated()
{
  m_Deactivated = false;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  this->FixedMaskSelected();
  this->MovingMaskSelected();
  this->OpacityUpdate(m_Opacity);
  this->ClearTransformationLists();
  this->CheckCalculateEnabled();
}

void QmitkRigidRegistrationView::Deactivated()
{
  m_Deactivated = true;
  this->SetImageColor(false);
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
    if (m_OldMovingLayerSet)
    {
      m_MovingNode->SetIntProperty("layer", m_OldMovingLayer);
      m_OldMovingLayerSet = false;
    }
  }
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  this->ClearTransformationLists();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Deactivated();
}

void QmitkRigidRegistrationView::FixedSelected(mitk::DataTreeNode::Pointer fixedImage)
{
  if (m_FixedNode.IsNotNull())
  {
    this->SetImageColor(false);
  }
  m_FixedNode = fixedImage;
  if (m_FixedNode.IsNotNull())
  {
    m_FixedNode->SetVisibility(true);
    m_Controls.TextLabelFixed->setText("Fixed Image: " + QString::fromStdString(m_FixedNode->GetName()));
    m_Controls.m_FixedLabel->show();
    m_Controls.TextLabelFixed->show();
    mitk::ColorProperty::Pointer colorProperty;
    colorProperty = dynamic_cast<mitk::ColorProperty*>(m_FixedNode->GetProperty("color"));
    if ( colorProperty.IsNotNull() )
    {
      m_FixedColor = colorProperty->GetColor();
    }
    this->SetImageColor(m_ShowRedGreen);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    if (dynamic_cast<mitk::Image*>(m_FixedNode->GetData()))
    {
      m_FixedDimension = dynamic_cast<mitk::Image*>(m_FixedNode->GetData())->GetDimension();
      m_Controls.qmitkRigidRegistrationSelector1->SetFixedDimension(m_FixedDimension);
      m_Controls.qmitkRigidRegistrationSelector1->SetFixedNode(m_FixedNode);
    }
  }
  else
  {
    m_Controls.m_FixedLabel->hide();
    m_Controls.TextLabelFixed->hide();
  }
  this->CheckCalculateEnabled();
  if(this->GetActiveStdMultiWidget())
  {
    m_TimeStepperAdapter = new QmitkStepperAdapter((QObject*) m_Controls.timeSlider, m_MultiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromRigidRegistration");
    connect( m_TimeStepperAdapter, SIGNAL( Refetch() ), this, SLOT( UpdateTimestep() ) );
  }
}

void QmitkRigidRegistrationView::MovingSelected(mitk::DataTreeNode::Pointer movingImage)
{
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
    this->SetImageColor(false);
  }
  m_MovingNode = movingImage;
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetVisibility(true);
    m_Controls.TextLabelMoving->setText("Moving Image: " + QString::fromStdString(m_MovingNode->GetName()));
    m_Controls.m_MovingLabel->show();
    m_Controls.TextLabelMoving->show();
    mitk::ColorProperty::Pointer colorProperty;
    colorProperty = dynamic_cast<mitk::ColorProperty*>(m_MovingNode->GetProperty("color"));
    if ( colorProperty.IsNotNull() )
    {
      m_MovingColor = colorProperty->GetColor();
    }
    this->SetImageColor(m_ShowRedGreen);
    m_MovingNode->GetFloatProperty("opacity", m_OriginalOpacity);
    this->OpacityUpdate(m_Opacity);
  }
  else
  {
    m_Controls.m_MovingLabel->hide();
    m_Controls.TextLabelMoving->hide();
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->MovingImageChanged();
  this->CheckCalculateEnabled();
}

void QmitkRigidRegistrationView::MovingMaskSelected(int)
{
//   if (m_Controls.m_MovingSelector->GetSelectedNode().IsNotNull())
//   {
//     if (m_MovingMaskNode != m_Controls.m_MovingSelector->GetSelectedNode())
//     {
//       if (m_MovingMaskNode != NULL)
//       {
//         if (m_MovingMaskNode != m_FixedNode || m_MovingMaskNode != m_MovingNode)
//         {
//           m_MovingNode->SetVisibility(false);
//         }
//       }
//       m_MovingMaskNode = m_Controls.m_MovingSelector->GetSelectedNode();
//       if (m_ShowBothImages)
//       {
//         this->ShowBothImages();
//       }
//       if (m_ShowFixedImage)
//       {
//         this->ShowFixedImage();
//       }
//       if (m_ShowMovingImage)
//       {
//         this->ShowMovingImage();
//       }
//       mitk::RenderingManager::GetInstance()->RequestUpdateAll();
//       this->ClearTransformationLists();
//       this->OpacityUpdate(m_Opacity);
//     }
//     this->MovingMaskImageChanged();
//   }
}

void QmitkRigidRegistrationView::FixedMaskSelected(int)
{
//   if (m_Controls.m_FixedSelector->GetSelectedNode().IsNotNull())
//   {
//     if (m_FixedMaskNode != m_Controls.m_FixedSelector->GetSelectedNode())
//     {
//       if (m_FixedMaskNode != NULL)
//       {
//         if (m_FixedMaskNode != m_FixedNode || m_FixedMaskNode != m_MovingNode || m_FixedMaskNode != m_MovingMaskNode )
//         {
//           m_FixedMaskNode->SetVisibility(false);
//         }
//       }
//       m_FixedMaskNode = m_Controls.m_FixedSelector->GetSelectedNode();
//       if (m_ShowBothImages)
//       {
//         this->ShowBothImages();
//       }
//       if (m_ShowFixedImage)
//       {
//         this->ShowFixedImage();
//       }
//       if (m_ShowMovingImage)
//       {
//         this->ShowMovingImage();
//       }
//       mitk::RenderingManager::GetInstance()->RequestUpdateAll();
//       this->ClearTransformationLists();
//       this->OpacityUpdate(m_Opacity);
//     }
//     this->FixedMaskImageChanged();
//   }
}

bool QmitkRigidRegistrationView::CheckCalculate()
{
  if(m_MovingNode==m_FixedNode)
    return false;
  return true;
}

void QmitkRigidRegistrationView::AddNewTransformationToUndoList()
{
  mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
  m_UndoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingData->GetGeometry(0)->Clone().GetPointer()));
  m_RedoGeometryList.clear();
  this->SetUndoEnabled(true);
  this->SetRedoEnabled(false);
}

void QmitkRigidRegistrationView::UndoTransformation()
{
  if(!m_UndoGeometryList.empty())
  {
    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    m_RedoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingData->GetGeometry(0)->Clone().GetPointer()));
    movingData->SetGeometry(m_UndoGeometryList.back());
    m_UndoGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingNode->SetMapper(1, NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_MultiWidget->mitkWidget4->GetRenderWindow());

    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    this->SetRedoEnabled(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();    
  }
  if(!m_UndoGeometryList.empty())
  {
    this->SetUndoEnabled(true);
  }
  else
  {
    this->SetUndoEnabled(false);
  }
  this->CheckCalculateEnabled();
}

void QmitkRigidRegistrationView::RedoTransformation()
{
  if(!m_RedoGeometryList.empty())
  {
    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    m_UndoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingData->GetGeometry(0)->Clone().GetPointer()));
    movingData->SetGeometry(m_RedoGeometryList.back());
    m_RedoGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingNode->SetMapper(1, NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_MultiWidget->mitkWidget4->GetRenderWindow());

    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    this->SetUndoEnabled(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  if(!m_RedoGeometryList.empty())
  {
    this->SetRedoEnabled(true);
  }
  else
  {
    this->SetRedoEnabled(false);
  }
}

void QmitkRigidRegistrationView::ShowRedGreen(bool redGreen)
{
  m_ShowRedGreen = redGreen;
  this->SetImageColor(m_ShowRedGreen);
}

void QmitkRigidRegistrationView::SetImageColor(bool redGreen)
{
  if (!redGreen && m_FixedNode.IsNotNull())
  {
    m_FixedNode->SetColor(m_FixedColor);
  }
  if (!redGreen && m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetColor(m_MovingColor);
  }
  if (redGreen && m_FixedNode.IsNotNull())
  {
    m_FixedNode->SetColor(1.0f, 0.0f, 0.0f);
  }
  if (redGreen && m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetColor(0.0f, 1.0f, 0.0f);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRigidRegistrationView::OpacityUpdate(float opacity)
{
  m_Opacity = opacity;
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_Opacity);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRigidRegistrationView::OpacityUpdate(int opacity)
{
  float fValue = ((float)opacity)/100.0f;
  this->OpacityUpdate(fValue);
}

void QmitkRigidRegistrationView::ClearTransformationLists()
{
  this->SetUndoEnabled(false);
  this->SetRedoEnabled(false);
  m_UndoGeometryList.clear();
  m_RedoGeometryList.clear();
}

void QmitkRigidRegistrationView::Translate(int* translateVector)
{ 
  if (m_MovingNode.IsNotNull())
  {
    mitk::Vector3D translateVec; 

    translateVec[0] = translateVector[0] - m_TranslateSliderPos[0];
    translateVec[1] = translateVector[1] - m_TranslateSliderPos[1];
    translateVec[2] = translateVector[2] - m_TranslateSliderPos[2];

    m_TranslateSliderPos[0] = translateVector[0];
    m_TranslateSliderPos[1] = translateVector[1];
    m_TranslateSliderPos[2] = translateVector[2];

    m_MovingNode->GetData()->GetGeometry()->Translate( translateVec );
    if (m_MovingMaskNode.IsNotNull())
    {
      m_MovingMaskNode->GetData()->GetGeometry()->Translate( translateVec );
      m_MovingMaskNode->GetData()->Modified();
    }
    m_MovingNode->GetData()->Modified();
    m_RedoGeometryList.clear();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRigidRegistrationView::Rotate(int* rotateVector)
{ 
  if (m_MovingNode.IsNotNull())
  {
    mitk::Vector3D rotateVec; 

    rotateVec[0] = rotateVector[0] - m_RotateSliderPos[0];
    rotateVec[1] = rotateVector[1] - m_RotateSliderPos[1];
    rotateVec[2] = rotateVector[2] - m_RotateSliderPos[2];

    m_RotateSliderPos[0] = rotateVector[0];
    m_RotateSliderPos[1] = rotateVector[1];
    m_RotateSliderPos[2] = rotateVector[2];

    vtkMatrix4x4* rotationMatrix = vtkMatrix4x4::New();
    vtkMatrix4x4* translationMatrix = vtkMatrix4x4::New();
    rotationMatrix->Identity();
    translationMatrix->Identity();

    double (*rotMatrix)[4] = rotationMatrix->Element;
    double (*transMatrix)[4] = translationMatrix->Element;
    
    mitk::Point3D centerBB = m_MovingNode->GetData()->GetGeometry()->GetCenter();

    transMatrix[0][3] = centerBB[0];
    transMatrix[1][3] = centerBB[1];
    transMatrix[2][3] = centerBB[2];

    translationMatrix->Invert();

    m_MovingNode->GetData()->GetGeometry()->Compose( translationMatrix );
    m_MovingMaskNode->GetData()->GetGeometry()->Compose( translationMatrix );

    double radianX = rotateVec[0] * vnl_math::pi / 180;
    double radianY = rotateVec[1] * vnl_math::pi / 180;
    double radianZ = rotateVec[2] * vnl_math::pi / 180;

    if ( rotateVec[0] != 0 )
    {   
      rotMatrix[1][1] = cos( radianX );
      rotMatrix[1][2] = -sin( radianX );
      rotMatrix[2][1] = sin( radianX );
      rotMatrix[2][2] = cos( radianX );
    }
    else if ( rotateVec[1] != 0 )
    {
      rotMatrix[0][0] = cos( radianY );
      rotMatrix[0][2] = sin( radianY );
      rotMatrix[2][0] = -sin( radianY );
      rotMatrix[2][2] = cos( radianY );      
    } 
    else if ( rotateVec[2] != 0 )
    {
      rotMatrix[0][0] = cos( radianZ );
      rotMatrix[0][1] = -sin( radianZ );
      rotMatrix[1][0] = sin( radianZ );
      rotMatrix[1][1] = cos( radianZ );      
    }

    m_MovingNode->GetData()->GetGeometry()->Compose( rotationMatrix );
    m_MovingMaskNode->GetData()->GetGeometry()->Compose( rotationMatrix );
    
    translationMatrix->Invert();

    m_MovingNode->GetData()->GetGeometry()->Compose( translationMatrix );
    if (m_MovingMaskNode.IsNotNull())
    {
      m_MovingMaskNode->GetData()->GetGeometry()->Compose( translationMatrix );
      m_MovingMaskNode->GetData()->Modified();
    }
    m_MovingNode->GetData()->Modified();
    m_RedoGeometryList.clear();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRigidRegistrationView::AlignCenters()
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull())
  {
    mitk::Point3D fixedPoint = m_FixedNode->GetData()->GetGeometry()->GetCenter();
    mitk::Point3D movingPoint = m_MovingNode->GetData()->GetGeometry()->GetCenter();
    mitk::Vector3D translateVec;
    translateVec =  fixedPoint - movingPoint;
    m_Controls.m_XTransSlider->setValue((int)m_Controls.m_XTransSlider->value() + (int)translateVec[0]);
    m_Controls.m_YTransSlider->setValue((int)m_Controls.m_YTransSlider->value() + (int)translateVec[1]);
    m_Controls.m_ZTransSlider->setValue((int)m_Controls.m_ZTransSlider->value() + (int)translateVec[2]);
  }
}

void QmitkRigidRegistrationView::SetUndoEnabled( bool enable )
{
  m_Controls.m_UndoTransformation->setEnabled(enable);
}

void QmitkRigidRegistrationView::SetRedoEnabled( bool enable )
{
  m_Controls.m_RedoTransformation->setEnabled(enable);
}

void QmitkRigidRegistrationView::CheckCalculateEnabled()
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull())
  {
    m_Controls.m_ManualFrame->setEnabled(true);
    m_Controls.m_CalculateTransformation->setEnabled(true);
    if ( (m_FixedDimension != m_MovingDimension && std::max<int>(m_FixedDimension, m_MovingDimension) != 4) || m_FixedDimension < 2 /*|| m_FixedDimension > 3*/)
    {
      m_Controls.m_CalculateTransformation->setEnabled(false);
    }
    else if (m_Controls.qmitkRigidRegistrationSelector1->GetSelectedTransform() < 5 &&  (m_FixedDimension < 2) /*|| m_FixedDimension > 3)*/)
    {
      m_Controls.m_CalculateTransformation->setEnabled(false);
    }
    else if ((m_Controls.qmitkRigidRegistrationSelector1->GetSelectedTransform() > 4 && m_Controls.qmitkRigidRegistrationSelector1->GetSelectedTransform() < 13) && !(m_FixedDimension > 2))
    {
      m_Controls.m_CalculateTransformation->setEnabled(false);
    }
    else if (m_Controls.qmitkRigidRegistrationSelector1->GetSelectedTransform() > 12 &&  m_FixedDimension != 2)
    {
      m_Controls.m_CalculateTransformation->setEnabled(false);
    }
  }
  else
  {
    m_Controls.m_CalculateTransformation->setEnabled(false);
    m_Controls.m_ManualFrame->setEnabled(false);
  }
}

void QmitkRigidRegistrationView::xTrans_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    translationParams[0]=v;
    translationParams[1]=m_Controls.m_YTransSlider->value();
    translationParams[2]=m_Controls.m_ZTransSlider->value();
    m_Controls.m_XTransSlider->setToolTip("");
    char str[33];
    sprintf(str,"%d",v);    
    m_Controls.m_XTransSlider->setToolTip( tr( str ) );
    Translate(translationParams);
  }
  else
  {
    MovingImageChanged();
  }  
}

void QmitkRigidRegistrationView::yTrans_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    translationParams[0]=m_Controls.m_XTransSlider->value();
    translationParams[1]=v;
    translationParams[2]=m_Controls.m_ZTransSlider->value();
    m_Controls.m_YTransSlider->setToolTip("");
    char str[33];
    sprintf(str,"%d",v);    
    m_Controls.m_YTransSlider->setToolTip( tr( str ) );
    Translate(translationParams);
  }
  else
  {
    MovingImageChanged();
  }  
}

void QmitkRigidRegistrationView::zTrans_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    translationParams[0]=m_Controls.m_XTransSlider->value();
    translationParams[1]=m_Controls.m_YTransSlider->value();
    translationParams[2]=v;
    m_Controls.m_ZTransSlider->setToolTip("");
    char str[33];
    sprintf(str,"%d",v);    
    m_Controls.m_ZTransSlider->setToolTip( tr( str ) );
    Translate(translationParams);
  }
  else
  {
    MovingImageChanged();
  } 
}

void QmitkRigidRegistrationView::xRot_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    rotationParams[0]=v;
    rotationParams[1]=m_Controls.m_YRotSlider->value();
    rotationParams[2]=m_Controls.m_ZRotSlider->value();
    m_Controls.m_XRotSlider->setToolTip("");
    char str[33];
    sprintf(str,"%d",v);    
    m_Controls.m_XRotSlider->setToolTip( tr( str ) );
    Rotate(rotationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::yRot_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    rotationParams[0]=m_Controls.m_XRotSlider->value();
    rotationParams[1]=v;
    rotationParams[2]=m_Controls.m_ZRotSlider->value();
    m_Controls.m_YRotSlider->setToolTip("");
    char str[33];
    sprintf(str,"%d",v);    
    m_Controls.m_YRotSlider->setToolTip( tr( str ) );
    Rotate(rotationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::zRot_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    rotationParams[0]=m_Controls.m_XRotSlider->value();
    rotationParams[1]=m_Controls.m_YRotSlider->value();
    rotationParams[2]=v;
    m_Controls.m_ZRotSlider->setToolTip("");
    char str[33];
    sprintf(str,"%d",v);    
    m_Controls.m_ZRotSlider->setToolTip( tr( str ) );
    Rotate(rotationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::MovingImageChanged()
{
  if (dynamic_cast<mitk::Image*>(m_MovingNode->GetData()))
  {
    m_Controls.m_XTransSlider->setValue(0);
    m_Controls.m_YTransSlider->setValue(0);
    m_Controls.m_ZTransSlider->setValue(0);
    translationParams[0]=0;
    translationParams[1]=0;
    translationParams[2]=0;
    m_Controls.m_XRotSlider->setValue(0);
    m_Controls.m_YRotSlider->setValue(0);
    m_Controls.m_ZRotSlider->setValue(0);
    rotationParams[0]=0;
    rotationParams[1]=0;
    rotationParams[2]=0;
    m_MovingDimension = dynamic_cast<mitk::Image*>(m_MovingNode->GetData())->GetDimension();
    m_Controls.qmitkRigidRegistrationSelector1->SetMovingDimension(m_MovingDimension);
    m_Controls.qmitkRigidRegistrationSelector1->SetMovingNode(m_MovingNode);
    this->CheckCalculateEnabled();
  }
}

void QmitkRigidRegistrationView::MovingMaskImageChanged()
{
//   m_Controls.qmitkRigidRegistrationSelector1->SetMovingMaskNode(m_Controls.m_MovingMaskSelector->GetSelectedNode());
//   this->CheckCalculateEnabled();
}

void QmitkRigidRegistrationView::FixedMaskImageChanged()
{
//   m_Controls.qmitkRigidRegistrationSelector1->SetFixedMaskNode(m_Controls.m_FixedMaskSelector->GetSelectedNode());
//   this->CheckCalculateEnabled();
}

void QmitkRigidRegistrationView::Calculate()
{
  m_Controls.qmitkRigidRegistrationSelector1->SetFixedNode(m_FixedNode);
  m_Controls.qmitkRigidRegistrationSelector1->SetMovingNode(m_MovingNode);
  m_Controls.frame4->setEnabled(false);
  m_Controls.m_StopOptimization->setEnabled(true);
  m_Controls.qmitkRigidRegistrationSelector1->CalculateTransformation(((QmitkSliderNavigatorWidget*)m_Controls.timeSlider)->GetPos());
  m_Controls.m_StopOptimization->setEnabled(false);
  m_Controls.qmitkRigidRegistrationSelector1->StopOptimization(false);
  m_Controls.frame4->setEnabled(true);
}

void QmitkRigidRegistrationView::SetOptimizerValue( double value )
{
  m_Controls.m_OptimizerValueLCD->display(value);
}

void QmitkRigidRegistrationView::StopOptimizationClicked()
{
  m_Controls.qmitkRigidRegistrationSelector1->StopOptimization(true);
}

void QmitkRigidRegistrationView::UpdateTimestep()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRigidRegistrationView::ShowManualRegistrationFrame(bool show)
{
  if (show)
  {
    m_Controls.m_ManualFrame->show();
  }
  else
  {
    m_Controls.m_ManualFrame->hide();
  }
}

void QmitkRigidRegistrationView::SetImagesVisible(cherry::ISelection::ConstPointer selection)
{
  if (this->m_CurrentSelection->Size() == 0)
  {
    // show all images
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
    {
      if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(nodeIt->Value()->GetData())==NULL)
      {
        nodeIt->Value()->SetVisibility(true);
      }
    }
  }
  else
  {
    // hide all images
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
    {
      if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(nodeIt->Value()->GetData())==NULL)
      {
        nodeIt->Value()->SetVisibility(false);
      }
    }
  }
}
