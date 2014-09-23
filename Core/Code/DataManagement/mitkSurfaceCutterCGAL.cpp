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

#include "mitkSurfaceCutterCGAL.h"
#include <mitkVector.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/bounding_box.h>

#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCell.h>
#include <vtkCleanPolyData.h>
#include <vtkStripper.h>
#include <vtkTriangleFilter.h>

// Definitions of CGAL entities
typedef CGAL::Simple_cartesian<double> CGAL_Kernel;
typedef CGAL_Kernel::Point_3 CGAL_Point;
typedef CGAL_Kernel::Vector_3 CGAL_Vector;
typedef CGAL_Kernel::Plane_3 CGAL_Plane;
typedef CGAL_Kernel::Segment_3 CGAL_Segment;
typedef CGAL_Kernel::Triangle_3 CGAL_Triangle;

// Definition of AABB tree types
typedef CGAL::AABB_triangle_primitive<CGAL_Kernel, std::vector<CGAL_Triangle>::iterator> CGAL_AABBPrimitive;
typedef CGAL::AABB_traits<CGAL_Kernel, CGAL_AABBPrimitive> CGAL_AABBTraits;
typedef CGAL::AABB_tree<CGAL_AABBTraits> CGAL_AABBTree;

namespace mitk {

class SurfaceCutterCGALPrivate {
public:
    SurfaceCutterCGALPrivate()
        : _tree(nullptr)
    {
    }

    void setPolyData(vtkPolyData* surface)
    {
        if (_tree) {
            _tree.release();
            _triangles.clear();
        }

        if (surface) {
            // Triangulate data
            vtkSmartPointer<vtkTriangleFilter> triangleFilter =
                vtkSmartPointer<vtkTriangleFilter>::New();
            triangleFilter->SetInputData(surface);
            triangleFilter->Update();

            vtkPolyData* polyData = triangleFilter->GetOutput();

            _triangles.reserve(polyData->GetNumberOfCells());
            for (int cellId = 0; cellId < polyData->GetNumberOfCells(); ++cellId) {
                vtkCell* cell = polyData->GetCell(cellId);

                CGAL_Point p[3];
                for (int i = 0; i < 3; ++i) {
                    double* pt = polyData->GetPoints()->GetPoint(cell->GetPointId(i));
                    p[i] = CGAL_Point(pt[0], pt[1], pt[2]);
                }

                // Convert triangle to CGAL
                _triangles.push_back(CGAL_Triangle(p[0], p[1], p[2]));
            }

            // Build the AABB tree
            _tree.reset(new CGAL_AABBTree(_triangles.begin(), _triangles.end()));
            _tree->build();
        }
    }

    vtkSmartPointer<vtkPolyData> cutWithPlane(const mitk::Point3D planePoints[4]) const
    {
        if (!_tree) {
            return vtkSmartPointer<vtkPolyData>::New();
        }

        // This constant allows to avoid numerical instabilities in Simple_cartesian kernel
        // It is used to avoid planes with normals parallel to coordinate axes, which is
        // often the case for non-rotated slicing directions in MITK
        static const double OFFSET_EPSILPON = 1e-8;

        CGAL_Point points[] = {
            CGAL_Point(planePoints[0][0] - OFFSET_EPSILPON, planePoints[0][1] - OFFSET_EPSILPON, planePoints[0][2] - OFFSET_EPSILPON),
            CGAL_Point(planePoints[1][0] + OFFSET_EPSILPON, planePoints[1][1] + OFFSET_EPSILPON, planePoints[1][2] + OFFSET_EPSILPON),
            CGAL_Point(planePoints[2][0], planePoints[2][1], planePoints[2][2]),
            CGAL_Point(planePoints[3][0], planePoints[3][1], planePoints[3][2])
        };

        CGAL::Bbox_3 bbox = points[0].bbox() + points[1].bbox() + points[2].bbox() + points[3].bbox();
        
        std::vector<CGAL_AABBTree::Intersection_and_primitive_id<CGAL_Plane>::Type> segments;

        if (_tree->do_intersect(bbox)) {
            mitk::Vector3D normal = itk::CrossProduct(planePoints[1] - planePoints[0], planePoints[2] - planePoints[0]);
            normal.Normalize();
            
            CGAL_Plane query_plane(points[0], CGAL_Vector(normal[0], normal[1], normal[2]));
            _tree->all_intersections(query_plane, std::back_inserter(segments));
        }

        vtkSmartPointer<vtkPolyData> pdOut = vtkSmartPointer<vtkPolyData>::New();
        vtkSmartPointer<vtkPoints> pointsOut = vtkSmartPointer<vtkPoints>::New();
        pdOut->SetPoints(pointsOut);
        pointsOut->Allocate(2 * segments.size());
        pdOut->Allocate(segments.size());

        for (int i = 0; i < segments.size(); ++i) {
            CGAL_Segment* s = boost::get<CGAL_Segment>(&segments[i].first);

            if (!s) {
                // Ignore non-segment intersections
                continue;
            }

            pointsOut->InsertNextPoint(s->start().x(), s->start().y(), s->start().z());
            pointsOut->InsertNextPoint(s->end().x(), s->end().y(), s->end().z());

            vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
            ids->InsertNextId(2 * i);
            ids->InsertNextId(2 * i + 1);
            pdOut->InsertNextCell(VTK_LINE, ids);
        }

        vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
        cleaner->SetInputData(pdOut);

        vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
        stripper->SetInputConnection(cleaner->GetOutputPort());
        stripper->Update();

        return stripper->GetOutput();
    }

private:
    std::unique_ptr<CGAL_AABBTree> _tree;
    std::vector<CGAL_Triangle> _triangles;
};


SurfaceCutterCGAL::SurfaceCutterCGAL()
: p(new SurfaceCutterCGALPrivate())
{
}

SurfaceCutterCGAL::~SurfaceCutterCGAL()
{
}

void SurfaceCutterCGAL::setPolyData(vtkPolyData* surface)
{
    p->setPolyData(surface);
}

vtkSmartPointer<vtkPolyData> SurfaceCutterCGAL::cutWithPlane(const mitk::Point3D planePoints[4]) const
{
    return p->cutWithPlane(planePoints);
}

} // namespace mitk
