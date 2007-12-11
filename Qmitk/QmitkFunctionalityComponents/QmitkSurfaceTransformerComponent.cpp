/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-07-17 17:59:12 +0200 (Di, 17 Jul 2007) $
Version:   $Revision: 11316 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkSurfaceTransformerComponent.h"
#include "QmitkSurfaceTransformerComponentGUI.h"

#include <QmitkDataTreeComboBox.h>
#include "QmitkStdMultiWidget.h"

#include "mitkAction.h"
#include "mitkDataTreeFilterFunctions.h"
#include "mitkProperties.h"
#include "mitkOperation.h"
#include "mitkOperationEvent.h"
#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkRotationOperation.h"
#include "mitkSurface.h"
#include "mitkInteractionConst.h" 

//#include <vtkLinearTransform.h>
//#include <vtkMatrix4x4.h>
#include <vtkGeometryFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyData.h>
#include <vtkReflectionFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include "vtkTransform.h"
#include "vtkReverseSense.h"

#include <vtkTransform.h>
#include <vtkMatrixToLinearTransform.h>


#include <qgroupbox.h>
#include <qlineedit.h>
#include <qslider.h>

#include <vnl/vnl_math.h>




/***************       CONSTRUCTOR      ***************/
QmitkSurfaceTransformerComponent::QmitkSurfaceTransformerComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget * mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
m_SurfaceTransformerImageNode(NULL),
m_SurfaceTransformerComponentGUI(NULL),
m_SurfaceTransformerNodeExisting(false),
m_SurfaceNode(NULL),
m_TransformationMode(0),
m_MultiWidget(mitkStdMultiWidget),
m_Angle(5.0),
m_Distance(1.0),
m_Scale(0.9)
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("SurfaceTransformer");
  m_Node = it->Get();
  m_XTranslate = 0.0;
  m_YTranslate = 0.0;
  m_ZTranslate = 0.0;
}

/***************        DESTRUCTOR      ***************/
QmitkSurfaceTransformerComponent::~QmitkSurfaceTransformerComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkSurfaceTransformerComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
  m_Node = m_DataTreeIterator->Get();
}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkSurfaceTransformerComponent::SetSelectorVisibility(bool visibility)
{
  if(m_SurfaceTransformerComponentGUI)
  {
    m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(visibility);
  }
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkSurfaceTransformerComponent::GetImageContent()
{
  return (QGroupBox*) m_SurfaceTransformerComponentGUI->GetImageContent();
}


/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkSurfaceTransformerComponent::GetTreeNodeSelector()
{
  return m_SurfaceTransformerComponentGUI->GetTreeNodeSelector();
}

/****************** GET SURFACE NODE ******************/
mitk::DataTreeNode::Pointer QmitkSurfaceTransformerComponent::GetSurfaceNode()
{
  return m_SurfaceNode;
}

/****************** SET SURFACE NODE ******************/
void QmitkSurfaceTransformerComponent::SetSurfaceNode(mitk::DataTreeNode::Pointer node)
{
  m_SurfaceNode = node;
}

/***************       CONNECTIONS      ***************/
void QmitkSurfaceTransformerComponent::CreateConnections()
{
  if ( m_SurfaceTransformerComponentGUI )
  {
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceTransformerContent(bool)));     
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool))); 
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetMoveButton()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(TransformationModeMove(bool))); 
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetRotateButton()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(TransformationModeRotate(bool)));
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetScaleButton()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(TransformationModeScale(bool)));
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetMirrorButton()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(TransformationModeMirror(bool)));
  
    //connect TextLabels
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetXBox()),  SIGNAL(returnPressed()), (QObject*) this, SLOT(TransformXBox()));
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetYBox()),  SIGNAL(returnPressed()), (QObject*) this, SLOT(TransformYBox()));
    connect( (QObject*)(m_SurfaceTransformerComponentGUI->GetZBox()),  SIGNAL(returnPressed()), (QObject*) this, SLOT(TransformZBox()));
  }
}


