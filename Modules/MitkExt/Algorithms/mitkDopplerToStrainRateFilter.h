/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MITKDOPPLERTOSTRAINRATEFILTER_H_HEADER_INCLUDED_C1F48A22
#define MITKDOPPLERTOSTRAINRATEFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkImageToImageFilter.h"

#ifndef M_PI
  #define M_PI       3.14159265358979323846
#endif

namespace mitk {
//##Documentation
//## @brief 
//## @ingroup Process

class MitkExt_EXPORT DopplerToStrainRateFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(DopplerToStrainRateFilter, ImageToImageFilter);

  itkNewMacro(Self);

  itkSetMacro(Distance, float);
  itkGetMacro(Distance, float);

  itkSetMacro(NoStrainInterval, int);
  itkGetMacro(NoStrainInterval, int);

  float GetLimit();

protected:

  //##Description 
  //## @brief Time when Header was last initialized
  itk::TimeStamp m_TimeOfHeaderInitialization;
 
 
protected:
  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  virtual void GenerateInputRequestedRegion();

  
  DopplerToStrainRateFilter();

    ~DopplerToStrainRateFilter();


private:

  float m_Distance;
  int m_NoStrainInterval;
  itk::Point<int, 3> m_Origin;  
    
};

} // namespace mitk

#endif /* MITKDOPPLERTOSTRAINRATE_H_HEADER_INCLUDED_C1F48A22 */


