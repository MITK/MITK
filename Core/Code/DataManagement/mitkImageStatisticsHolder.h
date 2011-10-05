#ifndef MITKIMAGESTATISTICSHOLDER_H
#define MITKIMAGESTATISTICSHOLDER_H

#include <MitkExports.h>
#include "mitkImage.h"

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

namespace mitk
{

class MITK_CORE_EXPORT ImageStatisticsHolder : public itk::Object
{
public:
    mitkClassMacro(ImageStatisticsHolder, itk::Object)

    itkNewMacro(Self)

    typedef itk::Statistics::Histogram<double> HistogramType;

    virtual const HistogramType* GetScalarHistogram(int t=0) const;

    //##Documentation
    //## \brief Get the minimum for scalar images
    virtual ScalarType GetScalarValueMin(int t=0) const;

    //##Documentation
    //## \brief Get the maximum for scalar images
    virtual ScalarType GetScalarValueMax(int t=0) const;

    //##Documentation
    //## \brief Get the second smallest value for scalar images
    virtual ScalarType GetScalarValue2ndMin(int t=0) const;

    //##Documentation
    //## \brief Get the smallest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValueMinNoRecompute( unsigned int t = 0 ) const
    {
      if ( t < m_ScalarMin.size() )
        return m_ScalarMin[t];
      else return itk::NumericTraits<ScalarType>::max();
    }

    //##Documentation
    //## \brief Get the second smallest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValue2ndMinNoRecompute( unsigned int t = 0 ) const
    {
      if ( t < m_Scalar2ndMin.size() )
        return m_Scalar2ndMin[t];
      else return itk::NumericTraits<ScalarType>::max();
    }

    //##Documentation
    //## \brief Get the second largest value for scalar images
    virtual ScalarType GetScalarValue2ndMax(int t=0) const;

    //##Documentation
    //## \brief Get the largest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValueMaxNoRecompute( unsigned int t = 0 ) const
    {
      if ( t < m_ScalarMax.size() )
        return m_ScalarMax[t];
      else return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    //##Documentation
    //## \brief Get the second largest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValue2ndMaxNoRecompute( unsigned int t = 0 ) const
    {
      if ( t < m_Scalar2ndMax.size() )
        return m_Scalar2ndMax[t];
      else return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    //##Documentation
    //## \brief Get the count of voxels with the smallest scalar value in the dataset
    mitk::ScalarType GetCountOfMinValuedVoxels(int t = 0) const;

    //##Documentation
    //## \brief Get the count of voxels with the largest scalar value in the dataset
    mitk::ScalarType GetCountOfMaxValuedVoxels(int t = 0) const;

    //##Documentation
    //## \brief Get the count of voxels with the largest scalar value in the dataset
    virtual unsigned int GetCountOfMaxValuedVoxelsNoRecompute( unsigned int t = 0 ) const
    {
      if ( t < m_CountOfMaxValuedVoxels.size() )
        return m_CountOfMaxValuedVoxels[t];
      else return 0;
    }

    //##Documentation
    //## \brief Get the count of voxels with the smallest scalar value in the dataset
    virtual unsigned int GetCountOfMinValuedVoxelsNoRecompute( unsigned int t = 0 ) const
    {
      if ( t < m_CountOfMinValuedVoxels.size() )
        return m_CountOfMinValuedVoxels[t];
      else return 0;
    }

    template < typename ItkImageType >
      friend void _ComputeExtremaInItkImage(ItkImageType* itkImage, mitk::Image * mitkImage, int t);

protected:

      ImageStatisticsHolder();

      virtual ~ImageStatisticsHolder();

      virtual void ResetImageStatistics() const;

      virtual void ComputeImageStatistics(int t=0) const;


      mutable itk::Object::Pointer m_HistogramGeneratorObject;

      mutable itk::Object::Pointer m_TimeSelectorForExtremaObject;
      mutable std::vector<unsigned int> m_CountOfMinValuedVoxels;
      mutable std::vector<unsigned int> m_CountOfMaxValuedVoxels;
      mutable std::vector<ScalarType> m_ScalarMin;
      mutable std::vector<ScalarType> m_ScalarMax;
      mutable std::vector<ScalarType> m_Scalar2ndMin;
      mutable std::vector<ScalarType> m_Scalar2ndMax;

      itk::TimeStamp m_LastRecomputeTimeStamp;

};

} //end namespace
#endif // MITKIMAGESTATISTICSHOLDER_H
