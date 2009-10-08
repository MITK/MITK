/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKRGBTORGBACASTIMAGEFILTER_H_HEADER_INCLUDED
#define MITKRGBTORGBACASTIMAGEFILTER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"

namespace itk {
template <class TPixel, unsigned int VImageDimension> class ITK_EXPORT Image;
}

namespace mitk {

//##Documentation
//## @brief 
//## @ingroup Process
class MITK_CORE_EXPORT RGBToRGBACastImageFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(RGBToRGBACastImageFilter, ImageToImageFilter);

  itkNewMacro(Self);


  /** Static convenience method to check if the passed mitk::Image is
   * an RGB image in the sense of this converter filter.
   * 
   * Returns falsefor RGBA and all other images.
   */
  static bool IsRGBImage( const mitk::Image *image );


protected:
  // Typedefs for supported RGB pixel types
  typedef itk::RGBPixel< unsigned char > UCRGBPixelType;
  typedef itk::RGBPixel< unsigned short > USRGBPixelType;
  typedef itk::RGBPixel< float > FloatRGBPixelType;
  typedef itk::RGBPixel< double > DoubleRGBPixelType;


  RGBToRGBACastImageFilter();

  ~RGBToRGBACastImageFilter();

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  template < typename TPixel, unsigned int VImageDimension >
  void InternalCast( itk::Image< TPixel, VImageDimension > *itkImage, 
    mitk::RGBToRGBACastImageFilter *addComponentFilter,
    typename TPixel::ComponentType defaultAlpha );

  mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
  mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

  //##Description 
  //## @brief Time when Header was last initialized
  itk::TimeStamp m_TimeOfHeaderInitialization;
};

} // namespace mitk

#endif /* MITKRGBTORGBACASTIMAGEFILTER_H_HEADER_INCLUDED */


