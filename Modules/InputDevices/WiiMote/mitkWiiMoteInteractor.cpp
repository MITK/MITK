#include <mitkWiiMoteInteractor.h>

// mitk
#include <mitkInteractionConst.h>
#include <mitkWiiMoteAllDataEvent.h>
#include <mitkVector.h>
#include <mitkSurface.h>
#include <mitkGeometry3D.h>
#include <mitkRenderingManager.h>

#include <mitkBaseRenderer.h>
#include <mitkGlobalInteraction.h>
#include <vtkRenderer.h>
//#include <vtkPropRenderer.h>
#include <vtkCamera.h>
#include <mitkVector.h>
#include <vtkTransform.h>



mitk::WiiMoteInteractor::WiiMoteInteractor(const char* type, DataNode* dataNode)
: Interactor(type, dataNode)
, m_Test(0)
{
  CONNECT_ACTION(mitk::AcONWIIMOTEINPUT,OnWiiMoteInput);
}

mitk::WiiMoteInteractor::~WiiMoteInteractor()
{

}

bool mitk::WiiMoteInteractor::OnWiiMoteActivateButton(Action* action, const mitk::StateEvent* stateEvent)
{

return true;
}

bool mitk::WiiMoteInteractor::OnWiiMoteInput(Action* action, const mitk::StateEvent* stateEvent)
{
  const mitk::WiiMoteAllDataEvent* wiiMoteEvent; 
  try
  {
    wiiMoteEvent = dynamic_cast<const mitk::WiiMoteAllDataEvent*>(stateEvent->GetEvent());
  }
  catch(...)
  {
    MITK_ERROR << "Event is not wiimote event and could not be transformed\n";
  }
 

    float xValue = wiiMoteEvent->GetRollSpeed();
    //xValue /= 15*8;

    float yValue = wiiMoteEvent->GetPitchSpeed();
    //yValue /= 0.5*8;

    float zValue = wiiMoteEvent->GetYawSpeed();
    //zValue /= 21*8;

    //translation
    /*
    mitk::Vector3D movementVector;
    movementVector.SetElement(0,1);
    movementVector.SetElement(1,1);
    movementVector.SetElement(2,1);
    */
    
    //checking corresponding Data; has to be a surface or a subclass
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_DataNode->GetData());
    if ( surface == NULL )
    {
      MITK_WARN<<"Wiimote Interactor got wrong type of data! Aborting interaction!\n";
      return false;
    }
    Geometry3D* geometry = surface->GetUpdatedTimeSlicedGeometry()->GetGeometry3D( m_TimeStep );

    ////only if 3D rendering
    //const mitk::BaseRenderer* br = 
    //  mitk::GlobalInteraction::GetInstance()->GetFocus();
    //this->SetRenderer( br );
    //mitk::BaseRenderer::MapperSlotId id = 
    //  ((mitk::BaseRenderer*)(br))->GetMapperID();
    //if (id != mitk::BaseRenderer::Standard3D)
    //  return true;

    ////only if focused by the FocusManager
    //if (this->GetRenderer() != mitk::GlobalInteraction::GetInstance()->GetFocus())
    //  return true;

    ////pre-checking for safety
    //vtkRenderer* vtkRenderer = 
    //  ((mitk::VtkPropRenderer*)this->GetRenderer())->GetVtkRenderer();
    //if (vtkRenderer == NULL)
    //  return false;

    //vtkCamera* vtkCam = (vtkCamera*)vtkRenderer->GetActiveCamera();

    ////then the vector to which the camera is heading at (focalpoint)
    //mitk::Vector3D focalPoint;
    //double focalPointTemp[3];
    //vtkCam->GetFocalPoint(focalPointTemp);
    //focalPoint[0] = focalPointTemp[0];
    //focalPoint[1] = focalPointTemp[1]; 
    //focalPoint[2] = focalPointTemp[2]; 
    //mitk::Vector3D focalVector;
    //focalVector = focalPoint - camPosition;
    //focalVector.Normalize();

    //rotation
    vtkTransform *vtkTransform = vtkTransform::New();
    geometry->TransferItkToVtkTransform();//copy m_vtkMatrix to m_VtkIndexToWorldTransform
    //vtkTransform->SetMatrix(geometry->GetVtkTransform()->GetMatrix());//m_VtkIndexToWorldTransform as vtkLinearTransform*
    //Vector3D rotationVector = geometry->Get;

    Point3D center = geometry->GetOrigin();
    ScalarType angleA = wiiMoteEvent->GetRollSpeed();
    ScalarType angleB = wiiMoteEvent->GetPitchSpeed();
    ScalarType angleC = wiiMoteEvent->GetYawSpeed();

    vtkTransform->PostMultiply();
    vtkTransform->Translate(-center[0], -center[1], -center[2]);
    //vtkTransform->RotateWXYZ(angle, rotationVector[0], rotationVector[1], rotationVector[2]);
    vtkTransform->RotateX(angleA);
    vtkTransform->RotateY(angleB);
    vtkTransform->RotateZ(angleC);

    vtkTransform->Translate(center[0], center[1], center[2]);

    vtkTransform->PreMultiply();

    geometry->SetIndexToWorldTransformByVtkMatrix(vtkTransform->GetMatrix());
    geometry->Modified();
    vtkTransform->Delete();

    //geometry->Translate(movementVector);

    // indicate modification of data tree node
    m_DataNode->Modified();

    //update rendering
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();



  return true;
}

bool mitk::WiiMoteInteractor::OnWiiMoteReleaseButton(Action* action, const mitk::StateEvent* stateEvent)
{
return true;
}