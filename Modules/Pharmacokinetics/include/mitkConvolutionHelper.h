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
#ifndef mitkConvolutionHelper_h
#define mitkConvolutionHelper_h

#include "itkArray.h"
#include "mitkAIFBasedModelBase.h"
#include <iostream>
#include "MitkPharmacokineticsExports.h"

namespace  mitk {
/** @namespace convolution
 * @brief Helper for itk implementation of vnl fourier transformation
 * This namespace provides functions for the preperation of vnl_fft_1d, including a wrapper
 * for wrapping the convolution kernel (turning it inside out) and a function for zeropadding
 * to avoid convolution artefacts. */
  namespace convolution {

    /** Some typedefs concerning data structures needed for vnl_fft_1d, which has vnl_vector< vcl_complex< double > >
     * as output typ of the forward transformation fwd_transform. Input is of type vnl_vector< vcl_complex< T > >
     * but since itk::Array is derived from vnl_vector, this works as well*/



    /** @brief Function that wraps the kernel */
inline    itk::Array<double> wrap1d(itk::Array<double> kernel)
        {
            int dim = kernel.GetNumberOfElements();
            itk::Array<double> wrappedKernel(dim);
            wrappedKernel.fill(0.);
                    for(int i=0; i< dim; ++i)
                    {
                        wrappedKernel.SetElement(i, kernel.GetElement((i+(dim/2))%dim));
                    }

            return wrappedKernel;
        }

    /** @brief Fuction for zeropadding (adding zeros) of an Array/vnl_vector, so that is has size paddedDimensions
     * @param paddedDimensions Dimensions that the Array should have after padding (convolution dimensions)
     * \remark dim = Dimensions of padded image --> PaddedDimension
     * \remark m dimensions of larger image
     * \remark n dimensions of image to be padded --> InitialDimension*/
   inline  itk::Array<double> zeropadding1d(itk::Array<double> unpaddedSpectrum, int paddedDimension)
        {

            int initialDimension = unpaddedSpectrum.GetNumberOfElements();

            itk::Array<double> paddedSpectrum(paddedDimension);
            paddedSpectrum.fill(0.);

            if(paddedDimension > initialDimension)
            {
                unsigned int padding = paddedDimension - initialDimension;

                for(int i=0; i<initialDimension ;++i)
                {
                    paddedSpectrum.SetElement(i+padding/2, unpaddedSpectrum.GetElement(i));
                }
            }
            return paddedSpectrum;
        }

     /** @brief Follow up function after back transformation from fourier space bwd_transform.
      * removes padding and scales (transformed values have to be divided by transformation dimensions) */
  inline  itk::Array<double> unpadAndScale(itk::Array<double> convolutionResult, int initialDimension)
        {
            int transformationDimension = convolutionResult.size();
            unsigned int padding = transformationDimension - initialDimension;

            itk::Array<double>  scaledResult(initialDimension);
            scaledResult.fill(0.0);

            for(int i = 0; i<initialDimension; ++i)
            {
                double value = convolutionResult(i+padding/2) / transformationDimension;
                scaledResult.SetElement(i,value);
            }
            return scaledResult;
        }
    /** @brief Convinience function for preparing 2 array for convolution with each other.
     * Takes both arrays of type itk::Array, zeropadds them to the sum of their sizes and wraps
     * the one specified as kernel. Returns them as vnl_vector<vcl_complex<double> >, ready to
     * be entered in fwd_transform*/

  inline  void prepareConvolution(const itk::Array<double>& kernel, const itk::Array<double>& spectrum, itk::Array<double>& preparedKernel, itk::Array<double>& preparedSpectrum ){
        int convolutionDimensions = kernel.GetSize() + spectrum.GetSize();

//        itk::Array<double> paddedKernel = zeropadding1d(kernel,convolutionDimensions);
        preparedKernel=zeropadding1d(kernel,convolutionDimensions);

        preparedSpectrum = zeropadding1d(spectrum,convolutionDimensions);
//        preparedKernel = wrap1d(paddedKernel);
    }

    }

  inline itk::Array<double> convoluteAIFWithExponential(mitk::ModelBase::TimeGridType timeGrid, mitk::AIFBasedModelBase::AterialInputFunctionType aif, double lambda)
  {
      /** @brief Iterative Formula to Convolve aif(t) with an exponential Residuefunction R(t) = exp(lambda*t)
       **/
      typedef itk::Array<double> ConvolutionResultType;
      ConvolutionResultType convolution(timeGrid.GetSize());
      convolution.fill(0.0);

      convolution(0) = 0;
      for(unsigned int i = 0; i< (timeGrid.GetSize()-1); ++i)
      {
          double dt = timeGrid(i+1) - timeGrid(i);
          double m = (aif(i+1) - aif(i))/dt;
          double edt = exp(-lambda *dt);

          convolution(i+1) =edt * convolution(i)
                           + (aif(i) - m*timeGrid(i))/lambda * (1 - edt )
                           + m/(lambda * lambda) * ((lambda * timeGrid(i+1) - 1) - edt*(lambda*timeGrid(i) -1));

      }
      return convolution;
  }


  inline itk::Array<double> convoluteAIFWithConstant(mitk::ModelBase::TimeGridType timeGrid, mitk::AIFBasedModelBase::AterialInputFunctionType aif, double constant)
  {
      /** @brief Iterative Formula to Convolve aif(t) with a constant value by linear interpolation of the Aif between sampling points
       **/
      typedef itk::Array<double> ConvolutionResultType;
      ConvolutionResultType convolution(timeGrid.GetSize());
      convolution.fill(0.0);

      convolution(0) = 0;
      for(unsigned int i = 0; i< (timeGrid.GetSize()-1); ++i)
      {
          double dt = timeGrid(i+1) - timeGrid(i);
          double m = (aif(i+1) - aif(i))/dt;

          convolution(i+1) = convolution(i) + constant * (aif(i)*dt + m*timeGrid(i)*dt + m/2*(timeGrid(i+1)*timeGrid(i+1) - timeGrid(i)*timeGrid(i)));

      }
      return convolution;
  }

}

#endif // mitkConvolutionHelper_h
