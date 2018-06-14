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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERBASE_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERBASE_H

#include "mitkImageToImageFilter.h"
#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterBase : public mitk::ImageToImageFilter
    {
    public:
      mitkClassMacro(SpectralUnmixingFilterBase, mitk::ImageToImageFilter);

      void AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType);
      void AddWavelength(int wavelength);
      void AddWeight(int Weight);
      ofstream myfile;


    protected:
      SpectralUnmixingFilterBase();
      virtual ~SpectralUnmixingFilterBase();

      std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore;
      std::vector<int> m_Wavelength; 

      virtual Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector) = 0;

      std::vector<int> m_Weight;


    private:
      virtual void CheckPreConditions(unsigned int size, unsigned int NumberOfInputImages, const float* inputDataArray);

      virtual void GenerateData() override;
      virtual void InitializeOutputs(unsigned int TotalNumberOfSequences);

      virtual Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> CalculateEndmemberMatrix(
        std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore, std::vector<int> m_Wavelength);
      PropertyCalculator::Pointer m_PropertyCalculatorEigen;

      virtual float propertyElement(mitk::pa::PropertyCalculator::ChromophoreType, int wavelength);
    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERBASE_
