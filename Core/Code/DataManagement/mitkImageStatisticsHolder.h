#ifndef MITKIMAGESTATISTICSHOLDER_H
#define MITKIMAGESTATISTICSHOLDER_H

#include <MitkExports.h>
#include "mitkImage.h"
#include "mitkImageTimeSelector.h"

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

namespace mitk
{

/**
  @brief Class holding the statistics informations about a single mitk::Image

  This computation was previously directly included in the definition and implementation of the mitk::Image class
  but for having a clear interface, all statistics computation is moved to the ImageStatisticsHolder class.

  Each mitk::Image holds a normal pointer to its StatisticsHolder object. To get access to the methods, use the GetStatistics() method
  in mitk::Image class.
  */
class MITK_CORE_EXPORT ImageStatisticsHolder
{
public:
    /** Constructor */
  ImageStatisticsHolder(mitk::Image* image);

    /** Desctructor */
    virtual ~ImageStatisticsHolder();

    typedef itk::Statistics::Histogram<double> HistogramType;

    virtual const HistogramType* GetScalarHistogram(int t=0);

    //##Documentation
    //## \brief Get the minimum for scalar images
    virtual ScalarType GetScalarValueMin(int t=0);

    //##Documentation
    //## \brief Get the maximum for scalar images
    virtual ScalarType GetScalarValueMax(int t=0);

    //##Documentation
    //## \brief Get the second smallest value for scalar images
    virtual ScalarType GetScalarValue2ndMin(int t=0);

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
    virtual ScalarType GetScalarValue2ndMax(int t=0);

    //##Documentation
    //## \brief Get the largest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValueMaxNoRecompute( unsigned int t = 0 )
    {
      if ( t < m_ScalarMax.size() )
        return m_ScalarMax[t];
      else return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    //##Documentation
    //## \brief Get the second largest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValue2ndMaxNoRecompute( unsigned int t = 0 )
    {
      if ( t < m_Scalar2ndMax.size() )
        return m_Scalar2ndMax[t];
      else return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    //##Documentation
    //## \brief Get the count of voxels with the smallest scalar value in the dataset
    mitk::ScalarType GetCountOfMinValuedVoxels(int t = 0);

    //##Documentation
    //## \brief Get the count of voxels with the largest scalar value in the dataset
    mitk::ScalarType GetCountOfMaxValuedVoxels(int t = 0);

    //##Documentation
    //## \brief Get the count of voxels with the largest scalar value in the dataset
    virtual unsigned int GetCountOfMaxValuedVoxelsNoRecompute( unsigned int t = 0 )
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

    bool IsValidTimeStep( int t) const;

    template < typename ItkImageType >
      friend void _ComputeExtremaInItkImage( const ItkImageType* itkImage, mitk::ImageStatisticsHolder* statisticsHolder, int t);

protected:

      virtual void ResetImageStatistics();

      virtual void ComputeImageStatistics(int t=0);

      virtual void Expand( unsigned int timeSteps );

      ImageTimeSelector::Pointer GetTimeSelector();

      mitk::Image* m_Image;

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
