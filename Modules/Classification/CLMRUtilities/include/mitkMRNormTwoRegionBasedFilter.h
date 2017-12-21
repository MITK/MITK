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

#ifndef MITKMRNORMTWOREGIONBASEDFILTER_H
#define MITKMRNORMTWOREGIONBASEDFILTER_H

#include "mitkCommon.h"
#include "MitkCLMRUtilitiesExports.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"

#include "itkImage.h"

namespace mitk {
  //##Documentation
  //## @brief
  //## @ingroup Process
  class MITKCLMRUTILITIES_EXPORT MRNormTwoRegionsBasedFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(MRNormTwoRegionsBasedFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetMask1( const mitk::Image* mask );
    void SetMask2( const mitk::Image* mask );

    const mitk::Image* GetMask1() const;
    const mitk::Image* GetMask2() const;

    enum NormalizationBase
    {
      MEAN,
      MODE,
      MEDIAN
    };

    itkGetConstMacro(Area1, NormalizationBase);
    itkGetConstMacro(Area2, NormalizationBase);
    itkSetMacro(Area1, NormalizationBase);
    itkSetMacro(Area2, NormalizationBase);

  protected:
    MRNormTwoRegionsBasedFilter();

    ~MRNormTwoRegionsBasedFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    template < typename TPixel, unsigned int VImageDimension >
    void InternalComputeMask(itk::Image<TPixel, VImageDimension>* itkImage);

    NormalizationBase m_Area1;
    NormalizationBase m_Area2;
  };
} // namespace mitk

#endif /* MITKMRNORMTWOREGIONBASEDFILTER_H */
