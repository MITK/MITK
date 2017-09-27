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
  //##Documentation
  //## @brief
  //## @ingroup Process

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
  * @brief SetInput Manually set the input data while providing dimensions and memory size of the input data.
  */
  void SetInput(void* data, unsigned int* dimensions, unsigned int BpE);

  void* GetOutput();

  /**
  * @brief GetOutputAsImage Returns an mitk::Image constructed from the processed data
  */
  mitk::Image::Pointer GetOutputAsImage();

  /** Update the filter */
  void Update();

  /** Set the Apodisation function to apply when beamforming */
  void SetApodisation(float* apodisation, unsigned short apodArraySize)
  {
    m_ApodArraySize = apodArraySize;
    m_Apodisation = apodisation;
  }

  void SetConfig(BeamformingSettings settings)
  {
    m_ConfOld = m_Conf;
    m_Conf = settings;
  }

protected:

  /** Constructor */
  PhotoacousticOCLBeamformingFilter();

  /** Destructor */
  virtual ~PhotoacousticOCLBeamformingFilter();

  /** Initialize the filter */
  bool Initialize();

  void UpdateDataBuffers();

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

  std::chrono::steady_clock::time_point m_Begin;
};
}
#else
class PhotoacousticOCLBeamformingFilter : public itk::Object
{
public:
  mitkClassMacroItkParent(mitk::PhotoacousticOCLBeamformingFilter, itk::Object);
  itkNewMacro(Self);

protected:
  /** Constructor */
  PhotoacousticOCLBeamformingFilter() {}

  /** Destructor */
  virtual ~PhotoacousticOCLBeamformingFilter() {}
};
}
#endif
#endif
