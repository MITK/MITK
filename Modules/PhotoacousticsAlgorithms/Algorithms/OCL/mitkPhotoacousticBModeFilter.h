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
#include "mitkOclImageToImageFilter.h"
#endif

#include <itkObject.h>

#include "mitkImageToImageFilter.h"

namespace mitk
{
  #ifdef PHOTOACOUSTICS_USE_GPU
  class OclImageToImageFilter;

  /** Documentation
  *
  * \brief The OclBinaryThresholdImageFilter computes a binary segmentation based on given
  threshold values.

  *
  * The filter requires two threshold values ( the upper and the lower threshold ) and two image values ( inside and outside ). The resulting voxel of the segmentation image is assigned the inside value 1 if the image value is between the given thresholds and the outside value otherwise.
  */

  class PhotoacousticOCLBModeFilter : public OclImageToImageFilter, public itk::Object
  {

  public:
    mitkClassMacroItkParent(PhotoacousticOCLBModeFilter, itk::Object);
    itkNewMacro(Self);

    /**
    * @brief SetInput Set the input image. Only 3D images are supported for now.
    * @param image a 3D image.
    * @throw mitk::Exception if the dimesion is not 3.
    */
    void SetInput(Image::Pointer image);

    /** Update the filter */
    void Update();

    void SetParameters(bool useLogFilter)
    {
      m_UseLogFilter = useLogFilter;
    }

  protected:

    /** Constructor */
    PhotoacousticOCLBModeFilter();

    /** Destructor */
    virtual ~PhotoacousticOCLBModeFilter();

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
    bool m_UseLogFilter;
  };
  #endif

  class PhotoacousticBModeFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(PhotoacousticBModeFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void SetParameters(bool useLogFilter)
    {
      m_UseLogFilter = useLogFilter;
    }

  protected:

    PhotoacousticBModeFilter();

    ~PhotoacousticBModeFilter();

    virtual void GenerateInputRequestedRegion() override;

    virtual void GenerateOutputInformation() override;

    virtual void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;

    bool m_UseLogFilter;
  };
}
#endif