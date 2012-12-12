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

#ifndef mitkSimulationDrawTool_h
#define mitkSimulationDrawTool_h

#include <SimulationExports.h>
#include <sofa/core/visual/DrawTool.h>

class vtkActor;
class vtkObjectBase;
class vtkProperty;

namespace mitk
{
  class Simulation_EXPORT SimulationDrawTool : public sofa::core::visual::DrawTool
  {
  public:
    SimulationDrawTool();
    ~SimulationDrawTool();

    void DisableUpdate();
    std::vector<vtkActor*> GetActors() const;
    void Reset();

    void drawPoints(const std::vector<Vector3>& points, float pointSize, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawLines(const std::vector<Vector3>& points, float lineWidth, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawLines(const std::vector<Vector3>& points, const std::vector<Vec2i>& indices, float lineWidth, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawTriangles(const std::vector<Vector3>& points, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawTriangles(const std::vector<Vector3>& points, const Vector3 normal, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawTriangles(const std::vector<Vector3>& points, const std::vector<Vec3i>& indices, const std::vector<Vector3>& normals, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawTriangles(const std::vector<Vector3>& points, const std::vector<Vector3>& normals, const std::vector<Vec4f>& colors);
    void drawTriangleStrip(const std::vector<Vector3>& points, const std::vector<Vector3>& normals, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawTriangleFan(const std::vector<Vector3>& points, const std::vector<Vector3>& normals, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawFrame(const Vector3& position, const Quaternion& orientation, const Vec3f& size);
    void drawSpheres(const std::vector<Vector3>& points, const std::vector<float>& radii, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawSpheres(const std::vector<Vector3>& points, float radius, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawCone(const Vector3& point1, const Vector3& point2, float radius1, float radius2, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f), int subdivisions = 16);
    void drawCube(const float& edgeRadius, const Vec4f& color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f), const int& subdivisions = 16);
    void drawCylinder(const Vector3& point1, const Vector3& point2, float radius, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f), int subdivisions = 16);
    void drawArrow(const Vector3& point1, const Vector3& point2, float radius, const Vec4f color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f), int subdivisions = 16);
    void drawPlus(const float& edgeRadius, const Vec4f& color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f), const int& subdivisions = 16);
    void drawPoint(const Vector3& position, const Vec4f& color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawPoint(const Vector3& position, const Vector3& normal, const Vec4f& color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawTriangle(const Vector3& point1, const Vector3& point2, const Vector3& point3, const Vector3& normal, const Vec4f& color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    void drawTriangle(const Vector3& point1, const Vector3& point2, const Vector3& point3, const Vector3& normal, const Vec4f& color1, const Vec4f& color2, const Vec4f& color3);
    void drawSphere(const Vector3& position, float radius);

    void pushMatrix();
    void popMatrix();
    void multMatrix(float* matrix);
    void scale(float factor);

    void setMaterial(const Vec4f& color, std::string name = "");
    void resetMaterial(const Vec4f& color, std::string name = "");
    void setPolygonMode(int mode, bool wireframe);
    void setLightingEnabled(bool isEnabled);

    void clear();

  private:
    SimulationDrawTool(const SimulationDrawTool&);
    SimulationDrawTool& operator=(const SimulationDrawTool&);

    void DeleteVtkObjects();
    void InitProperty(vtkProperty* property) const;

    std::vector<vtkObjectBase*> m_VtkObjects;
    std::vector<vtkActor*> m_Actors;
    int m_PolygonMode;
    bool m_Wireframe;
    bool m_Update;
  };
}

#endif
