/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKHISTOGRAMMATCHING_H
#define MITKHISTOGRAMMATCHING_H

#include "itkHistogramMatchingImageFilter.h"

#include "mitkRegistrationBase.h"
#include "mitkImageAccessByItk.h"

namespace mitk
{

  class MITK_CORE_EXPORT HistogramMatching : public RegistrationBase
  {

  public:
    
    mitkClassMacro(HistogramMatching, RegistrationBase);

    itkNewMacro(Self);

    void SetNumberOfMatchPoints(int matchPoints);
    void SetNumberOfHistogramLevels(int histogrammLevels);
    void SetThresholdAtMeanIntensity(bool on);

    virtual void GenerateData()
    {
      if (this->GetInput())
      {
        AccessByItk(this->GetInput(), GenerateData2);
      }
    }

  protected:
    HistogramMatching();
    virtual ~HistogramMatching();
    template < typename TPixel, unsigned int VImageDimension >
      void GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1);

    bool m_ThresholdAtMeanIntensity;
    int m_NumberOfHistogramLevels;
    int m_NumberOfMatchPoints;
  };

} // end namespace
#endif // MITKHISTOGRAMMATCHING_H
