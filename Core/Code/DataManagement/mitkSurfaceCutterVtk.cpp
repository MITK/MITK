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

#include "mitkSurfaceCutterVtk.h"

#include <vtkCutter.h>
#include <vtkPlane.h>
#include <mitkVector.h>

namespace mitk {

class SurfaceCutterVtkPrivate {
public:
    SurfaceCutterVtkPrivate() 
        : _plane(vtkPlane::New())
        , _cutter(vtkCutter::New())
    {
        _cutter->SetCutFunction(_plane);
    }

    void setPolyData(vtkPolyData* surface)
    {
        _surface = surface;
        _cutter->SetInputData(_surface);
    }

    vtkSmartPointer<vtkPolyData> cutWithPlane(const mitk::Point3D planePoints[4]) const
    {
        if (!_surface) {
            return vtkSmartPointer<vtkPolyData>::New();
        }

        double vtkOrigin[3];
        planePoints[0].ToArray(vtkOrigin);
        _plane->SetOrigin(vtkOrigin);

        mitk::Vector3D normal = itk::CrossProduct(planePoints[1] - planePoints[0], planePoints[2] - planePoints[0]);
        normal.Normalize();
        double vtkNormal[3];
        normal.ToArray(vtkNormal);
        _plane->SetNormal(vtkNormal);

        _cutter->Update();

        return _cutter->GetOutput();
    }

private:
    vtkSmartPointer<vtkPlane> _plane;
    vtkSmartPointer<vtkCutter> _cutter;
    vtkSmartPointer<vtkPolyData> _surface;
};


SurfaceCutterVtk::SurfaceCutterVtk()
: p(new SurfaceCutterVtkPrivate())
{
}

SurfaceCutterVtk::~SurfaceCutterVtk()
{
}

void SurfaceCutterVtk::setPolyData(vtkPolyData* surface)
{
    p->setPolyData(surface);
}

vtkSmartPointer<vtkPolyData> SurfaceCutterVtk::cutWithPlane(const mitk::Point3D planePoints[4]) const
{
    return p->cutWithPlane(planePoints);
}

} // namespace mitk