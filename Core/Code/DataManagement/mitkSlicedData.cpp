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


#include "mitkSlicedData.h"
#include "mitkBaseProcess.h"
#include <mitkProportionalTimeGeometry.h>


mitk::SlicedData::SlicedData() : m_UseLargestPossibleRegion(false)
{
  unsigned int i;
  for(i=0;i<4;++i)
  {
    m_LargestPossibleRegion.SetIndex(i, 0);
    m_LargestPossibleRegion.SetSize (i, 1);
  }
}

mitk::SlicedData::SlicedData( const SlicedData &other ): BaseData(other),
m_LargestPossibleRegion(other.m_LargestPossibleRegion),
m_RequestedRegion(other.m_RequestedRegion),
m_BufferedRegion(other.m_BufferedRegion),
m_UseLargestPossibleRegion(other.m_UseLargestPossibleRegion)
{

}
mitk::SlicedData::~SlicedData()
{
}

void mitk::SlicedData::UpdateOutputInformation()
{
  Superclass::UpdateOutputInformation();

  if (this->GetSource().IsNull())
  // If we don't have a source, then let's make our Image
  // span our buffer
  {
    m_UseLargestPossibleRegion = true;
  }

  // Now we should know what our largest possible region is. If our
  // requested region was not set yet, (or has been set to something
  // invalid - with no data in it ) then set it to the largest possible
  // region.
  if ( ! m_RequestedRegionInitialized)
  {
    this->SetRequestedRegionToLargestPossibleRegion();
    m_RequestedRegionInitialized = true;
  }

  m_LastRequestedRegionWasOutsideOfTheBufferedRegion = 0;
}

void mitk::SlicedData::PrepareForNewData()
{
  if ( GetUpdateMTime() < GetPipelineMTime() || GetDataReleased() )
  {
   ReleaseData();
  }
}

void mitk::SlicedData::SetRequestedRegionToLargestPossibleRegion()
{
  m_UseLargestPossibleRegion = true;
  if(GetGeometry()==NULL)
    return;
  unsigned int i;
  const RegionType::IndexType & index = GetLargestPossibleRegion().GetIndex();
  const RegionType::SizeType & size = GetLargestPossibleRegion().GetSize();
  for(i=0;i<RegionDimension;++i)
  {
    m_RequestedRegion.SetIndex(i, index[i]);
    m_RequestedRegion.SetSize(i, size[i]);
  }
}

bool mitk::SlicedData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  // Is the requested region within the currently buffered data?
  // SlicedData and subclasses store entire volumes or slices. The
  // methods IsVolumeSet() and IsSliceSet are provided to check,
  // a volume or slice, respectively, is available. Thus, these
  // methods used here.
  const IndexType &requestedRegionIndex = m_RequestedRegion.GetIndex();

  const SizeType& requestedRegionSize = m_RequestedRegion.GetSize();
  const SizeType& largestPossibleRegionSize
    = GetLargestPossibleRegion().GetSize();

  // are whole channels requested?
  int c, cEnd;
  c=requestedRegionIndex[4];
  cEnd=c+static_cast<long>(requestedRegionSize[4]);
  if(requestedRegionSize[3] == largestPossibleRegionSize[3])
  {
    for (; c< cEnd; ++c)
      if(IsChannelSet(c)==false) return true;
    return false;
  }

  // are whole volumes requested?
  int t, tEnd;
  t=requestedRegionIndex[3];
  tEnd=t+static_cast<long>(requestedRegionSize[3]);
  if(requestedRegionSize[2] == largestPossibleRegionSize[2])
  {
    for (; c< cEnd; ++c)
      for (; t< tEnd; ++t)
        if(IsVolumeSet(t, c)==false) return true;
    return false;
  }

  // ok, only slices are requested. Check if they are available.
  int s, sEnd;
  s=requestedRegionIndex[2];
  sEnd=s+static_cast<long>(requestedRegionSize[2]);
  for (; c< cEnd; ++c)
    for (; t< tEnd; ++t)
      for (; s< sEnd; ++s)
        if(IsSliceSet(s, t, c)==false) return true;

  return false;
}

bool mitk::SlicedData::VerifyRequestedRegion()
{
  if(GetTimeGeometry() == NULL) return false;

  unsigned int i;

  // Is the requested region within the LargestPossibleRegion?
  // Note that the test is indeed against the largest possible region
  // rather than the buffered region; see DataObject::VerifyRequestedRegion.
  const IndexType &requestedRegionIndex = m_RequestedRegion.GetIndex();
  const IndexType &largestPossibleRegionIndex
    = GetLargestPossibleRegion().GetIndex();

  const SizeType& requestedRegionSize = m_RequestedRegion.GetSize();
  const SizeType& largestPossibleRegionSize
    = GetLargestPossibleRegion().GetSize();

  for (i=0; i< RegionDimension; ++i)
  {
    if ( (requestedRegionIndex[i] < largestPossibleRegionIndex[i]) ||
      ((requestedRegionIndex[i] + static_cast<long>(requestedRegionSize[i]))
    > (largestPossibleRegionIndex[i]+static_cast<long>(largestPossibleRegionSize[i]))))
    {
      return false;
    }
  }

  return true;
}

void mitk::SlicedData::SetRequestedRegion( const itk::DataObject *data)
{
  m_UseLargestPossibleRegion=false;

  const mitk::SlicedData *slicedData = dynamic_cast<const mitk::SlicedData*>(data);

  if (slicedData)
  {
    m_RequestedRegion = slicedData->GetRequestedRegion();
    m_RequestedRegionInitialized = true;
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::SlicedData::SetRequestedRegion(DataObject*) cannot cast " << typeid(data).name() << " to " << typeid(SlicedData*).name() );
  }
}

