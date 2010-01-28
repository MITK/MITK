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


#ifndef PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22
#define PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkImageToImageFilter.h"
#include "mitkPlaneGeometry.h"

namespace itk
{
  template <class TPixel, unsigned int VImageDimension> class ITK_EXPORT Image;
}

namespace mitk {

/**
  \brief Filter to cut an image with a plane.

        Everything in the direction of the normal
        of the planes (if fill mode is set to "FILL") will be set to a specified value.
 */
class MitkExt_EXPORT PlaneCutFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(PlaneCutFilter, ImageToImageFilter);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef enum {FILL, FILL_INVERSE} FillMode;

  //##Documentation
  //## @brief Set background grey level
  itkSetMacro(BackgroundLevel, float);
  itkGetMacro(BackgroundLevel, float);

  itkSetMacro(FillMode, FillMode);
  itkGetMacro(FillMode, FillMode);

  itkSetObjectMacro(Plane, const PlaneGeometry);
  itkGetObjectMacro(Plane, const PlaneGeometry);
protected:
  float m_BackgroundLevel;
  PlaneGeometry::ConstPointer m_Plane;
  FillMode m_FillMode;

  PlaneCutFilter();
  ~PlaneCutFilter();
  virtual void GenerateData();

  template <typename TPixel, unsigned int VImageDimension>
  void _computeIntersection(itk::Image<TPixel, VImageDimension> *itkImage, const PlaneGeometry *plane, const Geometry3D *geometry);
};

} // namespace mitk

#endif /* PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22 */


