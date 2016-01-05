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
#include <MitkGizmoExports.h>

namespace mitk {

class DataNode;
class DataStorage;

class MITKGIZMO_EXPORT Gizmo : public Surface
{
public:
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

  itkGetMacro(Scaling, Vector3D);
  itkSetMacro(Scaling, Vector3D);

  void UpdateRepresentation();

  void FollowGeometry(BaseGeometry* geom);

  void OnFollowedGeometryModified();

  static void AddGizmoToNode(DataNode* node, DataStorage* storage);

protected:
  Gizmo();
  virtual ~Gizmo();

  Gizmo(const Gizmo&) = delete;
  Gizmo& operator=(const Gizmo&) = delete;


  vtkSmartPointer<vtkPolyData> BuildGizmo();

private:

  Point3D m_Center;
  Vector3D m_AxisX;
  Vector3D m_AxisY;
  Vector3D m_AxisZ;
  Vector3D m_Scaling;

  BaseGeometry::Pointer m_FollowedGeometry;
};

}
#endif /* BOUNDINGOBJECT_H_HEADER_INCLUDED */
