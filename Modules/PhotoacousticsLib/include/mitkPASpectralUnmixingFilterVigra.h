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

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterVigra : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(SpectralUnmixingFilterVigra, SpectralUnmixingFilterBase)
        //itkFactorylessNewMacro(Self)

    protected:
      SpectralUnmixingFilterVigra();
      virtual ~SpectralUnmixingFilterVigra();
 
      virtual Eigen::VectorXf NNLSLARS(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector);
      virtual Eigen::VectorXf NNLSGoldfarb(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector);

    private:

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERVIGRA_H
