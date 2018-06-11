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

      // Contains all (so far) possible chromophores


      // Void to creat m_vector of all chosen chromophores with push back method
      void AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType);
      
      // Void to creat m_vector of all wavelengths with push back method
      void AddWavelength(int wavelength);



    protected:
      SpectralUnmixingFilterBase();
      virtual ~SpectralUnmixingFilterBase();

      std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore;
      std::vector<int> m_Wavelength; 

      virtual Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector) = 0;

    private:
      // Void checking precondtions possibly throwing exeptions
      virtual void CheckPreConditions(unsigned int size, unsigned int NumberOfInputImages, const float* inputDataArray);

      virtual void GenerateData() override;
      virtual void InitializeOutputs();

      // Void to creat Eigen::Matrix of all absorbtions
      // @ specific wavelength (columns) of chromophores (rows)
      Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> AddEndmemberMatrix();

      PropertyCalculator::Pointer m_PropertyCalculator;
            
    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERBASE_
