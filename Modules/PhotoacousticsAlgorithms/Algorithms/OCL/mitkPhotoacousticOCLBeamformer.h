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

#include "..\\..\\..\\OpenCL\\mitkOclImageToImageFilter.h"
#include <itkObject.h>
namespace mitk
{
class OclImageToImageFilter;

/** Documentation
  *
  * \brief The OclBinaryThresholdImageFilter computes a binary segmentation based on given
  threshold values.

  *
  * The filter requires two threshold values ( the upper and the lower threshold ) and two image values ( inside and outside ). The resulting voxel of the segmentation image is assigned the inside value 1 if the image value is between the given thresholds and the outside value otherwise.
  */


class PhotoacousticOCLBeamformer : public OclImageToImageFilter, public itk::Object
{

public:
  mitkClassMacroItkParent(PhotoacousticOCLBeamformer, itk::Object);
  itkNewMacro(Self);

  /**
  * @brief SetInput Set the input image. Only 3D images are supported for now.
  * @param image a 3D image.
  * @throw mitk::Exception if the dimesion is not 3.
  */
  void SetInput(Image::Pointer image);

  mitk::Image::Pointer GetOutput();

  /** Update the filter */
  void Update();
  
  void SetOutputDim( unsigned int outputDim[3])
  {
	  m_OutputDim[0] = outputDim[0];
	  m_OutputDim[1] = outputDim[1];
    m_OutputDim[2] = outputDim[2];
  }

  void SetApodisation(float* apodisation, unsigned short apodArraySize)
  {
    m_ApodArraySize = apodArraySize;
    m_Apodisation = apodisation;
  }

  enum BeamformingAlgorithm { DASQuad, DMASQuad, DASSphe, DMASSphe };

  void SetAlgorithm(BeamformingAlgorithm algorithm, bool PA)
  {
    m_Algorithm = algorithm;
    m_PAImage = PA;
  }

  void SetBeamformingParameters(float SpeedOfSound, float RecordTime, float Pitch, float Angle, bool PAImage, unsigned short transducerElements)
  {
    m_SpeedOfSound = SpeedOfSound;
    m_RecordTime = RecordTime;
    m_Pitch = Pitch;
    m_Angle = Angle;
    m_PAImage = PAImage;
    m_TransducerElements = transducerElements;
  }

protected:

  /** Constructor */
  PhotoacousticOCLBeamformer();

  /** Destructor */
  virtual ~PhotoacousticOCLBeamformer();

  /** Initialize the filter */
  bool Initialize();

  void Execute();

  mitk::PixelType GetOutputType()
  {
    return mitk::MakeScalarPixelType<float>();
  }

  int GetBytesPerElem()
  {
    return sizeof(double);
  }

  virtual us::Module* GetModule();


private:
  /** The OpenCL kernel for the filter */
  cl_kernel m_PixelCalculation;

  unsigned int m_OutputDim[3];
  float* m_Apodisation;
  unsigned short m_ApodArraySize;
  BeamformingAlgorithm m_Algorithm;
  unsigned short m_PAImage;
  float m_SpeedOfSound;
  float m_RecordTime;
  float m_Pitch;
  float m_Angle;
  unsigned short m_TransducerElements;
};

class PhotoacousticPixelSum : public OclImageToImageFilter, public itk::Object
{

public:
  mitkClassMacroItkParent(PhotoacousticPixelSum, itk::Object);
  itkNewMacro(Self);

  /**
  * @brief SetInput Set the input image. Only 3D images are supported for now.
  * @param image a 3D image.
  * @throw mitk::Exception if the dimesion is not 3.
  */
  void SetInput(Image::Pointer image);

  void* GetRawOutput();

  /** Update the filter */
  void Update();

  void SetOutputDim(unsigned int outputDim[3])
  {
    m_OutputDim[0] = outputDim[0];
    m_OutputDim[1] = outputDim[1];
    m_OutputDim[2] = outputDim[2];
  }

  void SetImagesToAdd(unsigned short number)
  {
    m_Images = number;
  }

  void SetData(void* data)
  {
    m_Data = (float*)data;
  }

protected:

  /** Constructor */
  PhotoacousticPixelSum();

  /** Destructor */
  virtual ~PhotoacousticPixelSum();

  /** Initialize the filter */
  bool Initialize();

  void Execute();

  mitk::PixelType GetOutputType()
  {
    return mitk::MakeScalarPixelType<float>();
  }

  int GetBytesPerElem()
  {
    return sizeof(double);
  }

  virtual us::Module* GetModule();

private:
  /** The OpenCL kernel for the filter */
  cl_kernel m_PixelCalculation;

  unsigned int m_OutputDim[3];
  float* m_Data;
  unsigned short m_Images;
};
}
#endif