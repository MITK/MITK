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

#ifndef _MITKPHOTOACOUSTICSBMODEFILTER_H_
#define _MITKPHOTOACOUSTICSBMODEFILTER_H_

#ifdef PHOTOACOUSTICS_USE_GPU
#include "mitkOclDataSetToDataSetFilter.h"
#endif

#include <itkObject.h>
#include "mitkImageToImageFilter.h"

namespace mitk
{
  #ifdef PHOTOACOUSTICS_USE_GPU

  /** Documentation
  *
  * \brief The PhotoacousticOCLBModeFilter simply takes the absolute of all pixels in the image.
  *
  * The filter gives the option to use a log after taking the absolute.
  */

  class PhotoacousticOCLBModeFilter : public OclDataSetToDataSetFilter, public itk::Object
  {

  public:
    mitkClassMacroItkParent(PhotoacousticOCLBModeFilter, itk::Object);
    itkNewMacro(Self);

    /**
    * @brief SetInput Set the input image
    * @param image a image.
    */
    void SetInput(Image::Pointer image);

    /** Update the filter */
    void Update();

    void SetParameters(bool useLogFilter)
    {
      m_UseLogFilter = useLogFilter;
    }    
    
    /**
     * @brief GetOutputAsImage Returns an mitk::Image constructed from the processed data
     */
    mitk::Image::Pointer GetOutput();

  protected:

    PhotoacousticOCLBModeFilter();

    virtual ~PhotoacousticOCLBModeFilter();

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

    mitk::Image::Pointer m_InputImage;
    unsigned int m_InputDim[3];
    unsigned int m_Size;

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
