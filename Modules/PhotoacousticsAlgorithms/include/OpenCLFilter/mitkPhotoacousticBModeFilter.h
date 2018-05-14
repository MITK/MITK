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

#if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN
#include "mitkOclDataSetToDataSetFilter.h"
#endif

#include <itkObject.h>
#include "mitkImageToImageFilter.h"

namespace mitk
{
  #if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN

  /*!
  * \brief Class implementing a mitk::OclDataSetToDataSetFilter for BMode filtering on GPU
  *
  *  The only parameter that needs to be provided is whether it should use a logfilter.
  *  Currently this class only performs an absolute BMode filter.
  */
  class PhotoacousticOCLBModeFilter : public OclDataSetToDataSetFilter, public itk::Object
  {

  public:
    mitkClassMacroItkParent(PhotoacousticOCLBModeFilter, itk::Object);
    itkNewMacro(Self);

    /** \brief Set the input image to be processed
    */
    void SetInput(Image::Pointer image);

    void Update();

    /** \brief Set parameters for the filter
    *
    * @param useLogFilter If true, the filter will apply a logfilter on the processed image
    */
    void SetParameters(bool useLogFilter)
    {
      m_UseLogFilter = useLogFilter;
    }    
    
    /**
     * @brief GetOutput Returns an mitk::Image constructed from the processed data
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

  /*!
  * \brief Class implementing a mitk::ImageToImageFilter for BMode filtering on CPU
  *
  *  The only parameter that needs to be provided is whether it should use a logfilter.
  *  Currently this class only performs an absolute BMode filter.
  */
  class PhotoacousticBModeFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(PhotoacousticBModeFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** \brief Set parameters for the filter
    *
    * @param useLogFilter If true, the filter will apply a logfilter on the processed image
    */
    void SetParameters(bool useLogFilter)
    {
      m_UseLogFilter = useLogFilter;
    }

  protected:

    PhotoacousticBModeFilter();

    ~PhotoacousticBModeFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;

    bool m_UseLogFilter;
  };
}
#endif
