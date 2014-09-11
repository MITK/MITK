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


#ifndef MITKGEOMETRYCLIPIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22
#define MITKGEOMETRYCLIPIMAGEFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "MitkAlgorithmsExtExports.h"
#include "mitkImageToImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkGeometryData.h"

namespace itk {
template <class TPixel, unsigned int VImageDimension> class ITK_EXPORT Image;
}

namespace mitk {

//##Documentation
//## @brief Filter for clipping an image with a PlaneGeometry
//##
//## The given geometry for clipping can be either a PlaneGeometry
//## or a TimeGeometry containing multiple instances
//## of PlaneGeometry
//##
//## \todo add AutoOrientLabels, which makes the "left" side (minimum X value) side of the image get one defined label.
//##       left-most because vtkPolyDataNormals uses the same definition and this filter is used for visualization of
//##       front/back side of curved planes
//##
//## @ingroup Process
class MitkAlgorithmsExt_EXPORT GeometryClipImageFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(GeometryClipImageFilter, ImageToImageFilter);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
  * Set the geometry to be used for clipping
  *
  * The given geometry for clipping must be a PlaneGeometry.
  */
  void SetClippingGeometry(const mitk::BaseGeometry* aClippingGeometry);

  /**
  * Set the geometry to be used for clipping
  *
  * The given geometry for clipping must a
  * TimeGeometry containing multiple instances
  * of PlaneGeometry
  */
  void SetClippingGeometry(const mitk::TimeGeometry* aClippingGeometry);

  const mitk::BaseGeometry* GetClippingGeometry() const;
  const mitk::TimeGeometry* GetClippingTimeGeometry() const;

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
    friend void _InternalComputeClippedImage(itk::Image<TPixel, VImageDimension>* itkImage, mitk::GeometryClipImageFilter* geometryClipper, const mitk::PlaneGeometry* clippingPlaneGeometry);

  mitk::BaseGeometry::ConstPointer m_ClippingGeometry;
  mitk::GeometryData::Pointer m_ClippingGeometryData;
  mitk::TimeGeometry::ConstPointer m_TimeClippingGeometry;
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