void QmitkSurfaceTransformerComponent::TransformXBox()
{
  if(m_SurfaceTransformerComponentGUI->GetMoveButton()->isOn())
  {
  //std::cout<<"Move"<<std::endl;
  double xValue = m_SurfaceTransformerComponentGUI->GetXBox()->text().toDouble();
  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
  surface->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,xValue);
  m_XTranslate =  xValue;
  surface->UpdateOutputData();
  }
  else if(m_SurfaceTransformerComponentGUI->GetRotateButton()->isOn())
  {
    //mitk::OperationType OpROTATE;
    
    //get rotation angle from GUI
    double xValue = m_SurfaceTransformerComponentGUI->GetXBox()->text().toDouble();

    //get surface that shall be rotated
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
    mitk::Point3D geometryCenterPoint = surface->GetGeometry()->GetCenter();
    surface->CalculateBoundingBox();
    mitk::Point3D boundingBoxCenterPoint = surface->GetGeometry()->GetBoundingBox()->GetCenter();
    //surface->GetGeometry()->
    
    //get the center of the surface that shall be rotated
    mitk::Point3D surfaceCenter = surface->GetGeometry()->GetCenter();

    //define the rotationaxix
    mitk::Vector3D rotationaxis;   
    rotationaxis[0] =  1;
    rotationaxis[1] =  0;
    rotationaxis[2] =  0;

    /* calculate rotation angle in degrees */
    mitk::ScalarType angle = xValue;
    //mitk::ScalarType angle = atan2((mitk::ScalarType)rotationaxis.GetNorm(), (mitk::ScalarType) (xValue)) * (180/vnl_math::pi);
    
    /* create operation with center of rotation, angle and axis and send it to the geometry and Undo controller */
    mitk::RotationOperation* doOp = new mitk::RotationOperation(mitk::OpROTATE, surfaceCenter, rotationaxis, angle);

    /* execute the Operation */
    surface->GetGeometry()->ExecuteOperation(doOp);

    surface->UpdateOutputData();

     //mitk::RotationOperation *rotateOp = dynamic_cast<mitk::RotationOperation *>(operation);
     // if (rotateOp == NULL)
     // {
     //   mitk::StatusBar::GetInstance()->DisplayText("received wrong type of operation!See mitkAffineInteractor.cpp", 10000);
     //   return;
     // }
     // Vector3D rotationVector = rotateOp->GetVectorOfRotation();
     // Point3D center = rotateOp->GetCenterOfRotation();
     // ScalarType angle = rotateOp->GetAngleOfRotation();
     // angle = (angle < -360) ? -360 : angle;
     // angle = (angle >  360) ?  360 : angle;
     // vtktransform->PostMultiply();
     // vtktransform->Translate(-center[0], -center[1], -center[2]);
     // vtktransform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);
     // vtktransform->Translate(center[0], center[1], center[2]);
     // vtktransform->PreMultiply();




  //  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
  //  vtkTransform* trans = vtkTransform::New();

  //  double xValue = m_SurfaceTransformerComponentGUI->GetXBox()->text().toDouble();

  //  //trans = dynamic_cast<vtkTransform*>(surface->GetGeometry()->GetVtkTransform());
  //  
  //  //undo all former transformations
  //  trans->Identity();

  //  //get the center of the local coordinates
  //  mitk::Point3D centerpoint = surface->GetGeometry()->GetCenter();
  //  float centerX = centerpoint[0];
  //  float centerY = centerpoint[1];
  //  float centerZ = centerpoint[2];
  //  
  //  double cX = centerX;
  //  double cY = centerY;
  //  double cZ = centerZ;

  //  ////move the surface to the center
  //  //trans->GetMatrix()->SetElement(0,3,cX);
  //  //trans->GetMatrix()->SetElement(1,3,cY);
  //  //trans->GetMatrix()->SetElement(2,3,cZ);
  //  
  //  //rotate the surface
  //  vtkTransform* transRotX = trans->RotateX(xValue);
  //  vtkTransform* transCent = trans->RotateWXYZ(xValue, cX, cY, cZ);
  //  vtkTransform* transPoin = trans->Translate(cX, cY, cZ)


  //  trans = ((trans->RotateX(xValue)) - (trans->RotateWXYZ(xValue, cX, cY, cZ)) + (trans->Translate(cX, cY, cZ)));

  //  //RotateWXYZ (double angle, const float axis[3])
  //  //RotateWXYZ (double angle, double x, double y, double z)

  //  ////move the surface back from the center
  //  //trans->GetMatrix()->SetElement(0,3,-cX);
  //  //trans->GetMatrix()->SetElement(1,3,-cY);
  //  //trans->GetMatrix()->SetElement(2,3,-cZ);

  //  //translate the surface to the last given coordinates again (redo the undo translation)
  //  trans->GetMatrix()->SetElement(0,3,m_XTranslate);
  //  trans->GetMatrix()->SetElement(1,3,m_YTranslate);
  //  trans->GetMatrix()->SetElement(2,3,m_ZTranslate);

  //vtkMatrixToLinearTransform* maToLinTran;
  //maToLinTran = dynamic_cast<vtkMatrixToLinearTransform*>(surface->GetGeometry()->GetVtkTransform());
  //maToLinTran->SetMatrix( trans->GetMatrix());

  ////surface->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,m_XTranslate);
  ////surface->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,m_YTranslate);
  ////surface->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,m_ZTranslate);
  ////vtkTransform* transform = vtkTransform::New();
  ////transform->RotateX(xValue);
  ////Transform(transform);

  ////double xValue = m_SurfaceTransformerComponentGUI->GetXBox()->text().toDouble();
  //////mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
  ////vtkTransform* transform = vtkTransform::New();
  ////transform->RotateX(xValue);
  ////Transform(transform);
  ////mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
  ////surface->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,0.0);


  ////surface->GetGeometry()->GetVtkTransform()->RotateX(xValue);

  }
  else
  {
   std::cout<<"Weder Rotation noch Move ist angeschaltet"<<std::endl;
  }
  //  GetTumorNode()->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,x);
    //->GetVtkPolyData()->Get
}

