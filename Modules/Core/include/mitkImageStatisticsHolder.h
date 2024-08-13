/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkImageStatisticsHolder_h
#define mitkImageStatisticsHolder_h

#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include <MitkCoreExports.h>

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

namespace mitk
{
  /**
    @brief Class holding the statistics information about a single mitk::Image

    This computation was previously directly included in the definition and implementation of the mitk::Image class
    but for having a clear interface, all statistics computation is moved to the ImageStatisticsHolder class.

    Each mitk::Image holds a normal pointer to its StatisticsHolder object. To get access to the methods, use the
    GetStatistics() method in mitk::Image class.

    Minimum or maximum might by infinite values. 2nd minimum and maximum are guaranteed to be finite values.
    */
  class MITKCORE_EXPORT ImageStatisticsHolder
  {
  public:
    /** Constructor */
    ImageStatisticsHolder(mitk::Image *image);

    /** Destructor */
    virtual ~ImageStatisticsHolder();

    typedef itk::Statistics::Histogram<double> HistogramType;

    virtual const HistogramType *GetScalarHistogram(int t = 0, unsigned int = 0);

    //##Documentation
    //## \brief Get the minimum for scalar images. Recomputation performed only when necessary.
    virtual ScalarType GetScalarValueMin(int t = 0, unsigned int component = 0);

    //##Documentation
    //## \brief Get the maximum for scalar images. Recomputation performed only when necessary.
    virtual ScalarType GetScalarValueMax(int t = 0, unsigned int component = 0);

    //##Documentation
    //## \brief Get the second smallest value for scalar images.
    //## Recomputation performed only when necessary.
    //## \post The returned value is always a finite value.
    virtual ScalarType GetScalarValue2ndMin(int t = 0, unsigned int component = 0);

    //##Documentation
    //## \brief Get the smallest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValueMinNoRecompute(unsigned int t = 0) const
    {
      if (t < m_ScalarMin.size())
        return m_ScalarMin[t];
      else
        return itk::NumericTraits<ScalarType>::max();
    }

    //##Documentation
    //## \brief Get the second smallest value for scalar images, but do not recompute it first
    //## \post The returned value is always a finite value.
    virtual mitk::ScalarType GetScalarValue2ndMinNoRecompute(unsigned int t = 0) const
    {
      if (t < m_Scalar2ndMin.size())
        return m_Scalar2ndMin[t];
      else
        return itk::NumericTraits<ScalarType>::max();
    }

    //##Documentation
    //## \brief Get the second largest value for scalar images
    //## \post The returned value is always a finite value.
    virtual ScalarType GetScalarValue2ndMax(int t = 0, unsigned int component = 0);

    //##Documentation
    //## \brief Get the largest value for scalar images, but do not recompute it first
    //## \post The returned value is always a finite value.
    virtual mitk::ScalarType GetScalarValueMaxNoRecompute(unsigned int t = 0)
    {
      if (t < m_ScalarMax.size())
        return m_ScalarMax[t];
      else
        return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    //##Documentation
    //## \brief Get the second largest value for scalar images, but do not recompute it first
    virtual mitk::ScalarType GetScalarValue2ndMaxNoRecompute(unsigned int t = 0)
    {
      if (t < m_Scalar2ndMax.size())
        return m_Scalar2ndMax[t];
      else
        return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    //##Documentation
    //## \brief Get the count of voxels with the smallest scalar value in the dataset
    virtual mitk::ScalarType GetCountOfMinValuedVoxels(int t = 0, unsigned int component = 0);

    //##Documentation
    //## \brief Get the count of voxels with the largest scalar value in the dataset
    virtual mitk::ScalarType GetCountOfMaxValuedVoxels(int t = 0, unsigned int component = 0);

    //##Documentation
    //## \brief Get the count of voxels with the largest scalar value in the dataset
    virtual unsigned int GetCountOfMaxValuedVoxelsNoRecompute(unsigned int t = 0)
    {
      if (t < m_CountOfMaxValuedVoxels.size())
        return m_CountOfMaxValuedVoxels[t];
      else
        return 0;
    }

    //##Documentation
    //## \brief Get the count of voxels with the smallest scalar value in the dataset
    virtual unsigned int GetCountOfMinValuedVoxelsNoRecompute(unsigned int t = 0) const
    {
      if (t < m_CountOfMinValuedVoxels.size())
        return m_CountOfMinValuedVoxels[t];
      else
        return 0;
    }

    bool IsValidTimeStep(int t) const;

    template <typename ItkImageType>
    friend void _ComputeExtremaInItkImage(const ItkImageType *itkImage,
                                          mitk::ImageStatisticsHolder *statisticsHolder,
                                          int t);

    template <typename ItkImageType>
    friend void _ComputeExtremaInItkVectorImage(const ItkImageType *itkImage,
                                                mitk::ImageStatisticsHolder *statisticsHolder,
                                                int t,
                                                unsigned int component);

  protected:
    virtual void ResetImageStatistics();

    virtual void ComputeImageStatistics(int t = 0, unsigned int component = 0);

    virtual void Expand(unsigned int timeSteps);

    ImageTimeSelector::Pointer GetTimeSelector();

    mitk::Image *m_Image;

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

} // end namespace
#endif
