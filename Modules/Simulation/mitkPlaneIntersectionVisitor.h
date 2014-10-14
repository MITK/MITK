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

#ifndef mitkPlaneIntersectionVisitor_h
#define mitkPlaneIntersectionVisitor_h

#include <mitkPoint.h>
#include <mitkVector.h>
#include <sofa/simulation/common/Visitor.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MitkSimulation_EXPORT PlaneIntersectionVisitor : public sofa::simulation::Visitor
  {
  public:
    PlaneIntersectionVisitor(const Point3D& point, const Vector3D& normal, const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    ~PlaneIntersectionVisitor();

    Result processNodeTopDown(sofa::simulation::Node* node);

  private:
    PlaneIntersectionVisitor(const PlaneIntersectionVisitor&);
    PlaneIntersectionVisitor& operator=(const PlaneIntersectionVisitor&);

    void processVisualModel(sofa::simulation::Node*, sofa::core::visual::VisualModel* visualModel);

    Point3D m_Point;
    Vector3D m_Normal;
  };
}

#endif