void QmitkSurfaceTransformerComponent::TransformYBox()
{
  if(m_SurfaceTransformerComponentGUI->GetMoveButton()->isOn())
  {
  //std::cout<<"Move"<<std::endl;
  double yValue = m_SurfaceTransformerComponentGUI->GetYBox()->text().toDouble();
  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
  surface->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(1,3,yValue);
  m_YTranslate =  yValue;
  surface->UpdateOutputData();
  }
  else if(m_SurfaceTransformerComponentGUI->GetRotateButton()->isOn())
  {
       //get rotation angle from GUI
    double yValue = m_SurfaceTransformerComponentGUI->GetYBox()->text().toDouble();

    //get surface that shall be rotated
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
    mitk::Point3D geometryCenterPoint = surface->GetGeometry()->GetCenter();
    surface->CalculateBoundingBox();
    mitk::Point3D boundingBoxCenterPoint = surface->GetGeometry()->GetBoundingBox()->GetCenter();
    //surface->GetGeometry()->
    
    //get the center of the surface that shall be rotated
    mitk::Point3D surfaceCenter = surface->GetGeometry()->GetCenter();

    //define the rotationaxix
    mitk::Vector3D rotationaxis;   
    rotationaxis[0] =  0;
    rotationaxis[1] =  1;
    rotationaxis[2] =  0;

    /* calculate rotation angle in degrees */
    mitk::ScalarType angle = yValue;
    //mitk::ScalarType angle = atan2((mitk::ScalarType)rotationaxis.GetNorm(), (mitk::ScalarType) (xValue)) * (180/vnl_math::pi);
    
    /* create operation with center of rotation, angle and axis and send it to the geometry and Undo controller */
    mitk::RotationOperation* doOp = new mitk::RotationOperation(mitk::OpROTATE, surfaceCenter, rotationaxis, angle);

    /* execute the Operation */
    surface->GetGeometry()->ExecuteOperation(doOp);

    surface->UpdateOutputData();

  }
  else
  {
   std::cout<<"Weder Rotation noch Move ist angeschaltet"<<std::endl;
  }
  //  GetTumorNode()->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,x);
    //->GetVtkPolyData()->Get
}
void QmitkSurfaceTransformerComponent::TransformZBox()
{
  if(m_SurfaceTransformerComponentGUI->GetMoveButton()->isOn())
  {
  //std::cout<<"Move"<<std::endl;
  double zValue = m_SurfaceTransformerComponentGUI->GetZBox()->text().toDouble();
  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
  surface->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(2,3,zValue);
  m_ZTranslate =  zValue;
  surface->UpdateOutputData();
  }
  else if(m_SurfaceTransformerComponentGUI->GetRotateButton()->isOn())
  {
       //get rotation angle from GUI
    double zValue = m_SurfaceTransformerComponentGUI->GetZBox()->text().toDouble();

    //get surface that shall be rotated
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());
    mitk::Point3D geometryCenterPoint = surface->GetGeometry()->GetCenter();
    surface->CalculateBoundingBox();
    mitk::Point3D boundingBoxCenterPoint = surface->GetGeometry()->GetBoundingBox()->GetCenter();
    //surface->GetGeometry()->
    
    //get the center of the surface that shall be rotated
    mitk::Point3D surfaceCenter = surface->GetGeometry()->GetCenter();

    //define the rotationaxix
    mitk::Vector3D rotationaxis;   
    rotationaxis[0] =  0;
    rotationaxis[1] =  0;
    rotationaxis[2] =  1;

    /* calculate rotation angle in degrees */
    mitk::ScalarType angle = zValue;
    //mitk::ScalarType angle = atan2((mitk::ScalarType)rotationaxis.GetNorm(), (mitk::ScalarType) (xValue)) * (180/vnl_math::pi);
    
    /* create operation with center of rotation, angle and axis and send it to the geometry and Undo controller */
    mitk::RotationOperation* doOp = new mitk::RotationOperation(mitk::OpROTATE, surfaceCenter, rotationaxis, angle);

    /* execute the Operation */
    surface->GetGeometry()->ExecuteOperation(doOp);

    surface->UpdateOutputData();

  }
  else
  {
   std::cout<<"Weder Rotation noch Move ist angeschaltet"<<std::endl;
  }
  //  GetTumorNode()->GetData()->GetGeometry()->GetVtkTransform()->GetMatrix()->SetElement(0,3,x);
    //->GetVtkPolyData()->Get
}

