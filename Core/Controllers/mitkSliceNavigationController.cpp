#include "mitkSliceNavigationController.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderWindow.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include <itkCommand.h>

//##ModelId=3E189B1D008D
mitk::SliceNavigationController::SliceNavigationController() : m_ViewDirection(Transversal), m_BlockUpdate(false), m_TimeSlicedWorldGeometry(NULL), m_WorldGeometry(NULL)
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
  ConnectGeometryEvents(this);
}

//##ModelId=3E189B1D00BF
mitk::SliceNavigationController::~SliceNavigationController()
{

}

bool mitk::SliceNavigationController::AddRenderer(mitk::BaseRenderer* renderer)
{
  bool result = Superclass::AddRenderer(renderer);

  if((m_Renderer != NULL) && (m_TimeSlicedWorldGeometry.IsNotNull()))
  {
    m_Renderer->SetWorldGeometry(m_TimeSlicedWorldGeometry);
    m_Renderer->SetSlice(m_Slice->GetPos());
    m_Renderer->SetTimeStep(m_Time->GetPos());
    m_Renderer->GetDisplayGeometry()->Fit();
  }

  return result;
}

void mitk::SliceNavigationController::Update()
{
  if(m_LastUpdateTime<GetMTime())
  {
    m_LastUpdateTime = GetMTime();

    if(m_WorldGeometry.IsNotNull())
    {
      // compute bounding box with respect to first images geometry
      //mitk::BoundingBox::ConstPointer boundingBox = aGeometry3D->GetBoundingBox();
      //const_cast<mitk::BoundingBox*>(boundingBox.GetPointer())->ComputeBoundingBox();
      //const mitk::BoundingBox::BoundsArrayType bounds = boundingBox->GetBounds();
      const mitk::BoundingBox::BoundsArrayType bounds = m_WorldGeometry->GetBoundingBox()->GetBounds();

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

      m_WorldGeometry->MMToUnits(dimensionVec, dimensionVec);

      m_BlockUpdate = true;
      m_Slice->SetSteps((int)dimensionVec.length()+1.0);
      m_Slice->SetPos(0);
      m_BlockUpdate = false;

      // initialize the viewplane
      mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();

      //@FIXME: ohne den Pointer-Umweg meckert gcc  
      mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom);
      planegeometry->SetPlaneView(*view);

      mitk::SlicedGeometry3D::Pointer slicedWorldGeometry=mitk::SlicedGeometry3D::New();
      slicedWorldGeometry->Initialize(m_Slice->GetSteps());

      slicedWorldGeometry->SetGeometry2D(planegeometry, 0);
      slicedWorldGeometry->SetDirectionVector(dimensionVec);
      Vector3D spacing(1.0,1.0,1.0); 
      slicedWorldGeometry->SetSpacing(spacing);
      slicedWorldGeometry->SetEvenlySpaced();

      // initialize TimeSlicedGeometry
      m_TimeSlicedWorldGeometry = TimeSlicedGeometry::New();
      const TimeSlicedGeometry* worldTimeSlicedGeometry = dynamic_cast<const TimeSlicedGeometry*>(m_WorldGeometry.GetPointer());
      if(worldTimeSlicedGeometry==NULL)
      {
        m_TimeSlicedWorldGeometry->Initialize(1);
      }
      else
      {
        m_TimeSlicedWorldGeometry->Initialize(worldTimeSlicedGeometry->GetTimeSteps());
        //@todo implement for non-evenly-timed geometry!
        m_TimeSlicedWorldGeometry->SetEvenlyTimed();
        slicedWorldGeometry->SetTimeBoundsInMS(worldTimeSlicedGeometry->GetGeometry3D(0)->GetTimeBoundsInMS());
      }
      m_TimeSlicedWorldGeometry->SetGeometry3D(slicedWorldGeometry, 0);
    }
  }

  //Send the geometry. Do this even if nothing was changed, because maybe Update() was only called to 
  //re-send the old geometry.
  if(m_Renderer != NULL)
  {
    m_Renderer->SetWorldGeometry(m_TimeSlicedWorldGeometry);
    m_Renderer->SetSlice(m_Slice->GetPos());
    m_Renderer->SetTimeStep(m_Time->GetPos());
    m_Renderer->GetDisplayGeometry()->Fit();

    InvokeEvent(GeometryTimeEvent(m_TimeSlicedWorldGeometry, m_Time->GetPos()));
    InvokeEvent(GeometrySliceEvent(m_TimeSlicedWorldGeometry, m_Slice->GetPos()));
  }
}

//##ModelId=3DD524D7038C
void mitk::SliceNavigationController::SliceStepperChanged()
{
  if(!m_BlockUpdate)
  {
    if(m_TimeSlicedWorldGeometry.IsNotNull())
    {
      if(m_Renderer != NULL)
      {
        m_Renderer->SetWorldGeometry(m_TimeSlicedWorldGeometry);
        m_Renderer->SetSlice(m_Slice->GetPos());
        InvokeEvent(GeometrySliceEvent(m_TimeSlicedWorldGeometry, m_Slice->GetPos()));
        mitk::RenderWindow::UpdateAllInstances();
      }
    }
  }
}

void mitk::SliceNavigationController::TimeStepperChanged()
{
  if(!m_BlockUpdate)
  {
    if(m_TimeSlicedWorldGeometry.IsNotNull())
    {
      if(m_Renderer != NULL)
      {
        m_Renderer->SetWorldGeometry(m_TimeSlicedWorldGeometry);
        m_Renderer->SetTimeStep(m_Time->GetPos());
        InvokeEvent(GeometryTimeEvent(m_TimeSlicedWorldGeometry, m_Time->GetPos()));
        mitk::RenderWindow::UpdateAllInstances();
      }
    }
  }
}

void mitk::SliceNavigationController::SetGeometrySlice(const itk::EventObject & geometryTimeEvent)
{
}

void mitk::SliceNavigationController::SetGeometryTime(const itk::EventObject & geometryTimeEvent)
{
}
