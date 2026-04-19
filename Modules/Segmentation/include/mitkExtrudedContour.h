/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtrudedContour_h
#define mitkExtrudedContour_h

#include <mitkBoundingObject.h>
#include <MitkSegmentationExports.h>
#include <mitkContour.h>
#include <mitkPlaneGeometry.h>
#include <vtkVersionMacros.h>

class vtkLinearExtrusionFilter;
class vtkPlanes;
class vtkClipPolyData;
class vtkLinearSubdivisionFilter;
class vtkTriangleFilter;
class vtkDecimatePro;
class vtkPolygon;

namespace mitk
{
  /**
   * \brief Data class containing a bounding-object created by extruding a Contour along a vector.
   *
   * The m_Contour is extruded in the direction m_Vector until
   * reaching m_ClippingGeometry.
   *
   * \ingroup Data
   */

  class MITKSEGMENTATION_EXPORT ExtrudedContour : public BoundingObject
  {
  public:
    mitkClassMacro(ExtrudedContour, BoundingObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      mitk::ScalarType GetVolume() override;
    bool IsInside(const Point3D &p) const override;
    void UpdateOutputInformation() override;

    /** \brief Contour to extrude. */
    itkGetConstObjectMacro(Contour, mitk::Contour);
    /** \brief Sets the contour to extrude. */
    itkSetObjectMacro(Contour, mitk::Contour);

    /** \brief Vector to specify the direction of the extrusion. */
    itkGetConstReferenceMacro(Vector, mitk::Vector3D);
    /** \brief Sets the direction vector for extrusion. */
    itkSetMacro(Vector, mitk::Vector3D);
    itkGetConstMacro(AutomaticVectorGeneration, bool);
    itkSetMacro(AutomaticVectorGeneration, bool);
    itkBooleanMacro(AutomaticVectorGeneration);

    /** \brief Optional vector to specify the orientation of the bounding-box. */
    itkGetConstReferenceMacro(RightVector, mitk::Vector3D);
    /** \brief Sets the optional orientation vector. */
    itkSetMacro(RightVector, mitk::Vector3D);

    /** \brief Optional geometry for clipping the extruded contour. */
    itkGetConstObjectMacro(ClippingGeometry, mitk::BaseGeometry);
    /** \brief Sets the clipping geometry. */
    itkSetObjectMacro(ClippingGeometry, mitk::BaseGeometry);

    itk::ModifiedTimeType GetMTime() const override;

  protected:
    ExtrudedContour();
    ~ExtrudedContour() override;

    void BuildSurface();
    void BuildGeometry();

    mitk::Contour::Pointer m_Contour;
    mitk::Vector3D m_Vector;
    mitk::Vector3D m_RightVector;
    mitk::BaseGeometry::Pointer m_ClippingGeometry;

    bool m_AutomaticVectorGeneration;

    vtkPolygon *m_Polygon;

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION == 4) && (VTK_MINOR_VERSION >= 4)))
    double m_ProjectedContourBounds[6];
#else
    float m_ProjectedContourBounds[6];
#endif

    mitk::PlaneGeometry::Pointer m_ProjectionPlane;
    /** \brief For fast projection on plane. */
    float m_Right[3];
    float m_Down[3];

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION == 4) && (VTK_MINOR_VERSION >= 4)))
    double m_Normal[3];
#else
    float m_Normal[3];
#endif
    float m_Origin[3];

    vtkLinearExtrusionFilter *m_ExtrusionFilter;
    vtkTriangleFilter *m_TriangleFilter;
    vtkDecimatePro *m_Decimate;
    vtkLinearSubdivisionFilter *m_SubdivisionFilter;
    vtkPlanes *m_ClippingBox;
    vtkClipPolyData *m_ClipPolyDataFilter;

    itk::TimeStamp m_LastCalculateExtrusionTime;
  };
}
#endif
