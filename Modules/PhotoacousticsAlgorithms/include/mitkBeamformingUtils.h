/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_BEAMFORMING_FILTER_UTILS
#define MITK_BEAMFORMING_FILTER_UTILS

#include "mitkImageToImageFilter.h"
#include <functional>
#include "./OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.h"
#include "mitkBeamformingSettings.h"

namespace mitk {
  /*!
  * \brief Class implementing util functionality for beamforming on CPU
  *
  */
  class BeamformingUtils final
  {
  public:

    /** \brief Function to perform beamforming on CPU for a single line, using DAS and spherical delay
    */
    static void DASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, const mitk::BeamformingSettings::Pointer config);

    /** \brief Function to perform beamforming on CPU for a single line, using DMAS and spherical delay
    */
    static void DMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, const mitk::BeamformingSettings::Pointer config);

    /** \brief Function to perform beamforming on CPU for a single line, using signed DMAS and spherical delay
    */
    static void sDMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, const mitk::BeamformingSettings::Pointer config);

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

    /** \brief
    */
    static unsigned short* MinMaxLines(const mitk::BeamformingSettings::Pointer config);

  protected:
    BeamformingUtils();

    ~BeamformingUtils();
  };
} // namespace mitk

#endif //MITK_BEAMFORMING_FILTER_UTILS
