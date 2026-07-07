/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRGBToRGBACastImageFilter_h
#define mitkRGBToRGBACastImageFilter_h

#include <mitkImageTimeSelector.h>
#include <mitkImageToImageFilter.h>
#include <MitkCoreExports.h>

#include <itkRGBPixel.h>

namespace itk
{
  template <class TPixel, unsigned int VImageDimension>
  class ITK_EXPORT Image;
}

namespace mitk
{
  /**
   * \brief Image filter that casts RGB images to RGBA by adding a fully opaque alpha channel.
   *
   * This filter takes a mitk::Image with RGB pixel type (unsigned char, unsigned short,
   * float, or double components) and produces an output image with an RGBA pixel type
   * where the alpha channel is set to the maximum value for the component type.
   *
   * \sa ImageToImageFilter
   * \ingroup Process
   */
  class MITKCORE_EXPORT RGBToRGBACastImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(RGBToRGBACastImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Checks if the passed mitk::Image is an RGB image supported by this filter.
       *
       * Supported component types are unsigned char, unsigned short, float, and double.
       * Returns false for RGBA and all other pixel types.
       *
       * \param image the image to check.
       * \return True if the image has an RGB pixel type with a supported component type.
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

    /** \brief Time when the output header was last initialized. */
    itk::TimeStamp m_TimeOfHeaderInitialization;
  };

} // namespace mitk

#endif
