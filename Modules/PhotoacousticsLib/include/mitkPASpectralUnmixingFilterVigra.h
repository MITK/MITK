/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERVIGRA_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERVIGRA_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>
#include <mitkPALinearSpectralUnmixingFilter.h>



namespace mitk {
  namespace pa {
    /**
    * \brief This filter is subclass of the spectral unmixing filter base. All algorithms in this class are
    * based on the vigra open source c++ library. It takes a multispectral pixel as input and returns a vector
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
    * Algorithms (see VigraAlgortihmType enum):
    * - LARS algorithm minimizes (A*x-b)^2  s.t. x>=0 using least angle regression
    * - GOLDFARB minimizes (A*x-b)^2  s.t. x>=0 using the Goldfarb-Idnani algorithm
    * - WEIGHTED minimizes transpose(A*x-b)*diag(weights)*(A*x-b) using QR decomposition
    * - LS minimizes (A*x-b)^2 using QR decomposition
    *
    * Possible exceptions:
    * - "404 VIGRA ALGORITHM NOT FOUND": Algorithm not implemented
    */

    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterVigra : public SpectralUnmixingFilterBase
    {
    public:
      mitkClassMacro(SpectralUnmixingFilterVigra, SpectralUnmixingFilterBase);
      itkFactorylessNewMacro(Self);

      /**
      * \brief Contains all implemented Vigra algorithms for spectral unmixing. For detailed information of the algorithms look at the
      * mitkPASpectralUnmixingFilterVigra.h documentation (section Algorithms).
      */
      enum VigraAlgortihmType
      {
        LARS,
        GOLDFARB,
        WEIGHTED,
        LS
      };

      /**
      * \brief AddWeight takes integers and writes them at the end of the weightsvec vector. The first call of the method then
      * corresponds to the first input image/wavelength and so on.
      * @param weight is a percentage (integer) between 1 and 100
      */
      void AddWeight(unsigned int weight);

      /**
      * \brief Takes a mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType and fix it for usage at the "SpectralUnmixingAlgorithm"
      * method.
      * @param algorithmName has to be a mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType
      */
      void SetAlgorithm(VigraAlgortihmType inputAlgorithmName);

    protected:
      SpectralUnmixingFilterVigra();
      ~SpectralUnmixingFilterVigra() override;

      /**
      * \brief overrides the baseclass method with a mehtod to calculate the spectral unmixing result vector. Herain it first converts the
      * Eigen inputs to the Vigra class. Afterwards the class performs the algorithm set by the "SetAlgorithm" method and writes the result
      * into a Eigen vector which is the return value.
      * @param endmemberMatrix Matrix with number of chromophores colums and number of wavelengths rows so matrix element (i,j) contains
      * the absorbtion of chromophore j @ wavelength i taken from the database by PropertyElement method.
      * @param inputVector Vector containing values of one pixel of XY-plane image with number of wavelength rows (z-dimension of a sequenece)
      * so the pixelvalue of the first wavelength is stored in inputVector[0] and so on.
      * @throws if the algorithmName is not a member of the enum VigraAlgortihmType
      */
      Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector) override;

    private:
      std::vector<double> weightsvec;
      SpectralUnmixingFilterVigra::VigraAlgortihmType algorithmName;
    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERVIGRA_H
