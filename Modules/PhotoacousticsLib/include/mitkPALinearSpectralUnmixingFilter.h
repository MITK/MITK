/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
#define MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    /**
    * \brief This filter is subclass of the spectral unmixing filter base. All algorithms in this class are
    * based on the Eigen open source c++ library. It takes a multispectral pixel as input and returns a vector
    * with the unmixing result.
    *
    * Input:
    * - endmemberMatrix Eigen Matrix with number of chromophores colums and number of wavelengths rows so matrix element (i,j) contains
    *   the absorbtion of chromophore j @ wavelength i taken from the database by PropertyElement method.
    * - inputVector Eigen Vector containing values of one pixel of XY-plane image with number of wavelength rows (z-dimension of a sequenece)
    *   so the pixelvalue of the first wavelength is stored in inputVector[0] and so on.
    *
    * Output:
    * - Eigen vector with unmixing result of one multispectral pixel. The ith element of the vector corresponds to the ith entry of the
    *   m_Chromophore vector.
    *
    * Algorithms (see AlgortihmType enum):
    * - HOUSEHOLDERQR computes the solution by QR decomposition
    * - COLPIVHOUSEHOLDERQR computes the solution by QR decomposition
    * - FULLPIVHOUSEHOLDERQR computes the solution by QR decomposition
    * - LDLT computes the solution by Cholesky decomposition
    * - LLT computes the solution by Cholesky decomposition
    * - JACOBISVD computes the solution by singular value decomposition uses least square solver
    *
    * Possible exceptions:
    * - "algorithm not working": doesn't work now (2018/06/19)
    * - "404 VIGRA ALGORITHM NOT FOUND": Algorithm not implemented
    */

    class MITKPHOTOACOUSTICSLIB_EXPORT LinearSpectralUnmixingFilter : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(LinearSpectralUnmixingFilter, SpectralUnmixingFilterBase);

      itkFactorylessNewMacro(Self);

      /**
      * \brief Contains all implemented Eigen algorithms for spectral unmixing. For detailed information of the algorithms look at the
      * mitkPALinearSpectralUnmixingFilter.h documentation (section Algorithms).
      */
      enum AlgortihmType
      {
        HOUSEHOLDERQR,
        LDLT,
        LLT,
        COLPIVHOUSEHOLDERQR,
        JACOBISVD,
        FULLPIVLU,
        FULLPIVHOUSEHOLDERQR
      };

      /**
      * \brief Takes a mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType and fix it for usage at the "SpectralUnmixingAlgorithm" method.
      * @param inputAlgorithmName has to be a mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType
      */
      void SetAlgorithm(AlgortihmType inputAlgorithmName);

    protected:
      LinearSpectralUnmixingFilter();
      ~LinearSpectralUnmixingFilter() override;

      /**
      * \brief overrides the baseclass method with a mehtod to calculate the spectral unmixing result vector. Herain the class performs the
      * algorithm set by the "SetAlgorithm" method and writes the result into a Eigen vector which is the return value.
      * @param endmemberMatrix Matrix with number of chromophores colums and number of wavelengths rows so matrix element (i,j) contains
      * the absorbtion of chromophore j @ wavelength i taken from the database by PropertyElement method.
      * @param inputVector Vector containing values of one pixel of XY-plane image with number of wavelength rows (z-dimension of a sequenece)
      * so the pixelvalue of the first wavelength is stored in inputVector[0] and so on.
      * @throws if the algorithmName is not a member of the enum VigraAlgortihmType
      * @throws if one chooses the ldlt/llt solver which doens't work yet
      */
      Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> endmemberMatrix,
        Eigen::VectorXf inputVector) override;

    private:
      AlgortihmType algorithmName;
    };
  }
}
#endif // MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
