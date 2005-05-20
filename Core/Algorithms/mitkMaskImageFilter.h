/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKMASKIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22
#define MITKMASKIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22

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
class MaskImageFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(MaskImageFilter, ImageToImageFilter);

  itkNewMacro(Self);

  void SetMask( const mitk::Image* mask );
  const mitk::Image* GetMask() const;

protected:
  MaskImageFilter();

  ~MaskImageFilter();

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  template < typename TPixel, unsigned int VImageDimension >
    friend void _InternalComputeMask(itk::Image<TPixel, VImageDimension>* itkImage, mitk::MaskImageFilter* volumeCalculator);

  mitk::Image::Pointer m_Mask;
  mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
  mitk::ImageTimeSelector::Pointer m_MaskTimeSelector;
  mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

  //##Description 
  //## @brief Time when Header was last initialized
  itk::TimeStamp m_TimeOfHeaderInitialization;
};

} // namespace mitk

#endif /* MITKMASKIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22 */

