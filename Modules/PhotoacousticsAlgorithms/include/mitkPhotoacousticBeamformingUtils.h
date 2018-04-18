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

#ifndef MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER_UTILS
#define MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER_UTILS

#include "mitkImageToImageFilter.h"
#include <functional>
#include "./OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.h"
#include "mitkPhotoacousticBeamformingSettings.h"

namespace mitk {
  /*!
  * \brief Class implementing util functionality for beamforming on CPU
  *
  */
  class PhotoacousticBeamformingUtils final
  {
  public:

    /** \brief Function to perform beamforming on CPU for a single line, using DAS and quadratic delay
   */
    static void DASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize, const mitk::BeamformingSettings::Pointer config);

    /** \brief Function to perform beamforming on CPU for a single line, using DAS and spherical delay
    */
    static void DASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize, const mitk::BeamformingSettings::Pointer config);

    /** \brief Function to perform beamforming on CPU for a single line, using DMAS and quadratic delay
    */
    static void DMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize, const mitk::BeamformingSettings::Pointer config);

    /** \brief Function to perform beamforming on CPU for a single line, using DMAS and spherical delay
    */
    static void DMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize, const mitk::BeamformingSettings::Pointer config);

    /** \brief Function to perform beamforming on CPU for a single line, using signed DMAS and quadratic delay
    */
    static void sDMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize, const mitk::BeamformingSettings::Pointer config);

    /** \brief Function to perform beamforming on CPU for a single line, using signed DMAS and spherical delay
    */
    static void sDMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize, const mitk::BeamformingSettings::Pointer config);

    /** \brief Pointer holding the Von-Hann apodization window for beamforming
    * @param samples the resolution at which the window is created
    */
    static float* VonHannFunction(int samples);

    /** \brief Function to create a Hamming apodization window
    * @param samples the resolution at which the window is created
    */
    static float* HammFunction(int samples);

    /** \brief Function to create a Box apodization window
    * @param samples the resolution at which the window is created
    */
    static float* BoxFunction(int samples);

  protected:
    PhotoacousticBeamformingUtils();

    ~PhotoacousticBeamformingUtils();
  };
} // namespace mitk

#endif //MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER_UTILS