/************** TRANSFORMATION MODE MOVE **************/
void QmitkSurfaceTransformerComponent::TransformationModeMove(bool toggleflag)
{
  if(toggleflag == true)
  {
    m_SurfaceTransformerComponentGUI->GetRotateButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetScaleButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetMirrorButton()->setOn(false);


    if(m_SurfaceNode)
    {
      //activate the connection between the arrow-keys and the translation-mehtods.
      m_Left->setEnabled(true);
      m_Right->setEnabled(true);
      m_Up->setEnabled(true);
      m_Down->setEnabled(true);
      m_Forward->setEnabled(true);
      m_Backward->setEnabled(true);

      //deactivate the connction between the keys and the rotation-methods
      m_RotX->setEnabled(false);
      m_RotInvX->setEnabled(false);
      m_RotY->setEnabled(false);
      m_RotInvY->setEnabled(false);
      m_RotZ->setEnabled(false);
      m_RotInvZ->setEnabled(false);

      //deactivate the connction between the keys and the scale-methods
      m_ScaleUp->setEnabled(false);
      m_ScaleDown->setEnabled(false);

      //deactivate the connection between the XYZ-Keys and the mirror-methods
      m_MirrorX->setEnabled(false);
      m_MirrorY->setEnabled(false);
      m_MirrorZ->setEnabled(false);
    }
  }
}

/************* TRANSFORMATION MODE ROTATE *************/
void QmitkSurfaceTransformerComponent::TransformationModeRotate(bool toggleflag)
{
  if(toggleflag == true)
  {
    m_SurfaceTransformerComponentGUI->GetMoveButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetScaleButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetMirrorButton()->setOn(false);

    //activate the connection between the arrow-keys and the rotation-mehtods.
    m_RotX->setEnabled(true);
    m_RotInvX->setEnabled(true);
    m_RotY->setEnabled(true);
    m_RotInvY->setEnabled(true);
    m_RotZ->setEnabled(true);
    m_RotInvZ->setEnabled(true);

    //deactivate the connction between the keys and the translation-methods
    m_Left->setEnabled(false);
    m_Right->setEnabled(false);
    m_Up->setEnabled(false);
    m_Down->setEnabled(false);
    m_Forward->setEnabled(false);
    m_Backward->setEnabled(false);

    //deactivate the connction between the keys and the scale-methods
    m_ScaleUp->setEnabled(false);
    m_ScaleDown->setEnabled(false);

    //deactivate the connection between the XYZ-Keys and the mirror-methods
    m_MirrorX->setEnabled(false);
    m_MirrorY->setEnabled(false);
    m_MirrorZ->setEnabled(false);
  }
}

/************* TRANSFORMATION MODE ROTATE *************/
void QmitkSurfaceTransformerComponent::TransformationModeScale(bool toggleflag)
{
  if(toggleflag == true)
  {
    m_SurfaceTransformerComponentGUI->GetMoveButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetRotateButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetMirrorButton()->setOn(false);

    //deactivate the connction between the keys and the scale-methods
    m_ScaleUp->setEnabled(true);
    m_ScaleDown->setEnabled(true);

    //activate the connection between the arrow-keys and the rotation-mehtods.
    m_RotX->setEnabled(false);
    m_RotInvX->setEnabled(false);
    m_RotY->setEnabled(false);
    m_RotInvY->setEnabled(false);
    m_RotZ->setEnabled(false);
    m_RotInvZ->setEnabled(false);

    //deactivate the connction between the keys and the translation-methods
    m_Left->setEnabled(false);
    m_Right->setEnabled(false);
    m_Up->setEnabled(false);
    m_Down->setEnabled(false);
    m_Forward->setEnabled(false);
    m_Backward->setEnabled(false);

    //deactivate the connection between the XYZ-Keys and the mirror-methods
    m_MirrorX->setEnabled(false);
    m_MirrorY->setEnabled(false);
    m_MirrorZ->setEnabled(false);
  }
}

