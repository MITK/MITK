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

#ifndef MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
#define MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

#include "mitkPAPropertyCalculator.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT LinearSpectralUnmixingFilter : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(LinearSpectralUnmixingFilter, SpectralUnmixingFilterBase)
        itkFactorylessNewMacro(Self)

      enum AlgortihmType
      {
        householderQr,
        ldlt,
        llt,
        colPivHouseholderQr,
        jacobiSvd,
        fullPivLu,
        fullPivHouseholderQr,
        test
      };

      void mitk::pa::LinearSpectralUnmixingFilter::SetAlgorithm(AlgortihmType SetAlgorithmIndex);

    protected:
      LinearSpectralUnmixingFilter();
      virtual ~LinearSpectralUnmixingFilter();
 
      virtual Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector) override;

    private:
      mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType algorithmIndex;

      virtual Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> CalculateEndmemberMatrix(
        std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore, std::vector<int> m_Wavelength);
      PropertyCalculator::Pointer m_PropertyCalculatorEigen;

      virtual float propertyElement(mitk::pa::PropertyCalculator::ChromophoreType, int wavelength);

    };
  }
}
#endif // MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
