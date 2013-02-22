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

#include "mitkPlanarFigureMapper3D.h"
#include <mitkProperties.h>
#include <mitkPlaneGeometry.h>

#include <vtkPointData.h>
#include <vtkProperty.h>

mitk::PlanarFigureMapper3D::PlanarFigureMapper3D()
    : m_points(vtkPoints::New())
    , m_polygon(vtkPolygon::New())
    , m_polygonPolyData(vtkPolyData::New())
    , m_polygonsCell(vtkCellArray::New())
    , m_VtkPolygonDataMapperGL(vtkOpenGLPolyDataMapper::New())
    , m_PolygonActor(vtkOpenGLActor::New())
    , m_PolygonAssembly(vtkPropAssembly::New())
{

}

//template<class TPixelType>
mitk::PlanarFigureMapper3D::~PlanarFigureMapper3D()
{

}


const mitk::PlanarFigure* mitk::PlanarFigureMapper3D::GetInput()
{
    return static_cast<const mitk::PlanarFigure * > ( GetData() );
}

/*
 This method is called once the mapper gets new input,
 for UI rotation or changes in colorcoding this method is NOT called
 */
void mitk::PlanarFigureMapper3D::GenerateData()
{
    try
    {
        mitk::PlanarFigure* pf = dynamic_cast< mitk::PlanarFigure* > (this->GetData());

        const mitk::Geometry2D* pfgeometry = pf->GetGeometry2D();
        const mitk::PlaneGeometry* planeGeo = dynamic_cast<const mitk::PlaneGeometry*>(pfgeometry);
        mitk::Point3D wp;

        mitk::PlanarFigure::PolyLineType line = pf->GetPolyLine(0);

        if (line.size() <= 2)
            return;

        m_points->SetNumberOfPoints(line.size());
        m_polygon->GetPointIds()->SetNumberOfIds(line.size());

        int i=0;
        for (mitk::PlanarFigure::PolyLineType::iterator it = line.begin(); it!=line.end(); ++it)
        {
            mitk::PlanarFigure::PolyLineElement elem = *it;
            planeGeo->Map(elem.Point, wp);

            m_points->InsertPoint(i,(double)wp[0], (double)wp[1], (double)wp[2] );
            m_polygon->GetPointIds()->SetId(i, i);
            i++;
        }

        m_polygonsCell->Reset();
        m_polygonsCell->InsertNextCell(m_polygon);
        m_polygonPolyData->SetPoints(m_points);
        m_polygonPolyData->SetPolys(m_polygonsCell);

        m_VtkPolygonDataMapperGL->SetInput(m_polygonPolyData);
        m_PolygonActor->SetMapper(m_VtkPolygonDataMapperGL);
        m_PolygonAssembly->AddPart(m_PolygonActor);

        m_VtkPolygonDataMapperGL->Modified();
    }
    catch(...)
    {

    }
}

void mitk::PlanarFigureMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
    try
    {
        mitk::PlanarFigure* pf = dynamic_cast< mitk::PlanarFigure* > (this->GetData());

        const mitk::Geometry2D* pfgeometry = pf->GetGeometry2D();
        const mitk::PlaneGeometry* planeGeo = dynamic_cast<const mitk::PlaneGeometry*>(pfgeometry);
        mitk::Point3D wp;
        mitk::PlanarFigure::PolyLineType line = pf->GetPolyLine(0);

        if (line.size() <= 2)
            return;

        m_points->SetNumberOfPoints(line.size());
        m_polygon->GetPointIds()->SetNumberOfIds(line.size());

        int i=0;
        for (mitk::PlanarFigure::PolyLineType::iterator it = line.begin(); it!=line.end(); ++it)
        {
            mitk::PlanarFigure::PolyLineElement elem = *it;
            planeGeo->Map(elem.Point, wp);

            m_points->InsertPoint(i,(double)wp[0], (double)wp[1], (double)wp[2] );
            m_polygon->GetPointIds()->SetId(i, i);
            i++;
        }

        //updates all polygon pipeline
        m_VtkPolygonDataMapperGL->Modified();

        float polyOpaq;
        this->GetDataNode()->GetOpacity(polyOpaq, NULL);
        m_PolygonActor->GetProperty()->SetOpacity((double) polyOpaq);

        float temprgb[3];
        this->GetDataNode()->GetColor( temprgb, NULL );
        double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
        m_PolygonActor->GetProperty()->SetColor(trgb);
    }
    catch (...)
    {

    }
}

void mitk::PlanarFigureMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
    Superclass::SetDefaultProperties(node, renderer, overwrite);
}

vtkProp* mitk::PlanarFigureMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    return m_PolygonAssembly;

}

void mitk::PlanarFigureMapper3D::UpdateVtkObjects()
{
}

void mitk::PlanarFigureMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{
}
