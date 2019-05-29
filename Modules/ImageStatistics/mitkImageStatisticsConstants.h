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

#ifndef _MITK_IMAGE_STATISTICS_CONSTANTS_H_
#define _MITK_IMAGE_STATISTICS_CONSTANTS_H_

#include <string>

#include <MitkImageStatisticsExports.h>

namespace mitk
{

  struct MITKIMAGESTATISTICS_EXPORT ImageStatisticsConstants

  {
    static const std::string MEAN();
    static const std::string MEDIAN();
    static const std::string STANDARDDEVIATION();
    static const std::string VARIANCE();
    static const std::string RMS();
    static const std::string MAXIMUM();
    static const std::string MAXIMUMPOSITION();
    static const std::string MINIMUM();
    static const std::string MINIMUMPOSITION();
    static const std::string NUMBEROFVOXELS();
    static const std::string VOLUME();
    static const std::string SKEWNESS();
    static const std::string KURTOSIS();
    static const std::string UNIFORMITY();
    static const std::string ENTROPY();
    static const std::string MPP();
    static const std::string UPP();

  };
}

#endif
