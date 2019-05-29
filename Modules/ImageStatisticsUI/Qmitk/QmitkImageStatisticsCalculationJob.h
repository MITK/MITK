/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef QMITKIMAGESTATISTICSCALCULATIONTHREAD_H_INCLUDED
#define QMITKIMAGESTATISTICSCALCULATIONTHREAD_H_INCLUDED

//QT headers
#include <QThread>


//mitk headers
#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include "mitkImageStatisticsContainer.h"
#include <MitkImageStatisticsUIExports.h>

// itk headers
#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif


/** /brief This class is executed as background thread for image statistics calculation.
  * Documentation: This class is derived from QThread and is intended to be used by QmitkImageStatisticsView
                   to run the image statistics calculation in a background thread keeping the gui usable.
  */

class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsCalculationJob : public QThread
{
  Q_OBJECT

public:

  typedef itk::Statistics::Histogram<double> HistogramType;

  /*!
  /brief standard constructor. */
  QmitkImageStatisticsCalculationJob();
  /*!
  /brief standard destructor. */
  ~QmitkImageStatisticsCalculationJob();

  /*!
  /brief Initializes the object with necessary data. */
  void Initialize(const mitk::Image* image, const mitk::Image* binaryImage, const mitk::PlanarFigure* planarFig );
  /*!
  /brief returns the calculated image statistics. */
  mitk::ImageStatisticsContainer* GetStatisticsData() const;

  const mitk::Image* GetStatisticsImage() const;
  const mitk::Image* GetMaskImage() const;
  const mitk::PlanarFigure* GetPlanarFigure() const;

  /*!
  /brief Set flag to ignore zero valued voxels */
  void SetIgnoreZeroValueVoxel( bool _arg );
  /*!
  /brief Get status of zero value voxel ignoring. */
  bool GetIgnoreZeroValueVoxel() const;
  /*!
  /brief Set bin size for histogram resolution.*/
  void SetHistogramNBins( unsigned int nbins);
  /*!
  /brief Get bin size for histogram resolution.*/
  unsigned int GetHistogramNBins() const;
  /*!
  /brief Returns the histogram of the currently selected time step. */
  const HistogramType* GetTimeStepHistogram(unsigned int t = 0) const;

  /*!
  /brief Returns a flag the indicates if the statistics are updated successfully */
  bool GetStatisticsUpdateSuccessFlag() const;
  /*!
  /brief Method called once the thread is executed. */
  void run() override;

  std::string GetLastErrorMessage() const;

private:
  mitk::Image::ConstPointer m_StatisticsImage;                         ///< member variable holds the input image for which the statistics need to be calculated.
  mitk::Image::ConstPointer m_BinaryMask;                              ///< member variable holds the binary mask image for segmentation image statistics calculation.
  mitk::PlanarFigure::ConstPointer m_PlanarFigureMask;                 ///< member variable holds the planar figure for segmentation image statistics calculation.
  mitk::ImageStatisticsContainer::Pointer m_StatisticsContainer;
  bool m_IgnoreZeros;                                             ///< member variable holds flag to indicate if zero valued voxel should be suppressed
  unsigned int m_HistogramNBins;                                      ///< member variable holds the bin size for histogram resolution.
  bool m_CalculationSuccessful;                                   ///< flag set if statistics calculation was successful
  std::vector<HistogramType::ConstPointer> m_HistogramVector;          ///< member holds the histograms of all time steps.
  std::string m_message;
};
#endif // QMITKIMAGESTATISTICSCALCULATIONTHREAD_H_INCLUDED
