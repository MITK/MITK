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


#ifndef BOUNDINGOBJECTCUTTER_H_HEADER_INCLUDED_C10B22CD
#define BOUNDINGOBJECTCUTTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "mitkBoundingObject.h"
#include <mitkImageToImageFilter.h>
#include "itkImage.h"
#include "itkImageRegionIteratorWithIndex.h"

namespace mitk {

//##Documentation
//## @brief Cuts an Boundingobject out of an mitk Image
//## @ingroup Process
//## 
//## Input Parameters are a mitk::BoundingObject and optionally an mitk::Image
//## if no mitk::Image is provided, the resulting image will have m_InsideValue as pixelvalue on inside pixel, 
//## otherwise it will have the pixelvalue of the input image.
//## Pixel on the outside of the BoundingObject will have a pixelvalue of m_OutsideValue
//## \todo What Image resolution/spacing should be used, if no input image is given?
template <typename TPixel> class BoundingObjectCutter : public ImageToImageFilter
{
public:
  mitkClassMacro(BoundingObjectCutter, ImageToImageFilter);
  itkNewMacro(Self);

  typedef TPixel PixelType;
  typedef itk::Image<PixelType, 3> ItkImageType;
  typedef typename ItkImageType::RegionType ItkRegionType;
  typedef itk::ImageRegionIteratorWithIndex< ItkImageType > ItkImageIteratorType;

  virtual void GenerateOutputInformation();
  virtual void GenerateData();

  void SetBoundingObject( const mitk::BoundingObject* boundingObject );
  const mitk::BoundingObject* GetBoundingObject() const;
  itkSetMacro(InsideValue,  PixelType);
  itkGetMacro(InsideValue,  PixelType);
  itkSetMacro(OutsideValue, PixelType);
  itkGetMacro(OutsideValue, PixelType);
  itkSetMacro(UseInsideValue, bool);
  itkGetMacro(UseInsideValue, bool);
  itkBooleanMacro(UseInsideValue);
  itkGetMacro(InsidePixelCount, unsigned int);
  itkGetMacro(OutsidePixelCount, unsigned int);

protected:
  BoundingObjectCutter();
  virtual ~BoundingObjectCutter();

  mitk::BoundingObject::Pointer m_BoundingObject; // BoundingObject that will be cut
  PixelType m_InsideValue;                        // Value for inside pixels, if UseInsideValue is true
  PixelType m_OutsideValue;                       // Value for outside pixels
  bool m_UseInsideValue;                          // if true, pixels that are inside m_BoundingObject 
                                                  // will get m_InsideValue in the cutting process 
                                                  // (else they keep their original value)
  unsigned int m_OutsidePixelCount;
  unsigned int m_InsidePixelCount;
};
} // namespace mitk

#include "mitkBoundingObjectCutter.txx"  // because it is a template

#endif /* BOUNDINGOBJECTCUTTER_H_HEADER_INCLUDED_C10B22CD */
