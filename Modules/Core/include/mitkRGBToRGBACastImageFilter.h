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

#ifndef MITKRGBTORGBACASTIMAGEFILTER_H_HEADER_INCLUDED
#define MITKRGBTORGBACASTIMAGEFILTER_H_HEADER_INCLUDED

#include "mitkImageTimeSelector.h"
#include "mitkImageToImageFilter.h"
#include <MitkCoreExports.h>

#include <itkRGBPixel.h>

namespace itk
{
  template <class TPixel, unsigned int VImageDimension>
  class ITK_EXPORT Image;
}

namespace mitk
{
  //##Documentation
  //## @brief
  //## @ingroup Process
  class MITKCORE_EXPORT RGBToRGBACastImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(RGBToRGBACastImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      /** Static convenience method to check if the passed mitk::Image is
       * an RGB image in the sense of this converter filter.
       *
       * Returns falsefor RGBA and all other images.
       */
      static bool IsRGBImage(const mitk::Image *image);

  protected:
    // Typedefs for supported RGB pixel types
    typedef itk::RGBPixel<unsigned char> UCRGBPixelType;
    typedef itk::RGBPixel<unsigned short> USRGBPixelType;
    typedef itk::RGBPixel<float> FloatRGBPixelType;
    typedef itk::RGBPixel<double> DoubleRGBPixelType;

    RGBToRGBACastImageFilter();

    ~RGBToRGBACastImageFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCast(itk::Image<TPixel, VImageDimension> *itkImage,
                      mitk::RGBToRGBACastImageFilter *addComponentFilter,
                      typename TPixel::ComponentType defaultAlpha);

    mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
    mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;
  };

} // namespace mitk

#endif /* MITKRGBTORGBACASTIMAGEFILTER_H_HEADER_INCLUDED */
