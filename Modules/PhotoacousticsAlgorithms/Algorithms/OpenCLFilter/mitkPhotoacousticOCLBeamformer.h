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
#ifdef PHOTOACOUSTICS_USE_GPU

#include "mitkOclDataSetToDataSetFilter.h"
#include <itkObject.h>
#include "mitkPhotoacousticBeamformingFilter.h"

#include "mitkPhotoacousticOCLDelayCalculation.h"
#include "mitkPhotoacousticOCLMemoryLocSum.h"
#include "mitkPhotoacousticOCLUsedLinesCalculation.h"

namespace mitk
{
/** Documentation
  *
  * \brief The OclBinaryThresholdImageFilter computes a binary segmentation based on given
  threshold values.

  *
  * The filter requires two threshold values ( the upper and the lower threshold ) and two image values ( inside and outside ). The resulting voxel of the segmentation image is assigned the inside value 1 if the image value is between the given thresholds and the outside value otherwise.
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
  * @brief SetInput Manually set the input data while providing dimensions and memory size of the input data.
  */
  void SetInput(void* data, unsigned int* dimensions, unsigned int BpE);

  /**
  * @brief GetOutputAsImage Returns an mitk::Image constructed from the processed data
  */
  mitk::Image::Pointer GetOutputAsImage();

  /** Update the filter */
  void Update();
  
  /** Set the Output dimensions, which are also used for the openCL global worksize */
  void SetOutputDim( unsigned int outputDim[3])
  {
	  m_OutputDim[0] = outputDim[0];
	  m_OutputDim[1] = outputDim[1];
    m_OutputDim[2] = outputDim[2];
  }

  /** Set the Apodisation function to apply when beamforming */
  void SetApodisation(float* apodisation, unsigned short apodArraySize)
  {
    m_ApodArraySize = apodArraySize;
    m_Apodisation = apodisation;
  }

  void SetConfig(BeamformingFilter::beamformingSettings settings)
  {
    m_Conf = settings;
  }

protected:

  /** Constructor */
  PhotoacousticOCLBeamformingFilter();

  /** Destructor */
  virtual ~PhotoacousticOCLBeamformingFilter();

  /** Initialize the filter */
  bool Initialize();

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
  unsigned int m_InputDim[3];

  float* m_Apodisation;
  unsigned short m_ApodArraySize;

  unsigned short m_PAImage;

  BeamformingFilter::beamformingSettings m_Conf;

  mitk::Image::Pointer m_InputImage;

  size_t m_ChunkSize[3];

  mitk::OCLMemoryLocSum::Pointer m_SumFilter;
  mitk::OCLUsedLinesCalculation::Pointer m_UsedLinesCalculation;
  mitk::OCLDelayCalculation::Pointer m_DelayCalculation;
};
}
#endif
#endif
