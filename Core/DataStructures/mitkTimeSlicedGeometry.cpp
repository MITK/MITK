#include "mitkTimeSlicedGeometry.h"
#include <vtkTransform.h>

mitk::Geometry3D* mitk::TimeSlicedGeometry::GetGeometry3D(int t) const
{
  mitk::Geometry3D::Pointer geometry3d = NULL;
  if(IsValidTime(t))
  {
    geometry3d = m_Geometry3Ds[t];
    //if (a) we don't have a Geometry3D stored for the requested time, 
    //(b) m_EvenlyTimed is activated and (c) the first geometry (t=0) 
    //is set, then we clone the geometry and set the m_TimeBoundsInMS accordingly.
    if((m_EvenlyTimed) && (geometry3d.IsNull()))
    {
      const Geometry3D* firstgeometry=m_Geometry3Ds[0].GetPointer();

      assert(firstgeometry != NULL);

      mitk::Geometry3D::Pointer requestedgeometry;
      requestedgeometry = static_cast<Geometry3D*>(firstgeometry->Clone().GetPointer());

      TimeBounds timebounds = requestedgeometry->GetTimeBoundsInMS();
      if(timebounds[1]<ScalarTypeNumericTraits::max())
      {
        mitk::ScalarType later = (timebounds[1]-timebounds[0])*t;
        timebounds[0]+=later; timebounds[1]+=later;
        requestedgeometry->SetTimeBoundsInMS(timebounds);
      }

      geometry3d = requestedgeometry;
      m_Geometry3Ds[t] = geometry3d;
    }
  }
  else
    return NULL;
  return geometry3d;
}

const mitk::TimeBounds& mitk::TimeSlicedGeometry::GetTimeBoundsInMS() const
{
  TimeBounds timebounds;

  mitk::Geometry3D::Pointer geometry3d;

  geometry3d = m_Geometry3Ds[0];
  assert(geometry3d.IsNotNull());
  timebounds[0] = geometry3d->GetTimeBoundsInMS()[0];

  geometry3d = GetGeometry3D(m_TimeSteps-1);
  assert(geometry3d.IsNotNull());
  timebounds[1]=geometry3d->GetTimeBoundsInMS()[1];

  m_TimeBoundsInMS = timebounds;

  return m_TimeBoundsInMS;
}

bool mitk::TimeSlicedGeometry::SetGeometry3D(mitk::Geometry3D* geometry3D, int t)
{
  if(IsValidTime(t))
  {
    m_Geometry3Ds[t]=geometry3D;
    return true;
  }
  return false;
}

int mitk::TimeSlicedGeometry::MSToTimeStep(mitk::ScalarType time_in_ms) const
{
  assert(m_EvenlyTimed);
  {
    if(time_in_ms < m_TimeBoundsInMS[0])
      return -1;
    if(time_in_ms >= m_TimeBoundsInMS[1])
      return m_TimeSteps;
    if(m_TimeBoundsInMS[0]==m_TimeBoundsInMS[1])
      return 0;
    if((m_TimeBoundsInMS[0]>-ScalarTypeNumericTraits::max()) && (m_TimeBoundsInMS[1]<ScalarTypeNumericTraits::max()))
      return (int) (time_in_ms - m_TimeBoundsInMS[0])/(m_TimeBoundsInMS[1]-m_TimeBoundsInMS[0])*m_TimeSteps;
    return 0;
  }
  return 0;
}

mitk::ScalarType mitk::TimeSlicedGeometry::TimeStepToMS(int timestep) const
{
  assert(m_EvenlyTimed);
  if(IsValidTime(timestep)==false)
    return ScalarTypeNumericTraits::max();
  return ((mitk::ScalarType)timestep)/m_TimeSteps*(m_TimeBoundsInMS[1]-m_TimeBoundsInMS[0])+m_TimeBoundsInMS[0];
}

void mitk::TimeSlicedGeometry::Initialize(unsigned int timeSteps)
{
  Superclass::Initialize();

  m_TimeSteps = timeSteps;
  
  m_Geometry3Ds.clear();  

  Geometry3D::Pointer gnull=NULL;
 
  m_Geometry3Ds.reserve(m_TimeSteps);
  m_Geometry3Ds.assign(m_TimeSteps, gnull);
}

void mitk::TimeSlicedGeometry::InitializeEvenlyTimed(mitk::Geometry3D* geometry3D, unsigned int timeSteps)
{
  assert(geometry3D!=NULL);

  Initialize(timeSteps);

  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  transform->SetMatrix(geometry3D->GetUnitsToMMAffineTransform()->GetMatrix());
  transform->SetOffset(geometry3D->GetUnitsToMMAffineTransform()->GetOffset());
  SetUnitsToMMAffineTransform(transform);

  SetBounds(geometry3D->GetBounds());
  SetGeometry3D(geometry3D, 0);
  SetEvenlyTimed();
}

mitk::TimeSlicedGeometry::TimeSlicedGeometry() : m_TimeSteps(0), m_EvenlyTimed(false)
{
}

mitk::TimeSlicedGeometry::~TimeSlicedGeometry()
{

}

const mitk::BoundingBox* mitk::TimeSlicedGeometry::GetBoundingBox() const
{
  mitk::BoundingBox::Pointer boundingBox=mitk::BoundingBox::New();
 
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
  mitk::ScalarType nullpoint[]={0,0,0};
  mitk::BoundingBox::PointType p(nullpoint);

  unsigned int t;

  mitk::Geometry3D* geometry3d;
  mitk::BoundingBox::ConstPointer nextBoundingBox;
  mitk::BoundingBox::PointIdentifier pointid=0;

  for(t=0; t<m_TimeSteps; ++t)
  {
    geometry3d = GetGeometry3D(t);
    assert(geometry3d!=NULL);
    nextBoundingBox = geometry3d->GetBoundingBox();
    assert(nextBoundingBox.IsNotNull());
    const mitk::BoundingBox::PointsContainer * nextPoints = nextBoundingBox->GetPoints();
    if(nextPoints!=NULL)
    {
      mitk::BoundingBox::PointsContainer::ConstIterator pointsIt = nextPoints->Begin();

      while (pointsIt != nextPoints->End() )
      {
        pointscontainer->InsertElement( pointid++, pointsIt->Value());
        ++pointsIt;
      }
    }
  }

  boundingBox->SetPoints(pointscontainer);

  boundingBox->ComputeBoundingBox();

  m_BoundingBox=boundingBox;

  return boundingBox.GetPointer();
}

void mitk::TimeSlicedGeometry::SetEvenlyTimed(bool on)
{
  m_EvenlyTimed = on;
  Modified();
}

bool mitk::TimeSlicedGeometry::IsValidTime(int t) const
{
  return (t>=0) && (t< (int)m_TimeSteps);
}

mitk::AffineGeometryFrame3D::Pointer mitk::TimeSlicedGeometry::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize(m_TimeSteps);
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

void mitk::TimeSlicedGeometry::InitializeGeometry(Self * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);

  newGeometry->SetEvenlyTimed(m_EvenlyTimed);

  unsigned int t;
  for(t=0; t<m_TimeSteps; ++t)
  {
    if(m_Geometry3Ds[t]==NULL)
    {
      assert(m_EvenlyTimed);
    }
    else
    {
      newGeometry->SetGeometry3D(dynamic_cast<Geometry3D*>(m_Geometry3Ds[t]->Clone().GetPointer()), t);
    }
  }
}
