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

#ifndef _MITKPHOTOACOUSTICSOCLBEAMFORMER_H_
#define _MITKPHOTOACOUSTICSOCLBEAMFORMER_H_

#include <itkObject.h>

#if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN

#include "mitkOclDataSetToDataSetFilter.h"

#include "mitkPhotoacousticOCLDelayCalculation.h"
#include "mitkPhotoacousticOCLUsedLinesCalculation.h"
#include "mitkPhotoacousticBeamformingSettings.h"

#include <chrono>

namespace mitk
{
  /*!
  * \brief Class implementing a mitk::OclDataSetToDataSetFilter for beamforming on GPU
  *
  *  The class must be given a configuration class instance of mitk::BeamformingSettings for beamforming parameters through mitk::PhotoacousticOCLBeamformingFilter::SetConfig(BeamformingSettings settings)
  *  Additional configuration of the apodisation function is needed.
  */

class PhotoacousticOCLBeamformingFilter : public OclDataSetToDataSetFilter, public itk::Object
{

public:
  mitkClassMacroItkParent(PhotoacousticOCLBeamformingFilter, itk::Object);
  itkNewMacro(Self);

  /**
  * @brief SetInput Set the input data through an image. Arbitrary images are supported
  */
  void SetInput(Image::Pointer image);
  /**
  *  brief SetInput Manually set the input data while providing 3 dimensions and memory size of the input data (Bytes per element).
  */
  void SetInput(void* data, unsigned int* dimensions, unsigned int BpE);
  /**
  * @brief GetOutput Get a pointer to the processed data. The standard datatype is float.
  */
  void* GetOutput();

  /**
  * @brief GetOutputAsImage Returns an mitk::Image constructed from the processed data
  */
  mitk::Image::Pointer GetOutputAsImage();

  /** \brief Update the filter */
  void Update();

  /** \brief Set the Apodisation function to apply when beamforming */
  void SetApodisation(float* apodisation, unsigned short apodArraySize)
  {
    m_ApodArraySize = apodArraySize;
    m_Apodisation = apodisation;
  }

  /** \brief Set beamforming settings to use when beamforming */
  void SetConfig(BeamformingSettings settings)
  {
    m_ConfOld = m_Conf;
    m_Conf = settings;
  }

protected:

  PhotoacousticOCLBeamformingFilter();
  virtual ~PhotoacousticOCLBeamformingFilter();

  /** \brief Initialize the filter */
  bool Initialize();

  /** \brief Updated the used data for beamforming depending on whether the configuration has significantly changed */
  void UpdateDataBuffers();

  /** \brief Execute the filter */
  void Execute();

  mitk::PixelType GetOutputType()
  {
    return mitk::MakeScalarPixelType<float>();
  }

  int GetBytesPerElem()
  {
    return sizeof(float);
  }

  virtual us::Module* GetModule();

private:
  /** The OpenCL kernel for the filter */
  cl_kernel m_PixelCalculation;

  unsigned int m_OutputDim[3];

  float* m_Apodisation;
  unsigned short m_ApodArraySize;

  unsigned short m_PAImage;

  BeamformingSettings m_Conf;
  BeamformingSettings m_ConfOld;

  mitk::Image::Pointer m_InputImage;

  size_t m_ChunkSize[3];

  mitk::OCLUsedLinesCalculation::Pointer m_UsedLinesCalculation;
  mitk::OCLDelayCalculation::Pointer m_DelayCalculation;

  cl_mem m_ApodizationBuffer;
  cl_mem m_MemoryLocationsBuffer;
  cl_mem m_DelaysBuffer;
  cl_mem m_UsedLinesBuffer;
};
}
#else
namespace mitk
{
  class PhotoacousticOCLBeamformingFilter : public itk::Object
  {
  public:
    mitkClassMacroItkParent(mitk::PhotoacousticOCLBeamformingFilter, itk::Object);
    itkNewMacro(Self);

  protected:
    /** Constructor */
    PhotoacousticOCLBeamformingFilter() {}

    /** Destructor */
    ~PhotoacousticOCLBeamformingFilter() override {}
  };
}
#endif
#endif
