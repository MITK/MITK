#include "mitkImageStatisticsHolder.h"

mitk::ImageStatisticsHolder::ImageStatisticsHolder()
{
}

const mitk::ImageStatisticsHolder::HistogramType* mitk::ImageStatisticsHolder::GetScalarHistogram(int t) const
{
  mitk::ImageTimeSelector* timeSelector = this->GetTimeSelector();
  if(timeSelector!=NULL)
  {
    timeSelector->SetTimeNr(t);
    timeSelector->UpdateLargestPossibleRegion();

    mitk::HistogramGenerator* generator = static_cast<mitk::HistogramGenerator*>(m_HistogramGeneratorObject.GetPointer());
    generator->SetImage(timeSelector->GetOutput());
    generator->ComputeHistogram();
    return static_cast<const mitk::ImageStatisticsHolder::HistogramType*>(generator->GetHistogram());
  }
  return NULL;
}

#include "mitkImageAccessByItk.h"

//#define BOUNDINGOBJECT_IGNORE

template < typename ItkImageType >
void mitk::_ComputeExtremaInItkImage(ItkImageType* itkImage, mitk::Image* mitkImage, int t)
{
  typename ItkImageType::RegionType region;
  region = itkImage->GetBufferedRegion();
  if(region.Crop(itkImage->GetRequestedRegion()) == false) return;
  if(region != itkImage->GetRequestedRegion()) return;

  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, region);
  typedef typename ItkImageType::PixelType TPixel;
  TPixel value = 0;

  if ( !mitkImage || !mitkImage->IsValidTimeStep( t ) ) return;
  mitkImage->Expand(t+1); // make sure we have initialized all arrays
  mitkImage->m_CountOfMinValuedVoxels[t] = 0;
  mitkImage->m_CountOfMaxValuedVoxels[t] = 0;

  mitkImage->m_Scalar2ndMin[t]=
    mitkImage->m_ScalarMin[t] = itk::NumericTraits<ScalarType>::max();
  mitkImage->m_Scalar2ndMax[t]=
    mitkImage->m_ScalarMax[t] = itk::NumericTraits<ScalarType>::NonpositiveMin();

  while( !it.IsAtEnd() )
  {
    value = it.Get();
    //  if ( (value > mitkImage->m_ScalarMin) && (value < mitkImage->m_Scalar2ndMin) )        mitkImage->m_Scalar2ndMin = value;
    //  else if ( (value < mitkImage->m_ScalarMax) && (value > mitkImage->m_Scalar2ndMax) )   mitkImage->m_Scalar2ndMax = value;
    //  else if (value > mitkImage->m_ScalarMax)                                              mitkImage->m_ScalarMax = value;
    //  else if (value < mitkImage->m_ScalarMin)                                              mitkImage->m_ScalarMin = value;

    // if numbers start with 2ndMin or 2ndMax and never have that value again, the previous above logic failed
#ifdef BOUNDINGOBJECT_IGNORE
    if( value > -32765)
    {
#endif
    // update min
    if ( value < mitkImage->m_ScalarMin[t] )
    {
        mitkImage->m_Scalar2ndMin[t] = mitkImage->m_ScalarMin[t];    mitkImage->m_ScalarMin[t] = value;
        mitkImage->m_CountOfMinValuedVoxels[t] = 1;
    }
    else if ( value == mitkImage->m_ScalarMin[t] )
    {
        ++mitkImage->m_CountOfMinValuedVoxels[t];
    }
    else if ( value < mitkImage->m_Scalar2ndMin[t] )
    {
        mitkImage->m_Scalar2ndMin[t] = value;
    }

    // update max
    if ( value > mitkImage->m_ScalarMax[t] )
    {
        mitkImage->m_Scalar2ndMax[t] = mitkImage->m_ScalarMax[t];    mitkImage->m_ScalarMax[t] = value;
        mitkImage->m_CountOfMaxValuedVoxels[t] = 1;
    }
    else if ( value == mitkImage->m_ScalarMax[t] )
    {
        ++mitkImage->m_CountOfMaxValuedVoxels[t];
    }
    else if ( value > mitkImage->m_Scalar2ndMax[t] )
    {
        mitkImage->m_Scalar2ndMax[t] = value;
    }
#ifdef BOUNDINGOBJECT_IGNORE
    }
#endif

    ++it;
  }

  //// guard for wrong 2dMin/Max on single constant value images
  if (mitkImage->m_ScalarMax[t] == mitkImage->m_ScalarMin[t])
  {
      mitkImage->m_Scalar2ndMax[t] = mitkImage->m_Scalar2ndMin[t] = mitkImage->m_ScalarMax[t];
  }
  mitkImage->m_LastRecomputeTimeStamp.Modified();
  //MITK_DEBUG <<"extrema "<<itk::NumericTraits<TPixel>::NonpositiveMin()<<" "<<mitkImage->m_ScalarMin<<" "<<mitkImage->m_Scalar2ndMin<<" "<<mitkImage->m_Scalar2ndMax<<" "<<mitkImage->m_ScalarMax<<" "<<itk::NumericTraits<TPixel>::max();
}

