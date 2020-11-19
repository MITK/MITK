/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    /**
    * \brief The spectral unmixing filter base is designed as superclass for several spectral unmixing filter eg. Eigen- or Vigrabased ones.
    * One can add wavelengths and chromophores and get a unmixed output images out of one MITK input image using algorithms from the subclasses.
    *
    * Input:
    * The unmixing input has to be a 3D MITK image where the XY-plane is a image and the Z-direction represents recordings for different
    * wavelengths. Herein a XY-plane for a specific Z-direction will be called "image". Every image has to be assigned to a certain wavelength.
    * The "AddWavelength" uses "push_back" to write float values into a vector. The first wavelength will correspond to the first image!!!
    * If there a more input images 'I' then added wavelengths 'w' the filter base interprets the next x images as repetition sequence of the same
    * wavelengths. If I % w !=0 the surplus image(s) will be dropped.
    * Addtionaly one has to add chromophores from the property calculator class enum "ChromophoreType" with the "AddChromophore" method.
    * This method as well uses "push_back" but the chosen (arbitary) order will be the order of the outputs.
    *
    * Output:
    * The output will be one MITK image per chosen chromophore. Where the XY-plane is a image and the Z-direction represents recordings for different
    * sequences. Furthermore it is possible to creat an output image that contains the information about the relative error between unmixing result
    * and the input image.
    *
    * Subclasses:
    * - mitkPASpectralUnmixingFilterVigra
    * - mitkPALinearSpectralUnmixingFilter (uses Eigen algorithms)
    * - mitkPASpectralUnmixingFilterSimplex
    *
    * Possible exceptions:
    * - "PIXELTYPE ERROR! FLOAT 32 REQUIRED": The MITK input image has to consist out of floats.
    * - "ERROR! REMOVE WAVELENGTHS!": One needs at least the same amount of images (z-dimension) then added wavelengths.
    * - "ADD MORE WAVELENGTHS!": One needs at least the same amount of wavelengths then added chromophores.
    * - "WAVELENGTH XXX nm NOT SUPPORTED!": The wavelength is not part of the proptery calculater data base. The data base can be found @
    *   [...]/mitk/Modules/PhotoacousticsLib/Resources/spectralLIB.dat
    * - "ADD OUTPUTS HAS TO BE LARGER THEN ZERO!"
    * - "NO WAVELENGHTS/CHROMOPHORES SELECZED!
    * - "INDEX ERROR! NUMBER OF OUTPUTS DOESN'T FIT TO OTHER SETTIGNS!"
    */

    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterBase : public mitk::ImageToImageFilter
    {
    public:
      mitkClassMacro(SpectralUnmixingFilterBase, mitk::ImageToImageFilter);

      /**
      * \brief AddChromophore takes mitk::pa::PropertyCalculator::ChromophoreType and writes them at the end of the m_Chromophore vector.
      * The call of the method sets the order of the GetOutput method!
      *
      * @param chromophore has to be element of porperty calculater enum chromophore type
      * @return for wavelength smaller then 300nm and larger then 1000nm the return will be 0, because not at the data base (2018/06/19)
      */
      void AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType chromophore);

      /**
      * \brief AddWavelength takes integers and writes them at the end of the m_Wavelength vector. The first call of the method then
      * corresponds to the first input image and so on.
      * @param wavelength database supports integers between 300 and 1000 nm
      */
      void AddWavelength(int wavelength);

      /*
      * \brief Verbose gives more information to the console. Default value is false.
      * @param verbose is the boolian to activate the MITK_INFO logged to the console
      */
      virtual void Verbose(bool verbose);

      /**
      * \brief AddOutputs takes an integer and sets indexed outputs
      * @param outputs integer correponds to the number of output images
      * @throws if outputs == 0
      */
      virtual void AddOutputs(unsigned int outputs);

      /*
      * \brief RelativeError returns a image which compare the L2 norm of the input vector with the unmixing result
      * @param relativeError is the boolian to activate this tool
      */
      virtual void RelativeError(bool relativeError);

      /**
      * \brief AddRelativeErrorSettings takes integers and writes them at the end of the m_RelativeErrorSettings vector.
      * @param value has to be a integer
      */
      virtual void AddRelativeErrorSettings(int value);

      std::ofstream myfile; // just for testing purposes; has to be removeed

    protected:
      /**
      * \brief Constructor creats proptery calculater smart pointer new()
      */
      SpectralUnmixingFilterBase();
      ~SpectralUnmixingFilterBase() override;

      /**
      * \brief The subclasses will override the mehtod to calculate the spectral unmixing result vector.
      * @param endmemberMatrix Matrix with number of chromophores colums and number of wavelengths rows so matrix element (i,j) contains
      * the absorbtion of chromophore j @ wavelength i taken from the database by PropertyElement method.
      * @param inputVector Vector containing values of one pixel of XY-plane image with number of wavelength rows (z-dimension of a sequenece)
      * so the pixelvalue of the first wavelength is stored in inputVector[0] and so on.
      * @throws if algorithm implementiation fails (implemented for the algorithms with critical requirements)
      */
      virtual Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> endmemberMatrix,
        Eigen::VectorXf inputVector) = 0;

      bool m_Verbose = false;
      bool m_RelativeError = false;

      std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore;
      std::vector<int> m_Wavelength;
      std::vector<int> m_RelativeErrorSettings;

    private:
      /*
      * \brief Initialized output images with the same XY-plane size like the input image and total size in z-direction equals number of sequences.
      * The pixel type is set to float.
      * @param totalNumberOfSequences = (unsigned int) (numberOfInputImages / numberOfWavelength) >= 1
      */
      virtual void InitializeOutputs(unsigned int totalNumberOfSequences);

      /*
      * \brief Checks if there are a suitable amount of wavelengths and if the input image consists of floats.
      * @param input pointer on the MITK input image
      * @throws if there are more wavelength then images
      * @throws if there are more chromophores then wavelengths
      * @throws if the pixel type is not float 32
      * @throws if no chromophores or wavelengths selected as input
      * @throws if the number of indexed outputs does'nt fit to the expected number
      */
      virtual void CheckPreConditions(mitk::Image::Pointer input);

      /*
      * \brief Inherit from the "ImageToImageFilter" Superclass. Herain it calls InitializeOutputs, CalculateEndmemberMatrix and
      * CheckPreConditions methods and enables pixelwise access to do spectral unmixing with the "SpectralUnmixingAlgorithm"
      * method. In the end the method writes the results into the new MITK output images.
      */
      void GenerateData() override;

      /*
      * \brief Creats a Matrix with number of chromophores colums and number of wavelengths rows so matrix element (i,j) contains
      * the absorbtion of chromophore j @ wavelength i. The absorbtion values are taken from the "PropertyElement" method.
      * @param m_Chromophore is a vector of "PropertyCalculator::ChromophoreType" containing all selected chromophores for the unmixing
      * @param m_Wavelength is a vector of integers containing all wavelengths of one sequence
      */
      virtual Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> CalculateEndmemberMatrix(
        std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore, std::vector<int> m_Wavelength);

      /*
      * \brief "PropertyElement" is the tool to access the absorbtion values out of the database using mitk::pa::PropertyCalculator::GetAbsorptionForWavelengt
      * and checks if the requested wavelength is part of the database (not zero values). The "ONEENDMEMBER" is a pseudo absorber with static absorbtion 1
      * at every wavelength and is therefor not part of the database. If this one is the selected chromophore the return value is 1 for every wavelength.
      * @param wavelength has to be integer between 300 and 1000 nm
      * @param chromophore has to be a mitk::pa::PropertyCalculator::ChromophoreType
      * @throws if mitk::pa::PropertyCalculator::GetAbsorptionForWavelengt returns 0, because this means that the delivered wavelength is not in the database.
      */
      virtual float PropertyElement(mitk::pa::PropertyCalculator::ChromophoreType, int wavelength);

      /*
      * \brief calculates the relative error between the input image and the unmixing result in the L2 norm
      * @param endmemberMatrix is a Eigen matrix containing the endmember information
      * @param inputVector is a Eigen vector containing the multispectral information of one pixel
      * @param resultVector is a Eigen vector containing the spectral unmmixing result
      */
      float CalculateRelativeError(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> endmemberMatrix,
        Eigen::VectorXf inputVector, Eigen::VectorXf resultVector);

      PropertyCalculator::Pointer m_PropertyCalculatorEigen;
    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERBASE_
