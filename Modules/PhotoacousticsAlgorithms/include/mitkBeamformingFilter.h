/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER
#define MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER

#include "mitkImageToImageFilter.h"
#include <functional>
#include "./OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.h"
#include "mitkBeamformingSettings.h"
#include "mitkBeamformingUtils.h"
#include "MitkPhotoacousticsAlgorithmsExports.h"

namespace mitk {
  /*!
  * \brief Class implementing an mitk::ImageToImageFilter for beamforming on both CPU and GPU
  *
  *  The class must be given a configuration class instance of mitk::BeamformingSettings for beamforming parameters through mitk::BeamformingFilter::Configure(BeamformingSettings settings)
  *  Whether the GPU is used can be set in the configuration.
  *  For significant problems or important messages a string is written, which can be accessed via GetMessageString().
  */

  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT BeamformingFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BeamformingFilter, ImageToImageFilter);

    mitkNewMacro1Param(Self, mitk::BeamformingSettings::Pointer);
    itkCloneMacro(Self);

    /** \brief Sets a callback for progress checking
    *
    *  An std::function<void(int, std::string)> can be set, through which progress of the currently updating filter is reported.
    *  The integer argument is a number between 0 an 100 to indicate how far completion has been achieved, the std::string argument indicates what the filter is currently doing.
    */
    void SetProgressHandle(std::function<void(int, std::string)> progressHandle);

  protected:
    BeamformingFilter(mitk::BeamformingSettings::Pointer settings);

    ~BeamformingFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;

    /** \brief The std::function<void(int, std::string)>, through which progress of the currently updating filter is reported.
    */
    std::function<void(int, std::string)> m_ProgressHandle;

    float* m_OutputData;
    float* m_InputData;
    float* m_InputDataPuffer;
    unsigned short* m_MinMaxLines;

    /** \brief Current configuration set
    */
    BeamformingSettings::Pointer m_Conf;

    /**
    * The size of the apodization array when it last changed.
    */
    int m_LastApodizationArraySize;

    /** \brief Pointer to the GPU beamforming filter class; for performance reasons the filter is initialized within the constructor and kept for all later computations.
    */
    mitk::PhotoacousticOCLBeamformingFilter::Pointer m_BeamformingOclFilter;
  };
} // namespace mitk

#endif //MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER
