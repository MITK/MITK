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

#include "QmitkDataGenerationJobBase.h"

// itk headers
#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

#include <MitkImageStatisticsUIExports.h>

/**
* /brief This class is executed as background thread for image statistics calculation.
*
*   This class is derived from QRunnable and is intended to be used by QmitkImageStatisticsView
*   to run the image statistics calculation in a background thread keeping the GUI usable.
*/
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsCalculationRunnable : public QmitkDataGenerationJobBase
{
  Q_OBJECT
public:

  typedef itk::Statistics::Histogram<double> HistogramType;

  /*!
  /brief standard constructor. */
  QmitkImageStatisticsCalculationRunnable();
  /*!
  /brief standard destructor. */
  ~QmitkImageStatisticsCalculationRunnable();

  /*!
  /brief Initializes the object with necessary data. */
  void Initialize(const mitk::Image* image, const mitk::BaseData* mask);
  /*!
  /brief returns the calculated image statistics. */
  mitk::ImageStatisticsContainer* GetStatisticsData() const;

  const mitk::Image* GetStatisticsImage() const;
  const mitk::BaseData* GetMaskData() const;

  /*!
  /brief Set flag to ignore zero valued voxels */
  void SetIgnoreZeroValueVoxel(bool _arg);
  /*!
  /brief Get status of zero value voxel ignoring. */
  bool GetIgnoreZeroValueVoxel() const;
  /*!
  /brief Set bin size for histogram resolution.*/
  void SetHistogramNBins(unsigned int nbins);
  /*!
  /brief Get bin size for histogram resolution.*/
  unsigned int GetHistogramNBins() const;

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
