/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkImageStatisticsHolder_h
#define mitkImageStatisticsHolder_h

#include <mitkImage.h>
#include <mitkImageTimeSelector.h>
#include <MitkCoreExports.h>

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

namespace mitk
{
  /**
   * \brief Class holding the statistics information about a single mitk::Image.
   *
   * This computation was previously directly included in the definition and implementation of the mitk::Image class
   * but for having a clear interface, all statistics computation is moved to the ImageStatisticsHolder class.
   *
   * Each mitk::Image holds a normal pointer to its StatisticsHolder object. To get access to the methods, use the
   * GetStatistics() method in mitk::Image class.
   *
   * Minimum or maximum might be infinite values. 2nd minimum and maximum are guaranteed to be finite values.
   *
   * \sa Image
   * \sa Image::GetStatistics()
   */
  class MITKCORE_EXPORT ImageStatisticsHolder
  {
  public:
    /**
     * \brief Constructor.
     *
     * \param image The image whose statistics this holder manages.
     */
    ImageStatisticsHolder(mitk::Image *image);

    /** \brief Destructor. */
    virtual ~ImageStatisticsHolder();

    typedef itk::Statistics::Histogram<double> HistogramType;

    /**
     * \brief Get the scalar histogram for the specified time step.
     *
     * \param t The time step. Defaults to 0.
     * \return Pointer to the histogram, or nullptr if unavailable.
     */
    virtual const HistogramType *GetScalarHistogram(int t = 0, unsigned int = 0);

    /**
     * \brief Get the minimum scalar value. Recomputation performed only when necessary.
     *
     * \param t The time step. Defaults to 0.
     * \param component The component index for vector images. Defaults to 0.
     * \return The minimum scalar value at the given time step.
     */
    virtual ScalarType GetScalarValueMin(int t = 0, unsigned int component = 0);

    /**
     * \brief Get the maximum scalar value. Recomputation performed only when necessary.
     *
     * \param t The time step. Defaults to 0.
     * \param component The component index for vector images. Defaults to 0.
     * \return The maximum scalar value at the given time step.
     */
    virtual ScalarType GetScalarValueMax(int t = 0, unsigned int component = 0);

    /**
     * \brief Get the second smallest scalar value. Recomputation performed only when necessary.
     *
     * \param t The time step. Defaults to 0.
     * \param component The component index for vector images. Defaults to 0.
     * \return The second smallest scalar value at the given time step.
     * \post The returned value is always a finite value.
     */
    virtual ScalarType GetScalarValue2ndMin(int t = 0, unsigned int component = 0);

    /**
     * \brief Get the smallest scalar value without triggering recomputation.
     *
     * \param t The time step. Defaults to 0.
     * \return The cached minimum scalar value, or max numeric limit if unavailable.
     */
    virtual mitk::ScalarType GetScalarValueMinNoRecompute(unsigned int t = 0) const
    {
      if (t < m_ScalarMin.size())
        return m_ScalarMin[t];
      else
        return itk::NumericTraits<ScalarType>::max();
    }

    /**
     * \brief Get the second smallest scalar value without triggering recomputation.
     *
     * \param t The time step. Defaults to 0.
     * \return The cached second minimum scalar value, or max numeric limit if unavailable.
     * \post The returned value is always a finite value.
     */
    virtual mitk::ScalarType GetScalarValue2ndMinNoRecompute(unsigned int t = 0) const
    {
      if (t < m_Scalar2ndMin.size())
        return m_Scalar2ndMin[t];
      else
        return itk::NumericTraits<ScalarType>::max();
    }

    /**
     * \brief Get the second largest scalar value. Recomputation performed only when necessary.
     *
     * \param t The time step. Defaults to 0.
     * \param component The component index for vector images. Defaults to 0.
     * \return The second largest scalar value at the given time step.
     * \post The returned value is always a finite value.
     */
    virtual ScalarType GetScalarValue2ndMax(int t = 0, unsigned int component = 0);

    /**
     * \brief Get the largest scalar value without triggering recomputation.
     *
     * \param t The time step. Defaults to 0.
     * \return The cached maximum scalar value, or NonpositiveMin if unavailable.
     * \post The returned value is always a finite value.
     */
    virtual mitk::ScalarType GetScalarValueMaxNoRecompute(unsigned int t = 0)
    {
      if (t < m_ScalarMax.size())
        return m_ScalarMax[t];
      else
        return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    /**
     * \brief Get the second largest scalar value without triggering recomputation.
     *
     * \param t The time step. Defaults to 0.
     * \return The cached second maximum scalar value, or NonpositiveMin if unavailable.
     */
    virtual mitk::ScalarType GetScalarValue2ndMaxNoRecompute(unsigned int t = 0)
    {
      if (t < m_Scalar2ndMax.size())
        return m_Scalar2ndMax[t];
      else
        return itk::NumericTraits<ScalarType>::NonpositiveMin();
    }

    /**
     * \brief Get the count of voxels with the smallest scalar value. Recomputation performed only when necessary.
     *
     * \param t The time step. Defaults to 0.
     * \param component The component index for vector images. Defaults to 0.
     * \return The number of voxels having the minimum scalar value.
     */
    mitk::ScalarType GetCountOfMinValuedVoxels(int t = 0, unsigned int component = 0);

    /**
     * \brief Get the count of voxels with the largest scalar value. Recomputation performed only when necessary.
     *
     * \param t The time step. Defaults to 0.
     * \param component The component index for vector images. Defaults to 0.
     * \return The number of voxels having the maximum scalar value.
     */
    mitk::ScalarType GetCountOfMaxValuedVoxels(int t = 0, unsigned int component = 0);

    /**
     * \brief Get the count of voxels with the largest scalar value without triggering recomputation.
     *
     * \param t The time step. Defaults to 0.
     * \return The cached count, or 0 if unavailable.
     */
    virtual unsigned int GetCountOfMaxValuedVoxelsNoRecompute(unsigned int t = 0)
    {
      if (t < m_CountOfMaxValuedVoxels.size())
        return m_CountOfMaxValuedVoxels[t];
      else
        return 0;
    }

    /**
     * \brief Get the count of voxels with the smallest scalar value without triggering recomputation.
     *
     * \param t The time step. Defaults to 0.
     * \return The cached count, or 0 if unavailable.
     */
    virtual unsigned int GetCountOfMinValuedVoxelsNoRecompute(unsigned int t = 0) const
    {
      if (t < m_CountOfMinValuedVoxels.size())
        return m_CountOfMinValuedVoxels[t];
      else
        return 0;
    }

    /**
     * \brief Check whether the given time step is valid for the associated image.
     *
     * \param t The time step to check.
     * \return True if the time step is valid.
     */
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
    /** \brief Reset all cached statistics values to their initial state. */
    virtual void ResetImageStatistics();

    /**
     * \brief Compute image statistics for the specified time step and component.
     *
     * \param t The time step. Defaults to 0.
     * \param component The component index for vector images. Defaults to 0.
     */
    virtual void ComputeImageStatistics(int t = 0, unsigned int component = 0);

    /**
     * \brief Expand internal statistics storage to accommodate the given number of time steps.
     *
     * \param timeSteps The number of time steps to support.
     */
    virtual void Expand(unsigned int timeSteps);

    /**
     * \brief Get an ImageTimeSelector configured with the associated image as input.
     *
     * \return A new ImageTimeSelector instance.
     */
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
