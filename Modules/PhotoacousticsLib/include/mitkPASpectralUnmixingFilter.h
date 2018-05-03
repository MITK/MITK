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

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

#include <MitkPhotoacousticsLibExports.h>

#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

// Test with ImagePixelAccessor
#include <mitkImagePixelAccessor.h>
#include <mitkImagePixelWriteAccessor.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilter : public mitk::ImageToImageFilter
    {
    public:

      mitkClassMacro(SpectralUnmixingFilter, mitk::ImageToImageFilter)
        itkFactorylessNewMacro(Self)

      //Contains all (so far) possible chromophores
      enum ChromophoreType
      {
        OXYGENATED = 1,
        DEOXYGENATED = 2
      };

      //Void to creat m_vector of all chosen chromophores with push back method
      void SetChromophores(ChromophoreType);
      std::vector<int> m_Chromophore;
      
      //Void to creat m_vector of all wavelengths with push back method
      void AddWavelength(int wavelength);
      std::vector<int> m_Wavelength; 
      
      void SetDimensions(int dimension);
      std::vector<int> m_Dimensions;


      //Void to creat Eigen::Matrix of all absorbtions
      //@ specific wavelength (columns) of chromophores (rows)
      Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> AddEndmemberMatrix();
     

    protected:
      SpectralUnmixingFilter();
      virtual ~SpectralUnmixingFilter();

    private:
      virtual void GenerateData();
      PropertyCalculator::Pointer m_PropertyCalculator;
      unsigned int numberofchromophores;
      unsigned int numberofwavelengths;
      unsigned int numberOfInputs;
      unsigned int numberOfOutputs;
      long length;

      Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>  EndmemberMatrix;

      //'New':
      virtual void CheckPreConditions(unsigned int NumberOfInputImages);

      virtual void InitializeOutputs();
      //Eigen::VectorXd OutputVector = (Eigen::VectorXd, Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>);
      Eigen::VectorXd SpectralUnmixingAlgorithms(Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXd inputVector);

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
