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


#ifndef MITKPADIMAGEFILTER_H_HEADER_INCLUDED
#define MITKPADIMAGEFILTER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"

#include "itkImage.h"
#include "mitkImageAccessByItk.h"

//namespace itk {
  //template <class TPixel, unsigned int VImageDimension> class ITK_EXPORT Image;
//}


namespace mitk {

//##Documentation
//## @brief PadImageFilter class pads the first input image to the size of the second input image.
//##
//## Two Images have to be set.
//## The first image is the image to pad. The second image defines the pad size.
//## @ingroup Process
class PadImageFilter : public ImageToImageFilter
{
public:    
  mitkClassMacro(PadImageFilter, ImageToImageFilter);

  itkNewMacro(Self);  

  //itkSetMacro(treeIterator, mitk::DataTreeIteratorBase*);

protected:
  PadImageFilter();
  
  virtual ~PadImageFilter();
  
  //virtual void GenerateInputRequestedRegion();
  
  //virtual void GenerateOutputInformation();

  virtual void GenerateData();

  //template < typename TPixel, unsigned int VImageDimension >
    //friend void PadImage(itk::Image< TPixel, VImageDimension >* inputItkImage, mitk::Image::ConstPointer m_origImage);

  //mitk::DataTreeIteratorBase* m_treeIterator;
  //mitk::Image::Pointer m_Image;
  //mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
  //mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

  //mitk::Image::Pointer TransformSegmentationCutResult(mitk::Image::ConstPointer origImage, mitk::PadImageFilter* bosiToErisFilter);
};

} // namespace mitk

#endif /* MITKPADIMAGEFILTER_H_HEADER_INCLUDED */
