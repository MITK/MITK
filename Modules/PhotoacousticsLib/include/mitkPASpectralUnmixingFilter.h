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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H

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
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilter : public mitk::ImageToImageFilter
    {
    public:

      mitkClassMacro(SpectralUnmixingFilter, mitk::ImageToImageFilter)
        itkFactorylessNewMacro(Self)

      // Contains all (so far) possible chromophores
      enum ChromophoreType
      {
        OXYGENATED_HEMOGLOBIN = 1,
        DEOXYGENATED_HEMOGLOBIN = 2
      };

      // Void to creat m_vector of all chosen chromophores with push back method
      void AddChromophore(ChromophoreType);
      std::vector<int> m_Chromophore;
      
      // Void to creat m_vector of all wavelengths with push back method
      void AddWavelength(int wavelength);
      std::vector<int> m_Wavelength; 
     
    protected:
      SpectralUnmixingFilter();
      virtual ~SpectralUnmixingFilter();

    private:
      // Void checking precondtions possibly throwing exeptions
      virtual void CheckPreConditions(unsigned int NumberOfInputImages);

      virtual void GenerateData();
      virtual void InitializeOutputs();

      // Void to creat Eigen::Matrix of all absorbtions
      // @ specific wavelength (columns) of chromophores (rows)
      Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> AddEndmemberMatrix();
      Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>  EndmemberMatrix;
      PropertyCalculator::Pointer m_PropertyCalculator;

      // Test algorithm for SU --> later a new class should be set up
      Eigen::VectorXf SpectralUnmixingTestAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector);
    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
