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


#ifndef MITKGEOMETRYCLIPIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22
#define MITKGEOMETRYCLIPIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkGeometryData.h"

namespace itk {
template <class TPixel, unsigned int VImageDimension> class ITK_EXPORT Image;
}

namespace mitk {

//##Documentation
//## @brief Filter for clipping an image with a Geometry2D
//##
//## The given geometry for clipping can be either a Geometry2D
//## or a TimeSlicedGeometry containing multiple instances
//## of Geometry2D
//##
//## \TODO add AutoOrientLabels, which makes the "left" side (minimum X value) side of the image get one defined label.
//##       left-most because vtkPolyDataNormals uses the same definition and this filter is used for visualization of
//##       front/back side of curved planes
//##
//## @ingroup Process
class MITK_CORE_EXPORT GeometryClipImageFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(GeometryClipImageFilter, ImageToImageFilter);

  itkNewMacro(Self);

  //##Description 
  //## Set the geometry to be used for clipping 
  //##
  //## The given geometry for clipping can be either a Geometry2D
  //## or a TimeSlicedGeometry containing multiple instances
  //## of Geometry2D
  void SetClippingGeometry(const mitk::Geometry3D* aClippingGeometry);
  const mitk::Geometry3D* GetClippingGeometry() const;

  //##Description 
  //## @brief Get whether the part above or below the geometry 
  //## shall be clipped (default: @a true)
  itkGetConstMacro(ClipPartAboveGeometry, bool);
  //## @brief Set whether the part above or below the geometry 
  //## shall be clipped (default: @a true)
  itkSetMacro(ClipPartAboveGeometry, bool);
  //## @brief Set whether the part above or below the geometry 
  //## shall be clipped (default: @a true)
  itkBooleanMacro(ClipPartAboveGeometry);

  //##Description 
  //## @brief Set value for outside pixels (default: 0), 
  //## used when m_AutoOutsideValue is \a false
  itkSetMacro(OutsideValue, ScalarType);
  itkGetConstMacro(OutsideValue, ScalarType);

  //##Description 
  //## @brief If set to \a true the minimum of the ouput pixel type is
  //## used as outside value (default: \a false)
  itkSetMacro(AutoOutsideValue, bool);
  itkGetConstMacro(AutoOutsideValue, bool);
  itkBooleanMacro(AutoOutsideValue);
  
  itkSetMacro(AutoOrientLabels, bool);
  itkGetConstMacro(AutoOrientLabels, bool);
  
  //##Description 
  //## @brief If set to \a true both sides of the clipping
  //## geometry will be labeld using m_AboveGeometryLabel and
  //## m_BelowGeometryLabel
  itkSetMacro(LabelBothSides, bool);
  itkGetConstMacro(LabelBothSides, bool);
  itkBooleanMacro(LabelBothSides);
  
  //##Description 
  //## @brief Set for voxels above the clipping geometry.
  //## This value is only used, if m_LabelBothSides is set to true.
  itkSetMacro(AboveGeometryLabel, ScalarType);
  itkGetConstMacro(AboveGeometryLabel, ScalarType);
  
  //##Description 
  //## @brief Set for voxels below the clipping geometry.
  //## This value is only used, if m_LabelBothSides is set to true.
  itkSetMacro(BelowGeometryLabel, ScalarType);
  itkGetConstMacro(BelowGeometryLabel, ScalarType);
  
  protected:
  GeometryClipImageFilter();

  ~GeometryClipImageFilter();

  virtual void GenerateInputRequestedRegion();

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  template < typename TPixel, unsigned int VImageDimension >
    friend void _InternalComputeClippedImage(itk::Image<TPixel, VImageDimension>* itkImage, mitk::GeometryClipImageFilter* geometryClipper, const mitk::Geometry2D* clippingGeometry2D);

  mitk::Geometry3D::ConstPointer m_ClippingGeometry;
  mitk::GeometryData::Pointer m_ClippingGeometryData;
  mitk::TimeSlicedGeometry::ConstPointer m_TimeSlicedClippingGeometry;
  mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
  mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;

  //##Description 
  //## @brief Defines whether the part above or below the geometry 
  //## shall be clipped (default: @a true)
  bool m_ClipPartAboveGeometry;

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
  //## @brief If \a true all pixels above and below the geometry
  //## are labeled with m_AboveGeometryLabel and m_BelowGeometryLabel
  bool m_LabelBothSides;

  /**
   * \brief Orient above like vtkPolyDataNormals does with AutoOrientNormals
   */
  bool m_AutoOrientLabels;
  
  //##Description 
  //## @brief Is used for labeling all pixels above the geometry
  //## when m_LabelBothSides is on
  ScalarType m_AboveGeometryLabel;
  
  //##Description 
  //## @brief Is used for labeling all pixels below the geometry
  //## when m_LabelBothSides is on
  ScalarType m_BelowGeometryLabel;

  //##Description 
  //## @brief Time when Header was last initialized
  itk::TimeStamp m_TimeOfHeaderInitialization;
};

} // namespace mitk

#endif /* MITKGEOMETRYCLIPIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22 */


