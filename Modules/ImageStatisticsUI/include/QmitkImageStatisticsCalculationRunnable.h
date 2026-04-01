/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkImageStatisticsCalculationRunnable_h
#define QmitkImageStatisticsCalculationRunnable_h

//mitk headers
#include <mitkImageStatisticsContainer.h>

#include <QmitkDataGenerationJobBase.h>

// itk headers
#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

#include <MitkImageStatisticsUIExports.h>

/**
 * \brief Background thread runnable for computing image statistics.
 *
 * This class derives from QmitkDataGenerationJobBase and performs image statistics
 * calculation in a background thread, keeping the GUI responsive. It supports optional
 * mask data (binary images, multi-label segmentations, or planar figures) and can be
 * configured to ignore zero-valued voxels and to use a specific number of histogram bins.
 *
 * The computed results include an mitk::ImageStatisticsContainer with relation rules
 * connecting it to the source image and mask.
 *
 * \sa QmitkDataGenerationJobBase
 * \sa QmitkImageStatisticsDataGenerator
 * \sa mitk::ImageStatisticsCalculator
 */
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsCalculationRunnable : public QmitkDataGenerationJobBase
{
  Q_OBJECT
public:

  /** \brief ITK histogram type used for statistics computation. */
  typedef itk::Statistics::Histogram<double> HistogramType;

  /** \brief Default constructor. Initializes with zero-ignore disabled and 100 histogram bins. */
  QmitkImageStatisticsCalculationRunnable();

  /** \brief Destructor. */
  ~QmitkImageStatisticsCalculationRunnable();

  /**
   * \brief Initializes the runnable with the image and optional mask data.
   *
   * \param[in] image The input image for which statistics will be calculated.
   * \param[in] mask Optional mask data. Supported types: mitk::MultiLabelSegmentation,
   *                 mitk::Image (binary mask), or mitk::PlanarFigure. Pass nullptr for no mask.
   * \throw mitk::Exception if the mask is not nullptr and not one of the supported types.
   */
  void Initialize(const mitk::Image* image, const mitk::BaseData* mask);

  /**
   * \brief Returns the computed statistics container.
   * \return Pointer to the ImageStatisticsContainer, or nullptr if computation has not run or failed.
   */
  mitk::ImageStatisticsContainer* GetStatisticsData() const;

  /**
   * \brief Returns the input image used for statistics calculation.
   * \return Pointer to the input image.
   */
  const mitk::Image* GetStatisticsImage() const;

  /**
   * \brief Returns the mask data used for statistics calculation.
   * \return Pointer to the mask data, or nullptr if no mask was set.
   */
  const mitk::BaseData* GetMaskData() const;

  /**
   * \brief Sets whether zero-valued voxels should be ignored during computation.
   * \param[in] _arg True to ignore zero-valued voxels; false to include them.
   */
  void SetIgnoreZeroValueVoxel(bool _arg);

  /**
   * \brief Returns whether zero-valued voxels are ignored during computation.
   * \return True if zero-valued voxels are ignored; false otherwise.
   */
  bool GetIgnoreZeroValueVoxel() const;

  /**
   * \brief Sets the number of bins for histogram computation.
   * \param[in] nbins The number of histogram bins.
   */
  void SetHistogramNBins(unsigned int nbins);

  /**
   * \brief Returns the number of bins used for histogram computation.
   * \return The number of histogram bins.
   */
  unsigned int GetHistogramNBins() const;

  /**
   * \brief Returns the computation results as a labeled map.
   *
   * The result map contains a single entry with key "statistics" mapping to the
   * computed mitk::ImageStatisticsContainer.
   *
   * \return The result map.
   */
  ResultMapType GetResults() const override;

protected:
  bool RunComputation() override;

private:
  mitk::Image::ConstPointer m_StatisticsImage;                         ///< member variable holds the input image for which the statistics need to be calculated.
  mitk::BaseData::ConstPointer m_MaskData;                             ///< member variable holds the data that should be used as mask statistics calculation.
  mitk::ImageStatisticsContainer::Pointer m_StatisticsContainer;
  bool m_IgnoreZeros;                                             ///< member variable holds flag to indicate if zero valued voxel should be suppressed
  unsigned int m_HistogramNBins;                                      ///< member variable holds the bin size for histogram resolution.
};
#endif
