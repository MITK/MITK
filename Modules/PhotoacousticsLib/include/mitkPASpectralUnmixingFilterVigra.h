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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERVIGRA_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERVIGRA_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>
#include <mitkPALinearSpectralUnmixingFilter.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterVigra : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(SpectralUnmixingFilterVigra, SpectralUnmixingFilterBase)
        itkFactorylessNewMacro(Self)

        enum VigraAlgortihmType
      {
        LARS,
        GOLDFARB,
        WEIGHTED,
        LS,
        vigratest
      };

      void mitk::pa::SpectralUnmixingFilterVigra::SetAlgorithm(VigraAlgortihmType SetAlgorithmIndex);
    protected:
      SpectralUnmixingFilterVigra();
      virtual ~SpectralUnmixingFilterVigra();

      virtual Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector) override;

    private:
      mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType algorithmIndex;

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERVIGRA_H
