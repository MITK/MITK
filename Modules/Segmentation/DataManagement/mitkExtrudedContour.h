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


#ifndef MITKEXTRUDEDCONTOUR_H_HEADER_INCLUDED
#define MITKEXTRUDEDCONTOUR_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
#include <MitkSegmentationExports.h>
#include <mitkContour.h>
#include <vtkConfigure.h>
#include <mitkPlaneGeometry.h>
#include <vtkVersionMacros.h>

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
//##
//## The m_Contour is extruded in the direction m_Vector until
//## reaching m_ClippingGeometry.
//## @ingroup Data
class MitkSegmentation_EXPORT ExtrudedContour : public BoundingObject
{
public:
  mitkClassMacro(ExtrudedContour, BoundingObject);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual mitk::ScalarType GetVolume();
  virtual bool IsInside(const Point3D& p) const;
  virtual void UpdateOutputInformation();

  //##Documentation
  //## @brief Contour to extrude
  itkGetConstObjectMacro(Contour, mitk::Contour);
  itkSetObjectMacro(Contour, mitk::Contour);

  //##Documentation
  //## @brief Vector to specify the direction of the extrusion
  mitkGetVectorMacro(Vector, mitk::Vector3D);
  mitkSetVectorMacro(Vector, mitk::Vector3D);
  itkGetConstMacro(AutomaticVectorGeneration, bool);
  itkSetMacro(AutomaticVectorGeneration, bool);
  itkBooleanMacro(AutomaticVectorGeneration);

  //##Documentation
  //## @brief Optional vector to specify the orientation of the bounding-box
  mitkGetVectorMacro(RightVector, mitk::Vector3D);
  mitkSetVectorMacro(RightVector, mitk::Vector3D);

  //##Documentation
  //## @brief Optional geometry for clipping the extruded contour
  itkGetConstObjectMacro(ClippingGeometry, mitk::BaseGeometry);
  itkSetObjectMacro(ClippingGeometry, mitk::BaseGeometry);

  virtual unsigned long GetMTime() const;
protected:
  ExtrudedContour();
  virtual ~ExtrudedContour();

  void BuildSurface();
  void BuildGeometry();

  mitk::Contour::Pointer m_Contour;
  mitk::Vector3D m_Vector;
  mitk::Vector3D m_RightVector;
  mitk::BaseGeometry::Pointer m_ClippingGeometry;

  bool m_AutomaticVectorGeneration;

  vtkPolygon* m_Polygon;

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double m_ProjectedContourBounds[6];
#else
  float m_ProjectedContourBounds[6];
#endif

  mitk::PlaneGeometry::Pointer m_ProjectionPlane;
  //##Documentation
  //## @brief For fast projection on plane
  float m_Right[3];
  float m_Down[3];

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double m_Normal[3];
#else
  float m_Normal[3];
#endif
  float m_Origin[3];

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