/************* TRANSFORMATION MODE ROTATE *************/
void QmitkSurfaceTransformerComponent::TransformationModeMirror(bool toggleflag)
{
  if(toggleflag == true)
  {
    m_SurfaceTransformerComponentGUI->GetMoveButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetRotateButton()->setOn(false);
    m_SurfaceTransformerComponentGUI->GetScaleButton()->setOn(false);

    //activate the connection between the XYZ-Keys and the mirror-methods
    m_MirrorX->setEnabled(true);
    m_MirrorY->setEnabled(true);
    m_MirrorZ->setEnabled(true);


    //deactivate the connction between the keys and the scale-methods
    m_ScaleUp->setEnabled(false);
    m_ScaleDown->setEnabled(false);

    //deactivate the connection between the arrow-keys and the rotation-mehtods.
    m_RotX->setEnabled(false);
    m_RotInvX->setEnabled(false);
    m_RotY->setEnabled(false);
    m_RotInvY->setEnabled(false);
    m_RotZ->setEnabled(false);
    m_RotInvZ->setEnabled(false);

    //deactivate the connction between the keys and the translation-methods
    m_Left->setEnabled(false);
    m_Right->setEnabled(false);
    m_Up->setEnabled(false);
    m_Down->setEnabled(false);
    m_Forward->setEnabled(false);
    m_Backward->setEnabled(false);
  }
}

/*****************************************************************************************************************************/
/****************************************     TRANSLATION      ***************************************************************/
/*****************************************************************************************************************************/

/*************          MOVE LEFT         *************/
void QmitkSurfaceTransformerComponent::MoveLeft()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Translate( m_Distance, 0.0, 0.0 );
    Transform(transform);
  }
}                     

/*************         MOVE RIGHT         *************/
void QmitkSurfaceTransformerComponent::MoveRight()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Translate( -m_Distance, 0.0, 0.0 );
    Transform(transform);
  }
}                    

/*************          MOVE UP           *************/
void QmitkSurfaceTransformerComponent::MoveUp()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Translate( 0.0, m_Distance, 0.0 );
    Transform(transform);
  }
}                      

/*************         MOVE DOWN          *************/
void QmitkSurfaceTransformerComponent::MoveDown()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Translate( 0.0, -m_Distance, 0.0 );
    Transform(transform);
  }
}  

/*************       MOVE FOREWARD        *************/
void QmitkSurfaceTransformerComponent::MoveForward()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Translate( 0.0, 0.0, m_Distance );
    Transform(transform);
  }
}                      

/*************        MOVE BACKWARD       *************/
void QmitkSurfaceTransformerComponent::MoveBackward()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Translate( 0.0, 0.0, -m_Distance );
    Transform(transform);
  }
} 


/*****************************************************************************************************************************/
/****************************************       ROTATION       ***************************************************************/
/*****************************************************************************************************************************/

///*************    ROTATE ARROUND X-AXIS   *************/
void QmitkSurfaceTransformerComponent::RotX()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->RotateX( m_Angle );

    Transform(transform);
  }
}

/*********** ROTATE INVERSE ARROUND X-AXIS ************/
void QmitkSurfaceTransformerComponent::RotInvX()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->RotateX( -m_Angle );

    Transform(transform);
  }
}

/*************    ROTATE ARROUND Y-AXIS   *************/
void QmitkSurfaceTransformerComponent::RotY()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->RotateY( m_Angle );

    Transform(transform);
  }
}

/*********** ROTATE INVERSE ARROUND Y-AXIS ************/
void QmitkSurfaceTransformerComponent::RotInvY()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->RotateY( -m_Angle );

    Transform(transform);
  }
}

/*************    ROTATE ARROUND Z-AXIS   *************/
void QmitkSurfaceTransformerComponent::RotZ()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->RotateZ( m_Angle );

    Transform(transform);
  }
}

/*********** ROTATE INVERSE ARROUND Z-AXIS ************/
void QmitkSurfaceTransformerComponent::RotInvZ()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->RotateZ( -m_Angle );

    Transform(transform);
  }
}

/*****************************************************************************************************************************/
/****************************************         SCALE        ***************************************************************/
/*****************************************************************************************************************************/

/*************           SCALE UP         *************/
void QmitkSurfaceTransformerComponent::ScaleUp()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Scale(  1/m_Scale, 1/m_Scale, 1/m_Scale );
    Transform(transform);
  }
}  

/*************          SCALE DOWN        *************/
void QmitkSurfaceTransformerComponent::ScaleDown()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Scale( m_Scale, m_Scale, m_Scale );

    Transform(transform);
  }
} 


/*****************************************************************************************************************************/
/****************************************        MIRROR        ***************************************************************/
/*****************************************************************************************************************************/