bool mitk::ImageStatisticsHolder::IsValidTimeStep(int t) const
{
  return ( ( m_Dimension >= 4 && t <= (int)m_Dimensions[3] && t > 0 ) || (t == 0) );
}

void mitk::ImageStatisticsHolder::Expand( unsigned int timeSteps )
{
  if(timeSteps < 1) itkExceptionMacro(<< "Invalid timestep in Image!");
  if(! IsValidTimeStep( timeSteps-1 ) ) return;
  Superclass::Expand(timeSteps);
  if(timeSteps > m_ScalarMin.size() )
  {
    m_ScalarMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_ScalarMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_Scalar2ndMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_Scalar2ndMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_CountOfMinValuedVoxels.resize(timeSteps, 0);
    m_CountOfMaxValuedVoxels.resize(timeSteps, 0);
  }
}

void mitk::ImageStatisticsHolder::ResetImageStatistics() const
{
  m_ScalarMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_ScalarMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_Scalar2ndMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_Scalar2ndMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_CountOfMinValuedVoxels.assign(1, 0);
  m_CountOfMaxValuedVoxels.assign(1, 0);
}

void mitk::ImageStatisticsHolder::ComputeImageStatistics(int t) const
{
  // timestep valid?
  if (!IsValidTimeStep(t)) return;

  // image modified?
  if (this->GetMTime() > m_LastRecomputeTimeStamp.GetMTime())
    this->ResetImageStatistics();

  // adapt vector length
  // the const_cast is necessary since the whole statistics are provided
  // with const-methods but are actually stored inside the object with mutable
  // members. This should be resolved in a redesign of the image class.
  const_cast<mitk::Image*>(this)->Expand(t+1);

  // do we have valid information already?
  if( m_ScalarMin[t] != itk::NumericTraits<ScalarType>::max() ||
    m_Scalar2ndMin[t] != itk::NumericTraits<ScalarType>::max() ) return; // Values already calculated before...
// FIXME Commended calls to itk statistics to avoid run-time erorrs
  const mitk::PixelType pType = m_ImageDescriptor->GetChannelTypeById(0);
  if(pType.GetNumberOfComponents() == 1)
  {
    // recompute
    mitk::ImageTimeSelector* timeSelector = this->GetTimeSelector();
    if(timeSelector!=NULL)
    {
      timeSelector->SetTimeNr(t);
      timeSelector->UpdateLargestPossibleRegion();
      mitk::Image* image = timeSelector->GetOutput();
      mitk::Image* thisImage = const_cast<Image*>(this);
      AccessByItk_2( image, _ComputeExtremaInItkImage, thisImage, t );
    }
  }
  else if(pType.GetNumberOfComponents() > 1)
  {
    m_ScalarMin[t] = 0;
    m_ScalarMax[t] = 255;
  }
}


mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValueMin(int t) const
{
  ComputeImageStatistics(t);
  return m_ScalarMin[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValueMax(int t) const
{
  ComputeImageStatistics(t);
  return m_ScalarMax[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValue2ndMin(int t) const
{
  ComputeImageStatistics(t);
  return m_Scalar2ndMin[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValue2ndMax(int t) const
{
  ComputeImageStatistics(t);
  return m_Scalar2ndMax[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetCountOfMinValuedVoxels(int t) const
{
  ComputeImageStatistics(t);
  return m_CountOfMinValuedVoxels[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetCountOfMaxValuedVoxels(int t) const
{
  ComputeImageStatistics(t);
  return m_CountOfMaxValuedVoxels[t];
}

