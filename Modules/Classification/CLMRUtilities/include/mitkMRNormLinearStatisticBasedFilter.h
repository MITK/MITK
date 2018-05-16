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

#ifndef MRNORMLINEARSTATISTICBASEDFILTER_H
#define MRNORMLINEARSTATISTICBASEDFILTER_H

#include "mitkCommon.h"
#include "MitkCLMRUtilitiesExports.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"

#include "itkImage.h"

namespace mitk {
  //##Documentation
  //## @brief
  //## @ingroup Process
  class MITKCLMRUTILITIES_EXPORT MRNormLinearStatisticBasedFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(MRNormLinearStatisticBasedFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetMask( const mitk::Image* mask );

    const mitk::Image* GetMask() const;

    enum NormalizationBase
    {
      MEAN,
      MODE,
      MEDIAN
    };

    itkGetConstMacro(CenterMode, NormalizationBase);
    itkSetMacro(CenterMode, NormalizationBase);

    itkGetConstMacro(IgnoreOutlier, bool);
    itkSetMacro(IgnoreOutlier, bool);

    itkGetConstMacro(TargetValue, double);
    itkSetMacro(TargetValue, double);

    itkGetConstMacro(TargetWidth, double);
    itkSetMacro(TargetWidth, double);

  protected:
    MRNormLinearStatisticBasedFilter();

    ~MRNormLinearStatisticBasedFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    template < typename TPixel, unsigned int VImageDimension >
    void InternalComputeMask(itk::Image<TPixel, VImageDimension>* itkImage);

    NormalizationBase m_CenterMode;
    bool m_IgnoreOutlier;
  private:
    double m_TargetValue;
    double m_TargetWidth;

  };
} // namespace mitk

#endif /* MRNORMLINEARSTATISTICBASEDFILTER_H */