/*************           MIRROR X         *************/
void QmitkSurfaceTransformerComponent::MirrorX()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Scale(  -1, 1, 1);

    vtkTransformPolyDataFilter* transformFilter = vtkTransformPolyDataFilter::New();
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());


    transformFilter->SetInput( surface->GetVtkPolyData() );  
    transformFilter->SetTransform( transform );

    mitk::DataTreeNode::Pointer transformNode = mitk::DataTreeNode::New();
    vtkPolyData* transformed = transformFilter->GetOutput();

    vtkReverseSense *reverse=vtkReverseSense ::New();
    reverse->SetInput(transformed);

    mitk::Surface::Pointer tf = dynamic_cast<mitk::Surface*>( m_SurfaceNode->GetData() );
    tf->SetVtkPolyData(reverse->GetOutput(), 0);

    m_SurfaceNode->SetData(tf);

    RepaintRenderWindows();

    std::ostringstream buffer;
    QString surfaceDataName = m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->currentText();
    buffer << surfaceDataName.ascii();
    std::string surfaceNodeName = "Mirror_" + buffer.str();
    m_SurfaceNode->SetProperty("name", new mitk::StringProperty(surfaceNodeName));
  }
}  

/*************           MIRROR Y         *************/
void QmitkSurfaceTransformerComponent::MirrorY()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Scale(  1, -1, 1);

    vtkTransformPolyDataFilter* transformFilter = vtkTransformPolyDataFilter::New();
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());


    transformFilter->SetInput( surface->GetVtkPolyData() );  
    transformFilter->SetTransform( transform );

    mitk::DataTreeNode::Pointer transformNode = mitk::DataTreeNode::New();
    vtkPolyData* transformed = transformFilter->GetOutput();

    vtkReverseSense *reverse=vtkReverseSense ::New();
    reverse->SetInput(transformed);

    mitk::Surface::Pointer tf = dynamic_cast<mitk::Surface*>( m_SurfaceNode->GetData() );
    tf->SetVtkPolyData(reverse->GetOutput(), 0);

    m_SurfaceNode->SetData(tf);

    RepaintRenderWindows();

    std::ostringstream buffer;
    QString surfaceDataName = m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->currentText();
    buffer << surfaceDataName.ascii();
    std::string surfaceNodeName = "Mirror_" + buffer.str();
    m_SurfaceNode->SetProperty("name", new mitk::StringProperty(surfaceNodeName));
  }
} 

/*************           MIRROR Z         *************/
void QmitkSurfaceTransformerComponent::MirrorZ()
{
  if(m_SurfaceNode)
  {
    vtkTransform* transform = vtkTransform::New();
    transform->Scale(  1, 1, -1);

    vtkTransformPolyDataFilter* transformFilter = vtkTransformPolyDataFilter::New();
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());


    transformFilter->SetInput( surface->GetVtkPolyData() );  
    transformFilter->SetTransform( transform );

    mitk::DataTreeNode::Pointer transformNode = mitk::DataTreeNode::New();
    vtkPolyData* transformed = transformFilter->GetOutput();

    vtkReverseSense *reverse=vtkReverseSense ::New();
    reverse->SetInput(transformed);

    mitk::Surface::Pointer tf = dynamic_cast<mitk::Surface*>( m_SurfaceNode->GetData() );
    tf->SetVtkPolyData(reverse->GetOutput(), 0);

    m_SurfaceNode->SetData(tf);

    RepaintRenderWindows();

    std::ostringstream buffer;
    QString surfaceDataName = m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->currentText();
    buffer << surfaceDataName.ascii();
    std::string surfaceNodeName = "Mirror_" + buffer.str();
    m_SurfaceNode->SetProperty("name", new mitk::StringProperty(surfaceNodeName));
  }
} 

/*************          TRANSFORM         *************/
void QmitkSurfaceTransformerComponent::Transform(vtkTransform* transform)
{
  vtkTransformPolyDataFilter* transformFilter = vtkTransformPolyDataFilter::New();
  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SurfaceNode->GetData());

  
  transformFilter->SetInput( surface->GetVtkPolyData() );  
  transformFilter->SetTransform( transform );

  mitk::DataTreeNode::Pointer transformNode = mitk::DataTreeNode::New();
  vtkPolyData* transformed = transformFilter->GetOutput();
  mitk::Surface::Pointer tf = dynamic_cast<mitk::Surface*>( m_SurfaceNode->GetData() );
  tf->SetVtkPolyData(transformed, 0);
  m_SurfaceNode->SetData(tf);

  RepaintRenderWindows();
}

/*************    REPAINT RENDERWINDOWS   *************/
void QmitkSurfaceTransformerComponent::RepaintRenderWindows()
{
  m_MultiWidget->GetRenderWindow1()->repaint();
  m_MultiWidget->GetRenderWindow2()->repaint();
  m_MultiWidget->GetRenderWindow3()->repaint();
  m_MultiWidget->GetRenderWindow4()->repaint();
}


