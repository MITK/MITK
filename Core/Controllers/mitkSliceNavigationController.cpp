#include "mitkSliceNavigationController.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderWindow.h"
#include <itkCommand.h>

//##ModelId=3E189B1D008D
mitk::SliceNavigationController::SliceNavigationController() : m_ViewDirection(Transversal), m_BlockUpdate(false)
{
  itk::SimpleMemberCommand<SliceNavigationController>::Pointer sliceStepperChangedCommand, timeStepperChangedCommand;
  sliceStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
  timeStepperChangedCommand = itk::SimpleMemberCommand<SliceNavigationController>::New();
#ifdef WIN32
  sliceStepperChangedCommand->SetCallbackFunction(this, SliceNavigationController::SliceStepperChanged);
  timeStepperChangedCommand->SetCallbackFunction(this,  SliceNavigationController::TimeStepperChanged);
#else
  sliceStepperChangedCommand->SetCallbackFunction(this, &SliceNavigationController::SliceStepperChanged);
  timeStepperChangedCommand->SetCallbackFunction(this,  &SliceNavigationController::TimeStepperChanged);
#endif

  m_Slice->AddObserver(itk::ModifiedEvent(), sliceStepperChangedCommand);
  m_Time->AddObserver(itk::ModifiedEvent(),  timeStepperChangedCommand);
}

//##ModelId=3E189B1D00BF
mitk::SliceNavigationController::~SliceNavigationController()
{
}

//##ModelId=3DD524D7038C
const mitk::Geometry3D* mitk::SliceNavigationController::GetGeometry()
{
  return m_Geometry3D.GetPointer();
}

//##ModelId=3DD524F9001A
void mitk::SliceNavigationController::SetGeometry2D(const mitk::Geometry2D* aGeometry2D)
{
}

bool mitk::SliceNavigationController::AddRenderer(mitk::BaseRenderer* renderer)
{
  bool result = Superclass::AddRenderer(renderer);

  if((m_Renderer != NULL) && (m_Plane.IsNotNull()))
  {
    m_Renderer->SetWorldGeometry(m_Plane);
    m_Renderer->GetDisplayGeometry()->Fit();
  }

  return result;
}

//##ModelId=3E3AA1E20393
void mitk::SliceNavigationController::SetGeometry(const mitk::Geometry3D* aGeometry3D)
{
  m_Geometry3D = aGeometry3D;

  // compute bounding box with respect to first images geometry
  const mitk::BoundingBox::BoundsArrayType bounds = aGeometry3D->GetBoundingBox()->GetBounds();

  Vector3D dimensionVec;
  Vector3D  origin, right, bottom;
  if ( (m_ViewDirection == Transversal) || (m_ViewDirection == Original))
  {  
    origin = Vector3f(bounds[0],bounds[3],bounds[5]);
    right = Vector3f(bounds[1],bounds[3],bounds[5]);
    bottom = Vector3f(bounds[0],bounds[2],bounds[5]);
    dimensionVec.set(0,0,bounds[4]-bounds[5]);
  }
  else if (m_ViewDirection == Frontal)
  {  
    origin = Vector3f(bounds[0],bounds[2],bounds[4]);
    right = Vector3f(bounds[1],bounds[2],bounds[4]);
    bottom = Vector3f(bounds[0],bounds[2],bounds[5]);
    dimensionVec.set(0,bounds[3]-bounds[2],0);
  }
  // init sagittal view
  else 
  {  
    origin = Vector3f(bounds[0],bounds[2],bounds[4]);
    right = Vector3f(bounds[0],bounds[3],bounds[4]);
    bottom = Vector3f(bounds[0],bounds[2],bounds[5]);
    dimensionVec.set(bounds[1]-bounds[0],0,0);
  }

  m_Geometry3D->MMToUnits(dimensionVec, dimensionVec);

  m_NormalizedSliceDirection=dimensionVec; m_NormalizedSliceDirection.normalize();

  m_BlockUpdate = true;
  m_Slice->SetSteps((int)dimensionVec.length()+1.0);
  m_Slice->SetPos(0);
  m_BlockUpdate = false;

  // initialize the viewplane
  m_Plane = mitk::PlaneGeometry::New();

  //@FIXME: ohne den Pointer-Umweg meckert gcc  
  mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom);
  m_Plane->SetPlaneView(*view);

  if(m_Renderer != NULL)
  {
    m_Renderer->SetWorldGeometry(m_Plane);
    m_Renderer->GetDisplayGeometry()->Fit();
  }
}

//##ModelId=3DD524D7038C
void mitk::SliceNavigationController::SliceStepperChanged()
{
  if(!m_BlockUpdate)
  {
    if(m_Plane.IsNotNull())
    {
      mitk::PlaneView pv = m_Plane->GetPlaneView();
      pv.point += m_NormalizedSliceDirection*m_Slice->GetPos();

      mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
      plane->SetPlaneView(pv);

      if(m_Renderer != NULL)
      {
        m_Renderer->SetWorldGeometry(plane);
        //m_Renderer->GetRenderWindow()->updateGL();
        mitk::RenderWindow::UpdateAllInstances();
      }
    }
  }
}

void mitk::SliceNavigationController::TimeStepperChanged()
{
  if(!m_BlockUpdate)
  {
  }
}

