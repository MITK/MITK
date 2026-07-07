/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkHistogramGenerator_h
#define mitkHistogramGenerator_h

#include <mitkImage.h>
#include <itkHistogram.h>
#include <itkImage.h>
#include <itkObject.h>

namespace mitk
{
  /**
   * \brief Provides an easy way to calculate an itk::Histogram for a mitk::Image.
   *
   * \sa mitk::Image
   */
  class MITKCORE_EXPORT HistogramGenerator : public itk::Object
  {
  public:
    mitkClassMacroItkParent(HistogramGenerator, itk::Object);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) typedef itk::Statistics::Histogram<double> HistogramType;

    /** \brief Set the input image for histogram computation. */
    itkSetMacro(Image, mitk::Image::ConstPointer);

    /** \brief Set the number of histogram bins. */
    itkSetMacro(Size, int);

    /** \brief Get the number of histogram bins. */
    itkGetConstMacro(Size, int);

    /** \brief Get the computed histogram. */
    itkGetConstObjectMacro(Histogram, HistogramType);

    /**
     * \brief Compute the histogram from the input image.
     *
     * The histogram is recomputed only if the image has been modified since
     * the last computation.
     */
    void ComputeHistogram();

    /**
     * \brief Get the maximum frequency in the computed histogram.
     *
     * \return The maximum frequency value.
     */
    float GetMaximumFrequency() const;

    /**
     * \brief Calculate the maximum frequency in a given histogram.
     *
     * \param histogram The histogram to analyze.
     * \return The maximum frequency value.
     */
    static float CalculateMaximumFrequency(const HistogramType *histogram);

  protected:
    HistogramGenerator();

    ~HistogramGenerator() override;

    mitk::Image::ConstPointer m_Image;
    int m_Size;
    HistogramType::ConstPointer m_Histogram;
  };

} // namespace mitk

#endif
