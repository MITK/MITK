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


#ifndef mitkGizmo_h
#define mitkGizmo_h

#include <mitkSurface.h>
#include <mitkDataNode.h>

#include <MitkGizmoExports.h>

namespace mitk {

class DataStorage;

//! A geometry manipulation "gizmo".
//!
//! This class represents the principal axes of some arbitrary BaseGeometry.
//!
//! The visualization shows the three axes x, y, and z along with a orthogonal ring around them.
//! In its center, a small sphere is visualized.
//!
//! The class is intended to be visualized along with another data object that "owns" the followed
//! BaseGeometry. The Gizmo will automatically update itself to all modifications to the
//! followed base geometry. Interactive modifications to the geometry can thus be easily visualized.
//!
//! The gizmo is definded by
//! - a center
//! - three axes for x, y, and z
//! - a radius
//!
//! The radius determines the size of the axes and the rings around them.
//!
//! A nice overview of similar / alternative representations can be found in
//! "Schmidt R, Singh K, and Balakrishnan R. Sketching and Composing Widgets for 3D Manipulation.
//!  EUROGRAPHICS 2008"
//!
//! \sa GizmoInteractor3D
class MITKGIZMO_EXPORT Gizmo : public Surface
{
public:

  //! Names for the three axes
  enum AxisType
  {
      AxisX,
      AxisY,
      AxisZ
  };

  //! Names for the different parts of the gizmo
  enum HandleType
  {
      MoveFreely, //< the central sphere
      MoveAlongAxisX,
      MoveAlongAxisY,
      MoveAlongAxisZ,
      RotateAroundAxisX,
      RotateAroundAxisY,
      RotateAroundAxisZ,
      ScaleX,
      ScaleY,
      ScaleZ,
      NoHandle //< to indicate picking failure
  };

  //! Conversion for any kind of logging/debug/... purposes
  static std::string HandleTypeToString(HandleType type);

  mitkClassMacro(Gizmo, Surface);
  itkNewMacro(Gizmo);

  itkGetMacro(Center, Point3D);
  itkSetMacro(Center, Point3D);

  itkGetMacro(AxisX, Vector3D);
  itkSetMacro(AxisX, Vector3D);

  itkGetMacro(AxisY, Vector3D);
  itkSetMacro(AxisY, Vector3D);

  itkGetMacro(AxisZ, Vector3D);
  itkSetMacro(AxisZ, Vector3D);

  itkGetMacro(Radius, Vector3D);
  itkSetMacro(Radius, Vector3D);

  //! Updates the representing surface object after changes to center, axes, or radius.
  void UpdateRepresentation();

  //! Setup the gizmo to follow any ModifiedEvents of the given geometry.
  //! The object will adapt and update itself in function of the geometry's changes.
  void FollowGeometry(BaseGeometry* geom);

  //! The ITK callback to receive modified events of the followed geometry
  void OnFollowedGeometryModified();

  //! Determine the nature of the the given vertex id.
  //! Can be used after picking a vertex id to determine what part of the
  //! gizmo has been picked.
  HandleType GetHandleFromPointID(vtkIdType id);

  //! Convenience creation of a gizmo for given node
  //! \param node The node holding the geometry to be visualized
  //! \param storage The DataStorage where a node holding the gizmo
  //!                shall be added to (ignored when nullptr)
  //!
  //! \return DataNode::Pointer containing the node used for vizualization of our gizmo
  static DataNode::Pointer AddGizmoToNode(DataNode* node, DataStorage* storage);

protected:
  Gizmo();
  virtual ~Gizmo();

  Gizmo(const Gizmo&) = delete;
  Gizmo& operator=(const Gizmo&) = delete;

  //! Creates a vtkPolyData representing the parameters defining the gizmo.
  vtkSmartPointer<vtkPolyData> BuildGizmo();

private:

  Point3D m_Center;
  Vector3D m_AxisX;
  Vector3D m_AxisY;
  Vector3D m_AxisZ;
  Vector3D m_Radius;

  BaseGeometry::Pointer m_FollowedGeometry;
  //! ITK tag for the observing of m_FollowedGeometry
  unsigned long m_FollowerTag;
};

}
#endif /* BOUNDINGOBJECT_H_HEADER_INCLUDED */