/***************     IMAGE SELECTED     ***************/
void QmitkSurfaceTransformerComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_SurfaceTransformerComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->GetFilter())
    {
      if(imageIt)
      {
        currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
      }
    }
  }
  if(currentItem)
  {
    currentItem->SetSelected(true);
  }
  if(m_SurfaceTransformerComponentGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(m_SelectedItem);
    }
  }
  m_Node = const_cast<mitk::DataTreeNode*>(m_SelectedItem->GetNode());
  m_SurfaceNode = m_Node;
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkSurfaceTransformerComponent::CreateControlWidget(QWidget* parent)
{
  m_SurfaceTransformerComponentGUI = new QmitkSurfaceTransformerComponentGUI(parent);
  m_GUI = m_SurfaceTransformerComponentGUI;

  m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());

  m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  m_SurfaceTransformerComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataType<mitk::Surface>());

  if(m_ShowSelector)
  {
    m_SurfaceTransformerComponentGUI->GetImageContent()->setShown(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->isChecked());
  }
  else
  {
    m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
  }


  //create connection between keys and translation methods

  m_Left = new QAccel(m_GUI);                               // create accels for multiWidget
  m_Left->connectItem( m_Left->insertItem(Qt::Key_Left),    // adds Qt::Key_Left accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MoveLeft()));                                      // MoveLeft() slot

  m_Right = new QAccel(m_GUI);                              // create accels for multiWidget
  m_Right->connectItem( m_Right->insertItem(Qt::Key_Right), // adds Qt::Key_Right accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MoveRight()));                                     // MoveRight() slot

  m_Up = new QAccel(m_GUI);                                 // create accels for multiWidget
  m_Up->connectItem( m_Up->insertItem(Qt::Key_Up),          // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MoveForward()));                                   // MoveUp() slot

  m_Down = new QAccel(m_GUI);                               // create accels for multiWidget
  m_Down->connectItem( m_Down->insertItem(Qt::Key_Down),    // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MoveBackward()));                                  // MoveDown() slot

  m_Forward = new QAccel(m_GUI);                            // create accels for multiWidget
  m_Forward->connectItem( m_Forward->insertItem(Qt::Key_A), // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MoveUp()));                                        // MoveForward() slot

  m_Backward = new QAccel(m_GUI);                           // create accels for multiWidget
  m_Backward->connectItem( m_Backward->insertItem(Qt::Key_S), // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MoveDown()));                                      // MoveBackward() slot



  //create connection between keys and rotation methods

  m_RotX = new QAccel(m_GUI);                               // create accels for multiWidget
  m_RotX->connectItem( m_RotX->insertItem(Qt::Key_Left),    // adds Qt::Key_Left accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(RotX()));                                          // RotY() slot

  m_RotInvX = new QAccel(m_GUI);                            // create accels for multiWidget
  m_RotInvX->connectItem( m_RotInvX->insertItem(Qt::Key_Right), // adds Qt::Key_Right accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(RotInvX()));                                       // RotInvX() slot

  m_RotY = new QAccel(m_GUI);                               // create accels for multiWidget
  m_RotY->connectItem( m_RotY->insertItem(Qt::Key_Up),      // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(RotY()));                                          // RotY() slot

  m_RotInvY = new QAccel(m_GUI);                            // create accels for multiWidget
  m_RotInvY->connectItem( m_RotInvY->insertItem(Qt::Key_Down), // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(RotInvY()));                                       // RotInvY() slot

  m_RotZ = new QAccel(m_GUI);                               // create accels for multiWidget
  m_RotZ->connectItem( m_RotZ->insertItem(Qt::Key_A),       // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(RotZ()));                                          // RotZ() slot

  m_RotInvZ = new QAccel(m_GUI);                            // create accels for multiWidget
  m_RotInvZ->connectItem( m_RotInvZ->insertItem(Qt::Key_S), // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(RotInvZ()));                                       // RotInvZ() slot


  //create connection between keys and scale methods

  m_ScaleUp = new QAccel(m_GUI);                            // create accels for multiWidget
  m_ScaleUp->connectItem( m_ScaleUp->insertItem(Qt::Key_Up), // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(ScaleUp()));                                       // RotY() slot

  m_ScaleDown = new QAccel(m_GUI);                          // create accels for multiWidget
  m_ScaleDown->connectItem( m_ScaleDown->insertItem(Qt::Key_Down), // adds SHIFT+ALT accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(ScaleDown()));                                     // RotY() slot


  //create connection between keys and mirror methods

  m_MirrorX = new QAccel(m_GUI);                            // create accels for multiWidget
  m_MirrorX->connectItem( m_MirrorX->insertItem(Qt::Key_X), // adds Key_X accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MirrorX()));                                       // MirrorX() slot

  m_MirrorY = new QAccel(m_GUI);                            // create accels for multiWidget
  m_MirrorY->connectItem( m_MirrorY->insertItem(Qt::Key_Y), // adds Key_Y accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MirrorY()));                                       // MirrorY() slot

  m_MirrorZ = new QAccel(m_GUI);                            // create accels for multiWidget
  m_MirrorZ->connectItem( m_MirrorZ->insertItem(Qt::Key_Z), // adds Key_Y accelerator
    this,                                                   // connected to SurfaceCreator
    SLOT(MirrorZ()));                                       // MirrorZ() slot

  /*  see list of Keys at http://doc.trolltech.com/3.3/qt.html#Key-enum */

  return m_SurfaceTransformerComponentGUI;
}





/***************      SET DISTANCE      ***************/
void QmitkSurfaceTransformerComponent::SetDistance(double distance)
{
  m_Distance = distance;
}

/***************      GET DISTANCE      ***************/
double QmitkSurfaceTransformerComponent::GetDistance()
{
  return m_Distance;
}

/***************       SET ANGLE        ***************/
void QmitkSurfaceTransformerComponent::SetAngle(double angle)
{
  m_Angle = angle;
}

/***************       GET ANGLE        ***************/
double QmitkSurfaceTransformerComponent::GetAngle()
{
  return m_Angle;
}


/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkSurfaceTransformerComponent::GetContentContainer()
{
  return m_SurfaceTransformerComponentGUI->GetContainerContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkSurfaceTransformerComponent::GetMainCheckBoxContainer()
{
  return m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox();
}

/***************        ACTIVATED       ***************/
void QmitkSurfaceTransformerComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Activated();
  } 

  m_SurfaceTransformerComponentGUI->GetMoveButton()->setOn(false);
  m_SurfaceTransformerComponentGUI->GetRotateButton()->setOn(false);

  m_Left->setEnabled(false);
  m_Right->setEnabled(false);
  m_Up->setEnabled(false);
  m_Down->setEnabled(false);
  m_Forward->setEnabled(false);
  m_Backward->setEnabled(false);

  m_RotX->setEnabled(false);
  m_RotInvX->setEnabled(false);
  m_RotY->setEnabled(false);
  m_RotInvY->setEnabled(false);
  m_RotZ->setEnabled(false);
  m_RotInvZ->setEnabled(false);

  m_ScaleUp->setEnabled(false);
  m_ScaleDown->setEnabled(false);

  m_MirrorX->setEnabled(false);
  m_MirrorY->setEnabled(false);
  m_MirrorZ->setEnabled(false);

}

/***************       DEACTIVATED      ***************/
void QmitkSurfaceTransformerComponent::Deactivated()
{
  QmitkBaseFunctionalityComponent::Deactivated();
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  } 

  m_Left->setEnabled(false);
  m_Right->setEnabled(false);
  m_Up->setEnabled(false);
  m_Down->setEnabled(false);
  m_Forward->setEnabled(false);
  m_Backward->setEnabled(false);

  m_RotX->setEnabled(false);
  m_RotInvX->setEnabled(false);
  m_RotY->setEnabled(false);
  m_RotInvY->setEnabled(false);
  m_RotZ->setEnabled(false);
  m_RotInvZ->setEnabled(false);

  m_ScaleUp->setEnabled(false);
  m_ScaleDown->setEnabled(false);

  m_MirrorX->setEnabled(false);
  m_MirrorY->setEnabled(false);
  m_MirrorZ->setEnabled(false);
}


///************ SHOW SurfaceTransformer CONTENT ***********/
void QmitkSurfaceTransformerComponent::ShowSurfaceTransformerContent(bool)
{
  m_SurfaceTransformerComponentGUI->GetContainerContent()->setShown(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()->isChecked());

  if(m_ShowSelector)
  {
    m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(m_SurfaceTransformerComponentGUI->GetSurfaceTransformerFinderGroupBox()->isChecked());
  }
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkSurfaceTransformerComponent::ShowImageContent(bool)
{
  m_SurfaceTransformerComponentGUI->GetImageContent()->setShown(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->isChecked());

  if(m_ShowSelector)
  {
    m_SurfaceTransformerComponentGUI->GetImageContent()->setShown(m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->isChecked());
  }
  else
  {
    m_SurfaceTransformerComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
  }
}


///***************  DELETE SurfaceTransformerD NODE  **************/
void QmitkSurfaceTransformerComponent::DeleteSurfaceTransformerNode()
{
  if(m_SurfaceTransformerImageNode)
  {
    mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
    while ( !iteratorClone->IsAtEnd() )
    {
      mitk::DataTreeNode::Pointer node = iteratorClone->Get();

      std::string name;
      node->GetName(name);

      if(name == "SurfaceTransformer image")
      {
        iteratorClone->Disconnect();
        m_SurfaceTransformerNodeExisting = false;
      }
      ++iteratorClone;
    }
  }
}