void mitk::SlicedData::SetRequestedRegion(SlicedData::RegionType *region)
{
  m_UseLargestPossibleRegion=false;

  if(region!=NULL)
  {
    m_RequestedRegion = *region;
    m_RequestedRegionInitialized = true;
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::SlicedData::SetRequestedRegion(SlicedData::RegionType*) cannot cast " << typeid(region).name() << " to " << typeid(SlicedData*).name() );
  }
}

void mitk::SlicedData::SetLargestPossibleRegion(SlicedData::RegionType *region)
{

  if(region!=NULL)
  {
    m_LargestPossibleRegion = *region;
    m_UseLargestPossibleRegion=true;
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::SlicedData::SetLargestPossibleRegion(SlicedData::RegionType*) cannot cast " << typeid(region).name() << " to " << typeid(SlicedData*).name() );
  }
}

void mitk::SlicedData::CopyInformation(const itk::DataObject *data)
{
  // Standard call to the superclass' method
  Superclass::CopyInformation(data);

  const mitk::SlicedData *slicedData;

  slicedData = dynamic_cast<const mitk::SlicedData*>(data);

  if (slicedData)
  {
    m_LargestPossibleRegion = slicedData->GetLargestPossibleRegion();
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::SlicedData::CopyInformation(const DataObject *data) cannot cast " << typeid(data).name() << " to " << typeid(SlicedData*).name() );
  }
}

//const mitk::Geometry2D* mitk::SlicedData::GetGeometry2D(int s, int t) const
//{
//  const_cast<SlicedData*>(this)->SetRequestedRegionToLargestPossibleRegion();
//
//  const_cast<SlicedData*>(this)->UpdateOutputInformation();
//
//  return GetSlicedGeometry(t)->GetGeometry2D(s);
//}
//
mitk::SlicedGeometry3D* mitk::SlicedData::GetSlicedGeometry(unsigned int t) const
{
  if (GetTimeGeometry() == NULL)
    return NULL;
  return dynamic_cast<SlicedGeometry3D*>(GetTimeGeometry()->GetGeometryForTimeStep(t).GetPointer());
}

const mitk::SlicedGeometry3D* mitk::SlicedData::GetUpdatedSlicedGeometry(unsigned int t)
{
  SetRequestedRegionToLargestPossibleRegion();

  UpdateOutputInformation();

  return GetSlicedGeometry(t);
}

void mitk::SlicedData::SetGeometry(Geometry3D* aGeometry3D)
{
  if(aGeometry3D!=NULL)
  {
    ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
    SlicedGeometry3D::Pointer slicedGeometry = dynamic_cast<SlicedGeometry3D*>(aGeometry3D);
    if(slicedGeometry.IsNull())
    {
      Geometry2D* geometry2d = dynamic_cast<Geometry2D*>(aGeometry3D);
      if(geometry2d!=NULL)
      {
        if((GetSlicedGeometry()->GetGeometry2D(0)==geometry2d) && (GetSlicedGeometry()->GetSlices()==1))
          return;
        slicedGeometry = SlicedGeometry3D::New();
        slicedGeometry->InitializeEvenlySpaced(geometry2d, 1);
      }
      else
      {
        slicedGeometry = SlicedGeometry3D::New();
        PlaneGeometry::Pointer planeGeometry = PlaneGeometry::New();
        planeGeometry->InitializeStandardPlane(aGeometry3D);
        slicedGeometry->InitializeEvenlySpaced(planeGeometry, (unsigned int)(aGeometry3D->GetExtent(2)));
      }
    }
    assert(slicedGeometry.IsNotNull());

    timeGeometry->Initialize(slicedGeometry, 1);
    Superclass::SetTimeGeometry(timeGeometry);
  }
  else
  {
    if(GetGeometry()==NULL)
      return;
    Superclass::SetGeometry(NULL);
  }
}

void mitk::SlicedData::SetSpacing(const float aSpacing[3])
{
  this->SetSpacing((mitk::Vector3D)aSpacing);
}

void mitk::SlicedData::SetOrigin(const mitk::Point3D& origin)
{
  TimeGeometry* timeGeometry = GetTimeGeometry();

  assert(timeGeometry!=NULL);

  mitk::SlicedGeometry3D* slicedGeometry;

  unsigned int steps = timeGeometry->CountTimeSteps();

  for(unsigned int timestep = 0; timestep < steps; ++timestep)
  {
    slicedGeometry = GetSlicedGeometry(timestep);
    if(slicedGeometry != NULL)
    {
      slicedGeometry->SetOrigin(origin);
      if(slicedGeometry->GetEvenlySpaced())
      {
        mitk::Geometry2D* geometry2D = slicedGeometry->GetGeometry2D(0);
        geometry2D->SetOrigin(origin);
        slicedGeometry->InitializeEvenlySpaced(geometry2D, slicedGeometry->GetSlices());
      }
    }
    //ProportionalTimeGeometry* timeGeometry = dynamic_cast<ProportionalTimeGeometry *>(GetTimeGeometry());
    //if(timeGeometry != NULL)
    //{
    //  timeGeometry->Initialize(slicedGeometry, steps);
    //  break;
    //}
  }
}

void mitk::SlicedData::SetSpacing(mitk::Vector3D aSpacing)
{
  TimeGeometry* timeGeometry = GetTimeGeometry();

  assert(timeGeometry!=NULL);

  mitk::SlicedGeometry3D* slicedGeometry;

  unsigned int steps = timeGeometry->CountTimeSteps();

  for(unsigned int timestep = 0; timestep < steps; ++timestep)
  {
    slicedGeometry = GetSlicedGeometry(timestep);
    if(slicedGeometry != NULL)
    {
      slicedGeometry->SetSpacing(aSpacing);
    }
  }
  timeGeometry->Update();
}


