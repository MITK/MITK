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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterSimplex : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(SpectralUnmixingFilterSimplex, SpectralUnmixingFilterBase)
        itkFactorylessNewMacro(Self)

    protected:
      SpectralUnmixingFilterSimplex();
      virtual ~SpectralUnmixingFilterSimplex();

    private:

      virtual Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector) override;

      int factorial(int n);
      virtual Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> GenerateA(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector, int i);
      Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> GenerateD2(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A);
      float simplexVolume(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrix);

      virtual Eigen::VectorXf Normalization(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector);

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H
