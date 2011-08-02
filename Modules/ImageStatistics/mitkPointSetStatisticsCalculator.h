/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PointSetSTATISTICSCALCULATOR_H
#define _MITK_PointSetSTATISTICSCALCULATOR_H

#include <itkObject.h>
#include "ImageStatisticsExports.h"
#include <mitkPointSet.h>

namespace mitk
{

/**
 * \brief Class for calculating statistics (like standard derivation, RMS, mean, etc.) for a PointSet.
 */
class ImageStatistics_EXPORT PointSetStatisticsCalculator : public itk::Object
{
public:

  //TODO: move this to an abstract class?
  struct Statistics
  {
    unsigned int N;
    double Min;
    double Max;
    double Mean;
    double Median;
    double StandardDerivation;
    double SampleStandardDerivation;
    double RMS;

    void Reset()
    {
      N = 0;
      Min = 0.0;
      Max = 0.0;
      Mean = 0.0;
      Median = 0.0;
      StandardDerivation = 0.0;
      SampleStandardDerivation = 0.0;
      RMS = 0.0;
    }
  };


  mitkClassMacro( PointSetStatisticsCalculator, itk::Object );
  itkNewMacro( PointSetStatisticsCalculator );

 
protected:
  
  PointSetStatisticsCalculator();
  virtual ~PointSetStatisticsCalculator();
};

}

#endif // #define _MITK_PointSetSTATISTICSCALCULATOR_H