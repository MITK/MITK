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


#ifndef MITKEXTRUDEDCONTOUR_H_HEADER_INCLUDED
#define MITKEXTRUDEDCONTOUR_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
#include <mitkContour.h>

class vtkLinearExtrusionFilter;
class vtkPlanes;
class vtkClipPolyData;
class vtkLinearSubdivisionFilter;
class vtkTriangleFilter;
class vtkDecimatePro;
class vtkPolygon;

namespace mitk {

//##Documentation
//## @brief Data class containing a bounding-object created by
//## extruding a Contour along a vector
//## @ingroup Data
//##
//## The m_Contour is extruded in the direction m_Vector until
//## reaching m_ClippingGeometry.
class ExtrudedContour : public BoundingObject
{
public:
  mitkClassMacro(ExtrudedContour, BoundingObject);  
  itkNewMacro(Self);
  
  virtual mitk::ScalarType GetVolume();
  virtual bool IsInside(const Point3D& p) const;
  virtual void UpdateOutputInformation();

  itkGetConstObjectMacro(Contour, mitk::Contour);
  itkSetObjectMacro(Contour, mitk::Contour);

  mitkGetVectorMacro(Vector, mitk::Vector3D);
  mitkSetVectorMacro(Vector, mitk::Vector3D);

  itkGetConstObjectMacro(ClippingGeometry, mitk::Geometry3D);
  itkSetObjectMacro(ClippingGeometry, mitk::Geometry3D);

  virtual unsigned long GetMTime() const;
protected:
  ExtrudedContour();
  virtual ~ExtrudedContour();

  void CalculateExtrusion();

  mitk::Contour::Pointer m_Contour;
  mitk::Vector3D m_Vector;
  mitk::Geometry3D::Pointer m_ClippingGeometry;

  vtkPolygon* m_Polygon;
  float m_ProjectedContourBounds[6];
  float m_Origin[3];
  float m_Normal[3];

  vtkLinearExtrusionFilter* m_ExtrusionFilter;
  vtkTriangleFilter *m_TriangleFilter;
  vtkDecimatePro* m_Decimate;
  vtkLinearSubdivisionFilter* m_SubdivisionFilter;
  vtkPlanes* m_ClippingBox;
  vtkClipPolyData* m_ClipPolyDataFilter;

  itk::TimeStamp m_LastCalculateExtrusionTime;
};

}
#endif /* MITKEXTRUDEDCONTOUR_H_HEADER_INCLUDED */

