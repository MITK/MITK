/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterSimplex : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(SpectralUnmixingFilterSimplex, SpectralUnmixingFilterBase);

      itkFactorylessNewMacro(Self);

    protected:
      SpectralUnmixingFilterSimplex();
      ~SpectralUnmixingFilterSimplex() override;

    private:

      Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
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
