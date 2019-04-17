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

#ifndef mitkPhotoacousticFilterService_H_HEADER_INCLUDED
#define mitkPhotoacousticFilterService_H_HEADER_INCLUDED

#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkImage.h"
#include <functional>

#include "mitkBeamformingSettings.h"
#include "mitkBeamformingFilter.h"
#include "MitkPhotoacousticsAlgorithmsExports.h"

namespace mitk {
  /*!
  * \brief Class holding methods to apply all Filters within the Photoacoustics Algorithms Module
  *
  *  Implemented are:
  *  - A B-Mode Filter
  *  - A Resampling Filter
  *  - Beamforming on GPU and CPU
  *  - A Bandpass Filter
  */

  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT PhotoacousticFilterService : public itk::Object
  {
  public:
    mitkClassMacroItkParent(mitk::PhotoacousticFilterService, itk::Object);
    itkFactorylessNewMacro(Self);

    /** \brief Defines the methods for the B-Mode filter
    * Currently implemented are an Envelope Detection filter and a simple Absolute filter.
    */
    enum BModeMethod { EnvelopeDetection, Abs };

    /** \brief Applies a B-Mode Filter
    *
    * Applies a B-Mode filter using the given parameters.
    * @param inputImage The image to be processed.
    * @param method The kind of B-Mode Filter to be used.
    * @param UseLogFilter Setting this to true will apply a simple logarithm to the image after the B-Mode Filter has been applied.
    * @param resampleSpacing If this is set to 0, nothing will be done; otherwise, the image is resampled to a spacing of resampleSpacing mm per pixel.
    * @return The processed image is returned after the filter has finished.
    */
    mitk::Image::Pointer ApplyBmodeFilter(mitk::Image::Pointer inputImage,
      BModeMethod method = BModeMethod::Abs,
      bool UseLogFilter = false);

    /** \brief Resamples the given image
    *
    * Resamples an image using the given parameters.
    * @param inputImage The image to be processed.
    * @param outputSize An array of dimensions the image should be resampled to.
    * @return The processed image is returned after the filter has finished.
    */
    mitk::Image::Pointer ApplyResampling(mitk::Image::Pointer inputImage, double* outputSpacing);

    mitk::Image::Pointer ApplyResamplingToDim(mitk::Image::Pointer inputImage, double* outputDimension);

    /** \brief Beamforms the given image
    *
    * Resamples an image using the given parameters.
    * @param inputImage The image to be processed.
    * @param config The configuration set to be used for beamforming.
    * @param progressHandle An std::function<void(int, std::string)>, through which progress of the currently updating filter is reported.
    * The integer argument is a number between 0 an 100 to indicate how far completion has been achieved, the std::string argument indicates what the filter is currently doing.
    * @return The processed image is returned after the filter has finished.
    */
    mitk::Image::Pointer ApplyBeamforming(mitk::Image::Pointer inputImage, BeamformingSettings::Pointer config, std::function<void(int, std::string)> progressHandle = [](int, std::string) {});

    /** \brief Crops the given image
    *
    * Crops an image in 3 dimension using the given parameters.
    * @param inputImage The image to be processed.
    * @param above How many voxels will be cut from the top of the image.
    * @param below How many voxels will be cut from the bottom of the image.
    * @param right How many voxels will be cut from the right side of the image.
    * @param left How many voxels will be cut from the left side of the image.
    * @param minSlice The first slice to be present in the resulting volume.
    * @param maxSlice How many slices are cut off from the end of the volume.
    * @return The processed image is returned after the filter has finished. For the purposes of this module, the returned image is always of type float.
    */
    mitk::Image::Pointer ApplyCropping(mitk::Image::Pointer inputImage, int above, int below, int right, int left, int minSlice, int maxSlice, int* errCode);

    mitk::Image::Pointer ExtendImage(mitk::Image::Pointer inputImage, float pixelColor, unsigned int outputDimensionY);

    /** \brief Applies a Bandpass filter to the given image
    *
    * Applies a bandpass filter to the given image using the given parameters.
    * @param data The image to be processed.
    * @param BPHighPass The position at which Lower frequencies are completely cut off in Hz.
    * @param BPLowPass The position at which Higher frequencies are completely cut off in Hz.
    * @param alphaHighPass The high pass tukey window parameter to control the shape of the bandpass filter: 0 will make it a Box function, 1 a Hann function. alpha can be set between those two bounds.
    * @param alphaLowPass The low passtukey window parameter to control the shape of the bandpass filter: 0 will make it a Box function, 1 a Hann function. alpha can be set between those two bounds.
    * @return The processed image is returned after the filter has finished.
    */
    mitk::Image::Pointer ApplyBandpassFilter(mitk::Image::Pointer data,
      float BPHighPass, float BPLowPass,
      float alphaHighPass, float alphaLowPass,
      float timeSpacing, float SpeedOfSound, bool IsBFImage);

  protected:
    PhotoacousticFilterService();
    ~PhotoacousticFilterService() override;

    /** \brief
      For performance reasons, an instance of the Beamforming filter is initialized as soon as possible and kept for all further uses.
    */
    mitk::BeamformingFilter::Pointer m_BeamformingFilter;

    mitk::Image::Pointer ConvertToFloat(mitk::Image::Pointer);
  };
} // namespace mitk

#endif /* mitkPhotoacousticFilterService_H_HEADER_INCLUDED */
