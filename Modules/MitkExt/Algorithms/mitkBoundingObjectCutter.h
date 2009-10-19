/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkImageToImageFilter.h"
#include "mitkBoundingObject.h"
#include "mitkImageTimeSelector.h"
#include "itkImage.h"


namespace mitk {

//##Documentation
//## @brief Cuts an Boundingobject out of an mitk Image
//## 
//## Input Parameters are a mitk::BoundingObject and optionally an mitk::Image
//## if no mitk::Image is provided, the resulting image will have m_InsideValue as pixelvalue on inside pixel, 
//## otherwise it will have the pixelvalue of the input image.
//## Pixel on the outside of the BoundingObject will have a pixelvalue of m_OutsideValue
//## \todo What Image resolution/spacing should be used, if no input image is given?
//## @ingroup Process
class MITKEXT_CORE_EXPORT BoundingObjectCutter : public ImageToImageFilter
{
public:
  mitkClassMacro(BoundingObjectCutter, ImageToImageFilter);
  itkNewMacro(Self);

  void SetBoundingObject( const mitk::BoundingObject* boundingObject );
  const mitk::BoundingObject* GetBoundingObject() const;
  //##Description 
  //## @brief Set for inside pixels, used when m_UseInsideValue is @a true
  itkSetMacro(InsideValue,  ScalarType);
  itkGetMacro(InsideValue,  ScalarType);
  //##Description 
  //## @brief Set value for outside pixels, used when m_AutoOutsideValue is \a false
  itkSetMacro(OutsideValue, ScalarType);
  itkGetMacro(OutsideValue, ScalarType);
  itkSetMacro(UseInsideValue, bool);
  itkGetMacro(UseInsideValue, bool);
  itkBooleanMacro(UseInsideValue);
  //##Description 
  //## @brief If set to \a true the minimum of the ouput pixel type is
  //## used as outside value.
  itkSetMacro(AutoOutsideValue, bool);
  itkGetMacro(AutoOutsideValue, bool);
  itkBooleanMacro(AutoOutsideValue);

  itkGetMacro(InsidePixelCount, unsigned int);
  itkGetMacro(OutsidePixelCount, unsigned int);

  itkSetMacro(UseWholeInputRegion,  bool);
  itkGetMacro(UseWholeInputRegion,  bool);

protected:
  BoundingObjectCutter();
  virtual ~BoundingObjectCutter();

  virtual const std::type_info& GetOutputPixelType();

  virtual void GenerateInputRequestedRegion();
  virtual void GenerateOutputInformation();
  virtual void GenerateData();

  template < typename TPixel, unsigned int VImageDimension, typename TOutputPixel > 
    friend void CutImageWithOutputTypeSelect( itk::Image< TPixel, VImageDimension >* inputItkImage, mitk::BoundingObjectCutter* cutter, int boTimeStep=0, TOutputPixel* dummy=NULL );
  template < typename TPixel, unsigned int VImageDimension > 
    friend void CutImage( itk::Image< TPixel, VImageDimension >* itkImage, mitk::BoundingObjectCutter* cutter, int boTimeStep );
  virtual void ComputeData(mitk::Image* input3D, int boTimeStep);

  //##Description 
  //## @brief BoundingObject that will be cut
  mitk::BoundingObject::Pointer m_BoundingObject; 
  //##Description 
  //## @brief Value for inside pixels, used when m_UseInsideValue is @a true
  //##
  //## \sa m_UseInsideValue
  ScalarType m_InsideValue;
  //##Description 
  //## @brief Value for outside pixels (default: 0)
  //##
  //## Used only if m_AutoOutsideValue is \a false.
  ScalarType m_OutsideValue;
  //##Description 
  //## @brief If \a true the minimum of the ouput pixel type is
  //## used as outside value (default: \a false)
  bool m_AutoOutsideValue;
  //##Description 
  //## @brief Use m_InsideValue for inside pixels (default: \a false)
  //##
  //## If @a true, pixels that are inside m_BoundingObject
  //## will get m_InsideValue in the cutting process
  //## If @a false, they keep their original value.
  //## \sa m_InsideValue
  bool m_UseInsideValue;

  unsigned int m_OutsidePixelCount;
  unsigned int m_InsidePixelCount;

  //##Description 
  //## @brief Region of input needed for cutting
  mitk::SlicedData::RegionType m_InputRequestedRegion;

  //##Description 
  //## @brief Time when Header was last initialized
  itk::TimeStamp m_TimeOfHeaderInitialization;

  mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
  mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

  bool m_UseWholeInputRegion;
};
} // namespace mitk

#endif /* BOUNDINGOBJECTCUTTER_H_HEADER_INCLUDED_C10B22CD */



