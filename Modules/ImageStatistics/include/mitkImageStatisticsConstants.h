/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageStatisticsConstants_h
#define mitkImageStatisticsConstants_h

#include <string>

#include <MitkImageStatisticsExports.h>

namespace mitk
{

  /**
   * \brief Provides string constants for the names of image statistics.
   *
   * These constants are used as keys when storing and retrieving individual
   * statistic values in an ImageStatisticsContainer::ImageStatisticsObject.
   *
   * \sa ImageStatisticsContainer
   */
  struct MITKIMAGESTATISTICS_EXPORT ImageStatisticsConstants

  {
    /** \brief Key for the arithmetic mean statistic. */
    static const std::string MEAN();
    /** \brief Key for the median statistic. */
    static const std::string MEDIAN();
    /** \brief Key for the standard deviation statistic. */
    static const std::string STANDARDDEVIATION();
    /** \brief Key for the variance statistic. */
    static const std::string VARIANCE();
    /** \brief Key for the root mean square (RMS) statistic. */
    static const std::string RMS();
    /** \brief Key for the maximum intensity value statistic. */
    static const std::string MAXIMUM();
    /** \brief Key for the index position of the maximum intensity. */
    static const std::string MAXIMUMPOSITION();
    /** \brief Key for the minimum intensity value statistic. */
    static const std::string MINIMUM();
    /** \brief Key for the index position of the minimum intensity. */
    static const std::string MINIMUMPOSITION();
    /** \brief Key for the total number of voxels statistic. */
    static const std::string NUMBEROFVOXELS();
    /** \brief Key for the volume (in physical units) statistic. */
    static const std::string VOLUME();
    /** \brief Key for the skewness statistic. */
    static const std::string SKEWNESS();
    /** \brief Key for the kurtosis statistic. */
    static const std::string KURTOSIS();
    /** \brief Key for the uniformity statistic. */
    static const std::string UNIFORMITY();
    /** \brief Key for the Shannon entropy statistic. */
    static const std::string ENTROPY();
    /** \brief Key for the mean of positive pixels (MPP) statistic. */
    static const std::string MPP();
    /** \brief Key for the uniformity of positive pixels (UPP) statistic. */
    static const std::string UPP();

  };
}

#endif
