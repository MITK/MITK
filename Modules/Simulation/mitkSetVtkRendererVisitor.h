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

#ifndef mitkSetVtkRendererVisitor_h
#define mitkSetVtkRendererVisitor_h

#include <mitkVtkModel.h>
#include <sofa/simulation/common/Visitor.h>
#include <MitkSimulationExports.h>

class vtkRenderer;

namespace mitk
{
  class MitkSimulation_EXPORT SetVtkRendererVisitor : public sofa::simulation::Visitor
  {
  public:
    explicit SetVtkRendererVisitor(vtkRenderer* renderer, const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    SetVtkRendererVisitor(vtkRenderer* renderer, const Point3D& planePoint, const Vector3D& planeNormal, ScalarType planeThickness = 1.0, const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    ~SetVtkRendererVisitor();

    Result processNodeTopDown(sofa::simulation::Node* node);

  private:
    SetVtkRendererVisitor(const SetVtkRendererVisitor&);
    SetVtkRendererVisitor& operator=(const SetVtkRendererVisitor&);

    void processVisualModel(sofa::simulation::Node*, sofa::core::visual::VisualModel* visualModel);

    vtkRenderer* m_VtkRenderer;
    VtkModel::RenderingMode m_RenderingMode;
    Point3D m_PlanePoint;
    Vector3D m_PlaneNormal;
    ScalarType m_PlaneThickness;
  };
}

#endif
