#include "mitkSlicedData.h"

//##ModelId=3E141028018A
void mitk::SlicedData::UpdateOutputInformation()
{
  if (this->GetSource())
  {
    this->GetSource()->UpdateOutputInformation();
  }
  // If we don't have a source, then let's make our Image
  // span our buffer
  else
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

//##ModelId=3E14102C029E
void mitk::SlicedData::SetRequestedRegionToLargestPossibleRegion()
{
  m_UseLargestPossibleRegion = true;
  if(GetGeometry()==NULL) 
    return;
  int i;
  const RegionType::IndexType & index = GetLargestPossibleRegion().GetIndex();
  const RegionType::SizeType & size = GetLargestPossibleRegion().GetSize();
  for(i=0;i<VmitkImageDimension;++i)
  {
    m_RequestedRegion.SetIndex(i, index[i]);
    m_RequestedRegion.SetSize(i, size[i]);
  }
}

//##ModelId=3E14104300AC
bool mitk::SlicedData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if(VerifyRequestedRegion()==false) return true;

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

//##ModelId=3E14105B00F7
bool mitk::SlicedData::VerifyRequestedRegion()
{
  if(m_Geometry3D.IsNull()) return false;

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

  for (i=0; i< VmitkImageDimension; ++i)
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

//##ModelId=3E1410760114
void mitk::SlicedData::SetRequestedRegion(itk::DataObject *data)
{
  m_UseLargestPossibleRegion=false;

  mitk::SlicedData *slicedData;

  slicedData = dynamic_cast<mitk::SlicedData*>(data);

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


//##ModelId=3E19EA3300BA
mitk::SlicedData::SlicedData()
{
  unsigned int i;
  for(i=0;i<4;++i)
  {
    m_LargestPossibleRegion.SetIndex(i, 0);
    m_LargestPossibleRegion.SetSize (i, 1);
  }
}


//##ModelId=3E19EA3300CE
mitk::SlicedData::~SlicedData()
{
}

//##ModelId=3E34513B016D
void mitk::SlicedData::CopyInformation(const itk::DataObject *data)
{
  const mitk::SlicedData *slicedData;

  slicedData = dynamic_cast<const mitk::SlicedData*>(data);

  if (slicedData)
  {
    m_Geometry3D = slicedData->GetGeometry()->Clone();//new SlicedGeometry3D(*slicedData->GetGeometry().GetPointer());
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::SlicedData::CopyInformation(const DataObject *data) cannot cast " << typeid(data).name() << " to " << typeid(SlicedData*).name() );
  }
}

const mitk::Geometry2D* mitk::SlicedData::GetGeometry2D(int s, int t) const
{
  const_cast<SlicedData*>(this)->SetRequestedRegionToLargestPossibleRegion();

  const_cast<SlicedData*>(this)->UpdateOutputInformation();

  return m_SlicedGeometry->GetGeometry2D(s,t);
}

mitk::SlicedGeometry3D* mitk::SlicedData::GetSlicedGeometry() const
{
  return m_SlicedGeometry;
}

const mitk::SlicedGeometry3D* mitk::SlicedData::GetUpdatedSlicedGeometry()
{
  SetRequestedRegionToLargestPossibleRegion();

  UpdateOutputInformation();

  return m_SlicedGeometry;
}

void mitk::SlicedData::SetGeometry(Geometry3D* aGeometry3D)
{
  m_SlicedGeometry = dynamic_cast<SlicedGeometry3D*>(aGeometry3D);

  assert(m_SlicedGeometry==aGeometry3D);

  Superclass::SetGeometry(m_SlicedGeometry);
}
