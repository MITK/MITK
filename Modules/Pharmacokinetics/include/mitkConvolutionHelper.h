/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkConvolutionHelper_h
#define mitkConvolutionHelper_h

#include <itkArray.h>
#include <mitkAIFBasedModelBase.h>
#include <iostream>
#include <MitkPharmacokineticsExports.h>

namespace  mitk {
/** \brief Helper functions for 1D convolution via VNL Fourier transformation.
 *
 * This namespace provides utility functions for preparing arrays for convolution using
 * vnl_fft_1d, including kernel wrapping (circular shift) and zero-padding to avoid
 * boundary artefacts.
 * \sa convoluteAIFWithExponential, convoluteAIFWithConstant
 */
  namespace convolution {

    /** \brief Wraps (circularly shifts) a 1D convolution kernel.
     *
     * Performs a circular shift of the kernel by half its length, which is required
     * for proper FFT-based convolution.
     * \param[in] kernel The input kernel array.
     * \return The wrapped kernel array. */
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

    /** \brief Zero-pads a 1D array to a specified size.
     *
     * Adds zeros symmetrically around the input array so it reaches the desired padded
     * dimension. This prevents circular convolution artefacts in FFT-based convolution.
     * \param[in] unpaddedSpectrum The input array to be padded.
     * \param[in] paddedDimension The target size after padding (sum of both convolution operand sizes).
     * \return The zero-padded array. */
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

     /** \brief Removes padding and scales the result after inverse FFT.
      *
      * After the backward FFT transformation, this function extracts the valid portion
      * of the result (removing padding) and divides by the transform size to correct
      * the FFT scaling.
      * \param[in] convolutionResult The raw inverse-FFT output.
      * \param[in] initialDimension The original (unpadded) array size.
      * \return The unpadded and scaled convolution result. */
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
    /** \brief Prepares two arrays for FFT-based convolution.
     *
     * Zero-pads both the kernel and spectrum arrays to the sum of their sizes so
     * they are ready for forward FFT transformation.
     * \param[in] kernel The convolution kernel array.
     * \param[in] spectrum The signal array to be convolved.
     * \param[out] preparedKernel The zero-padded kernel.
     * \param[out] preparedSpectrum The zero-padded spectrum. */
  inline  void prepareConvolution(const itk::Array<double>& kernel, const itk::Array<double>& spectrum, itk::Array<double>& preparedKernel, itk::Array<double>& preparedSpectrum ){
        int convolutionDimensions = kernel.GetSize() + spectrum.GetSize();

//        itk::Array<double> paddedKernel = zeropadding1d(kernel,convolutionDimensions);
        preparedKernel=zeropadding1d(kernel,convolutionDimensions);

        preparedSpectrum = zeropadding1d(spectrum,convolutionDimensions);
//        preparedKernel = wrap1d(paddedKernel);
    }

    }

  /** \brief Convolves the AIF with an exponential residue function using an iterative formula.
   *
   * Computes the convolution of the arterial input function aif(t) with the exponential
   * residue function R(t) = exp(-lambda * t) using linear interpolation between time grid
   * points. This is used by compartment models to compute tissue concentration curves.
   *
   * \param[in] timeGrid The time grid in seconds.
   * \param[in] aif The arterial input function values.
   * \param[in] lambda The exponential decay rate constant.
   * \return Array containing the convolution result at each time grid point. */
  inline itk::Array<double> convoluteAIFWithExponential(mitk::ModelBase::TimeGridType timeGrid, mitk::AIFBasedModelBase::AterialInputFunctionType aif, double lambda)
  {
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


  /** \brief Convolves the AIF with a constant value using an iterative formula.
   *
   * Computes the cumulative integral of the AIF multiplied by a constant factor
   * using linear interpolation between time grid points. This is used by irreversible
   * compartment models (e.g. FDG model).
   *
   * \param[in] timeGrid The time grid in seconds.
   * \param[in] aif The arterial input function values.
   * \param[in] constant The constant factor to multiply.
   * \return Array containing the convolution result at each time grid point. */
  inline itk::Array<double> convoluteAIFWithConstant(mitk::ModelBase::TimeGridType timeGrid, mitk::AIFBasedModelBase::AterialInputFunctionType aif, double constant)
  {
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

#endif
