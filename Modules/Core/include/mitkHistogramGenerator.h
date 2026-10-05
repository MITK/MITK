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
  class ProgressTask;

  /**
   * \brief Provides an easy way to calculate an itk::Histogram for a mitk::Image.
   *
   * The histogram covers the first time step, binned evenly from its smallest
   * value to just above its largest, as itk::Statistics::ImageToHistogramFilter
   * bins it with an automatic range. The voxels are binned in parallel, and an
   * image without a pipeline source is only read, so the computation may run
   * on a worker thread.
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
     *
     * \throw mitk::Exception No image is set, the bin count is not positive, or
     *        the image has more than one component per pixel or a pixel type
     *        without a scalar equivalent.
     * \throw itk::ProcessAborted A cancel was requested of the progress task.
     */
    void ComputeHistogram();

    /**
     * \brief Report the progress of ComputeHistogram into the given task.
     *
     * The steps are added to the task when the computation starts, and a
     * cancel requested of it ends the computation early. The task is not owned.
     * mitk::ScopedProgressTask hands one over for the duration of a call.
     *
     * \param[in] task The task, or nullptr for none.
     */
    void SetProgressTask(ProgressTask *task);

    /** \brief The task ComputeHistogram reports into, or nullptr. */
    ProgressTask *GetProgressTask() const;

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
    ProgressTask *m_ProgressTask;
  };

} // namespace mitk

#endif
