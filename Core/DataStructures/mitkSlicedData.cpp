#include "SlicedData.h"

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
	const Geometry3D::SizeType& largestPossibleRegionSize
		= GetGeometry().GetLargestPossibleRegion().GetSize();

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
	unsigned int i;

	// Is the requested region within the LargestPossibleRegion?
	// Note that the test is indeed against the largest possible region
	// rather than the buffered region; see DataObject::VerifyRequestedRegion.
	const IndexType &requestedRegionIndex = m_RequestedRegion.GetIndex();
	const Geometry3D::IndexType &largestPossibleRegionIndex
		= GetGeometry().GetLargestPossibleRegion().GetIndex();

	const SizeType& requestedRegionSize = m_RequestedRegion.GetSize();
	const Geometry3D::SizeType& largestPossibleRegionSize
		= GetGeometry().GetLargestPossibleRegion().GetSize();

#if _MSC_VER < 1300
	for (i=0; i< GImageDimension; ++i)
#else
	for (i=0; i< Geometry3D::GImageDimension; ++i)
#endif
	{
		if ( (requestedRegionIndex[i] < largestPossibleRegionIndex[i]) ||
			((requestedRegionIndex[i] + static_cast<long>(requestedRegionSize[i]))
				> (largestPossibleRegionIndex[i]+static_cast<long>(largestPossibleRegionSize[i]))))
		{
			return false;
		}
	}
	if ((i==4) && 
		((requestedRegionIndex[i] < 0) ||
		((requestedRegionIndex[i] + static_cast<long>(requestedRegionSize[i]))
			> m_ChannelNumber))
		)
	{
		return false;
	}

	return true;
}

//##ModelId=3E1410760114
void mitk::SlicedData::SetRequestedRegion(DataObject *data)
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
//##ModelId=3E19EA3300BA
mitk::SlicedData::SlicedData() : m_ChannelNumber(0)
{
}


//##ModelId=3E19EA3300CE
mitk::SlicedData::~SlicedData()
{
}

