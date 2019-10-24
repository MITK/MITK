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


#define _USE_MATH_DEFINES
#include "mitkFiberBundle.h"

#include <mitkPlanarCircle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkImageRegionAccessor.h>

#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdFilter.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkDoubleArray.h>
#include <vtkKochanekSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSpline.h>
#include <vtkPolygon.h>
#include <vtkCleanPolyData.h>
#include <cmath>
#include <boost/progress.hpp>
#include <vtkTransformPolyDataFilter.h>
#include <mitkTransferFunction.h>
#include <vtkLookupTable.h>
#include <mitkLookupTable.h>

const char* mitk::FiberBundle::FIBER_ID_ARRAY = "Fiber_IDs";

using namespace std;

mitk::FiberBundle::FiberBundle( vtkPolyData* fiberPolyData )
    : m_NumFibers(0)
    , m_FiberSampling(0)
{
    m_FiberWeights = vtkSmartPointer<vtkFloatArray>::New();
    m_FiberWeights->SetName("FIBER_WEIGHTS");

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    if (fiberPolyData != nullptr)
    {
        m_FiberPolyData = fiberPolyData;
        this->ColorFibersByOrientation();
    }

    this->UpdateFiberGeometry();
    this->GenerateFiberIds();
}

mitk::FiberBundle::~FiberBundle()
{

}

mitk::FiberBundle::Pointer mitk::FiberBundle::GetDeepCopy()
{
    mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(m_FiberPolyData);
    newFib->SetFiberColors(this->m_FiberColors);
    newFib->SetFiberWeights(this->m_FiberWeights);
    return newFib;
}

vtkSmartPointer<vtkPolyData> mitk::FiberBundle::GeneratePolyDataByIds(std::vector<long> fiberIds)
{
    vtkSmartPointer<vtkPolyData> newFiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> newLineSet = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> newPointSet = vtkSmartPointer<vtkPoints>::New();

    auto finIt = fiberIds.begin();
    while ( finIt != fiberIds.end() )
    {
        if (*finIt < 0 || *finIt>GetNumFibers()){
            MITK_INFO << "FiberID can not be negative or >NumFibers!!! check id Extraction!" << *finIt;
            break;
        }

        vtkSmartPointer<vtkCell> fiber = m_FiberIdDataSet->GetCell(*finIt);//->DeepCopy(fiber);
        vtkSmartPointer<vtkPoints> fibPoints = fiber->GetPoints();
        vtkSmartPointer<vtkPolyLine> newFiber = vtkSmartPointer<vtkPolyLine>::New();
        newFiber->GetPointIds()->SetNumberOfIds( fibPoints->GetNumberOfPoints() );

        for(int i=0; i<fibPoints->GetNumberOfPoints(); i++)
        {
            newFiber->GetPointIds()->SetId(i, newPointSet->GetNumberOfPoints());
            newPointSet->InsertNextPoint(fibPoints->GetPoint(i)[0], fibPoints->GetPoint(i)[1], fibPoints->GetPoint(i)[2]);
        }

        newLineSet->InsertNextCell(newFiber);
        ++finIt;
    }

    newFiberPolyData->SetPoints(newPointSet);
    newFiberPolyData->SetLines(newLineSet);
    return newFiberPolyData;
}

// merge two fiber bundles
mitk::FiberBundle::Pointer mitk::FiberBundle::AddBundle(mitk::FiberBundle* fib)
{
    if (fib==nullptr)
    {
        MITK_WARN << "trying to call AddBundle with NULL argument";
        return nullptr;
    }
    MITK_INFO << "Adding fibers";

    vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

    // add current fiber bundle
    vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
    weights->SetNumberOfValues(this->GetNumFibers()+fib->GetNumFibers());

    unsigned int counter = 0;
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double p[3];
            points->GetPoint(j, p);

            vtkIdType id = vNewPoints->InsertNextPoint(p);
            container->GetPointIds()->InsertNextId(id);
        }
        weights->InsertValue(counter, this->GetFiberWeight(i));
        vNewLines->InsertNextCell(container);
        counter++;
    }

    // add new fiber bundle
    for (int i=0; i<fib->GetFiberPolyData()->GetNumberOfCells(); i++)
    {
        vtkCell* cell = fib->GetFiberPolyData()->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double p[3];
            points->GetPoint(j, p);

            vtkIdType id = vNewPoints->InsertNextPoint(p);
            container->GetPointIds()->InsertNextId(id);
        }
        weights->InsertValue(counter, fib->GetFiberWeight(i));
        vNewLines->InsertNextCell(container);
        counter++;
    }

    // initialize polydata
    vNewPolyData->SetPoints(vNewPoints);
    vNewPolyData->SetLines(vNewLines);

    // initialize fiber bundle
    mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(vNewPolyData);
    newFib->SetFiberWeights(weights);
    return newFib;
}

// subtract two fiber bundles
mitk::FiberBundle::Pointer mitk::FiberBundle::SubtractBundle(mitk::FiberBundle* fib)
{
    MITK_INFO << "Subtracting fibers";

    vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

    // iterate over current fibers
    boost::progress_display disp(m_NumFibers);
    for( int i=0; i<m_NumFibers; i++ )
    {
        ++disp;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        if (points==nullptr || numPoints<=0)
            continue;

        int numFibers2 = fib->GetNumFibers();
        bool contained = false;
        for( int i2=0; i2<numFibers2; i2++ )
        {
            vtkCell* cell2 = fib->GetFiberPolyData()->GetCell(i2);
            int numPoints2 = cell2->GetNumberOfPoints();
            vtkPoints* points2 = cell2->GetPoints();

            if (points2==nullptr)// || numPoints2<=0)
                continue;

            // check endpoints
            if (numPoints2==numPoints)
            {
                itk::Point<float, 3> point_start = GetItkPoint(points->GetPoint(0));
                itk::Point<float, 3> point_end = GetItkPoint(points->GetPoint(numPoints-1));
                itk::Point<float, 3> point2_start = GetItkPoint(points2->GetPoint(0));
                itk::Point<float, 3> point2_end = GetItkPoint(points2->GetPoint(numPoints2-1));

                if ((point_start.SquaredEuclideanDistanceTo(point2_start)<=mitk::eps && point_end.SquaredEuclideanDistanceTo(point2_end)<=mitk::eps) ||
                        (point_start.SquaredEuclideanDistanceTo(point2_end)<=mitk::eps && point_end.SquaredEuclideanDistanceTo(point2_start)<=mitk::eps))
                {
                    // further checking ???
                    contained = true;
                    break;
                }
            }
        }

        // add to result because fiber is not subtracted
        if (!contained)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            for( int j=0; j<numPoints; j++)
            {
                vtkIdType id = vNewPoints->InsertNextPoint(points->GetPoint(j));
                container->GetPointIds()->InsertNextId(id);
            }
            vNewLines->InsertNextCell(container);
        }
    }
    if(vNewLines->GetNumberOfCells()==0)
        return nullptr;
    // initialize polydata
    vNewPolyData->SetPoints(vNewPoints);
    vNewPolyData->SetLines(vNewLines);

    // initialize fiber bundle
    return mitk::FiberBundle::New(vNewPolyData);
}

itk::Point<float, 3> mitk::FiberBundle::GetItkPoint(double point[3])
{
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

/*
 * set polydata (additional flag to recompute fiber geometry, default = true)
 */
void mitk::FiberBundle::SetFiberPolyData(vtkSmartPointer<vtkPolyData> fiberPD, bool updateGeometry)
{
    if (fiberPD == nullptr)
        this->m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    else
    {
        m_FiberPolyData->DeepCopy(fiberPD);
        ColorFibersByOrientation();
    }

    m_NumFibers = m_FiberPolyData->GetNumberOfLines();

    if (updateGeometry)
        UpdateFiberGeometry();
    GenerateFiberIds();
}

/*
 * return vtkPolyData
 */
vtkSmartPointer<vtkPolyData> mitk::FiberBundle::GetFiberPolyData() const
{
    return m_FiberPolyData;
}

void mitk::FiberBundle::ColorFibersByOrientation()
{
    //===== FOR WRITING A TEST ========================
    //  colorT size == tupelComponents * tupelElements
    //  compare color results
    //  to cover this code 100% also polydata needed, where colorarray already exists
    //  + one fiber with exactly 1 point
    //  + one fiber with 0 points
    //=================================================

    vtkPoints* extrPoints = nullptr;
    extrPoints = m_FiberPolyData->GetPoints();
    int numOfPoints = 0;
    if (extrPoints!=nullptr)
        numOfPoints = extrPoints->GetNumberOfPoints();

    //colors and alpha value for each single point, RGBA = 4 components
    unsigned char rgba[4] = {0,0,0,0};
    int componentSize = 4;
    m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    m_FiberColors->Allocate(numOfPoints * componentSize);
    m_FiberColors->SetNumberOfComponents(componentSize);
    m_FiberColors->SetName("FIBER_COLORS");

    int numOfFibers = m_FiberPolyData->GetNumberOfLines();
    if (numOfFibers < 1)
        return;

    /* extract single fibers of fiberBundle */
    vtkCellArray* fiberList = m_FiberPolyData->GetLines();
    fiberList->InitTraversal();
    for (int fi=0; fi<numOfFibers; ++fi) {

        vtkIdType* idList; // contains the point id's of the line
        vtkIdType pointsPerFiber; // number of points for current line
        fiberList->GetNextCell(pointsPerFiber, idList);

        /* single fiber checkpoints: is number of points valid */
        if (pointsPerFiber > 1)
        {
            /* operate on points of single fiber */
            for (int i=0; i <pointsPerFiber; ++i)
            {
                /* process all points except starting and endpoint for calculating color value take current point, previous point and next point */
                if (i<pointsPerFiber-1 && i > 0)
                {
                    /* The color value of the current point is influenced by the previous point and next point. */
                    vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
                    vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(idList[i+1])[0], extrPoints->GetPoint(idList[i+1])[1], extrPoints->GetPoint(idList[i+1])[2]);
                    vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(idList[i-1])[0], extrPoints->GetPoint(idList[i-1])[1], extrPoints->GetPoint(idList[i-1])[2]);

                    vnl_vector_fixed< double, 3 > diff1;
                    diff1 = currentPntvtk - nextPntvtk;

                    vnl_vector_fixed< double, 3 > diff2;
                    diff2 = currentPntvtk - prevPntvtk;

                    vnl_vector_fixed< double, 3 > diff;
                    diff = (diff1 - diff2) / 2.0;
                    diff.normalize();

                    rgba[0] = (unsigned char) (255.0 * std::fabs(diff[0]));
                    rgba[1] = (unsigned char) (255.0 * std::fabs(diff[1]));
                    rgba[2] = (unsigned char) (255.0 * std::fabs(diff[2]));
                    rgba[3] = (unsigned char) (255.0);
                }
                else if (i==0)
                {
                    /* First point has no previous point, therefore only diff1 is taken */

                    vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
                    vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(idList[i+1])[0], extrPoints->GetPoint(idList[i+1])[1], extrPoints->GetPoint(idList[i+1])[2]);

                    vnl_vector_fixed< double, 3 > diff1;
                    diff1 = currentPntvtk - nextPntvtk;
                    diff1.normalize();

                    rgba[0] = (unsigned char) (255.0 * std::fabs(diff1[0]));
                    rgba[1] = (unsigned char) (255.0 * std::fabs(diff1[1]));
                    rgba[2] = (unsigned char) (255.0 * std::fabs(diff1[2]));
                    rgba[3] = (unsigned char) (255.0);
                }
                else if (i==pointsPerFiber-1)
                {
                    /* Last point has no next point, therefore only diff2 is taken */
                    vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
                    vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(idList[i-1])[0], extrPoints->GetPoint(idList[i-1])[1], extrPoints->GetPoint(idList[i-1])[2]);

                    vnl_vector_fixed< double, 3 > diff2;
                    diff2 = currentPntvtk - prevPntvtk;
                    diff2.normalize();

                    rgba[0] = (unsigned char) (255.0 * std::fabs(diff2[0]));
                    rgba[1] = (unsigned char) (255.0 * std::fabs(diff2[1]));
                    rgba[2] = (unsigned char) (255.0 * std::fabs(diff2[2]));
                    rgba[3] = (unsigned char) (255.0);
                }
                m_FiberColors->InsertTypedTuple(idList[i], rgba);
            }
        }
        else if (pointsPerFiber == 1)
        {
            /* a single point does not define a fiber (use vertex mechanisms instead */
            continue;
        }
        else
        {
            MITK_DEBUG << "Fiber with 0 points detected... please check your tractography algorithm!" ;
            continue;
        }
    }
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::ColorFibersByCurvature(bool minMaxNorm)
{
    double window = 5;

    //colors and alpha value for each single point, RGBA = 4 components
    unsigned char rgba[4] = {0,0,0,0};
    int componentSize = 4;
    m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    m_FiberColors->Allocate(m_FiberPolyData->GetNumberOfPoints() * componentSize);
    m_FiberColors->SetNumberOfComponents(componentSize);
    m_FiberColors->SetName("FIBER_COLORS");

    mitk::LookupTable::Pointer mitkLookup = mitk::LookupTable::New();
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetTableRange(0.0, 0.8);
    lookupTable->Build();
    mitkLookup->SetVtkLookupTable(lookupTable);
    mitkLookup->SetType(mitk::LookupTable::JET);

    vector< double > values;
    double min = 1;
    double max = 0;
    MITK_INFO << "Coloring fibers by curvature";
    boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        ++disp;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        // calculate curvatures
        for (int j=0; j<numPoints; j++)
        {
            double dist = 0;
            int c = j;
            std::vector< vnl_vector_fixed< float, 3 > > vectors;
            vnl_vector_fixed< float, 3 > meanV; meanV.fill(0.0);
            while(dist<window/2 && c>1)
            {
                double p1[3];
                points->GetPoint(c-1, p1);
                double p2[3];
                points->GetPoint(c, p2);

                vnl_vector_fixed< float, 3 > v;
                v[0] = p2[0]-p1[0];
                v[1] = p2[1]-p1[1];
                v[2] = p2[2]-p1[2];
                dist += v.magnitude();
                v.normalize();
                vectors.push_back(v);
                if (c==j)
                    meanV += v;
                c--;
            }
            c = j;
            dist = 0;
            while(dist<window/2 && c<numPoints-1)
            {
                double p1[3];
                points->GetPoint(c, p1);
                double p2[3];
                points->GetPoint(c+1, p2);

                vnl_vector_fixed< float, 3 > v;
                v[0] = p2[0]-p1[0];
                v[1] = p2[1]-p1[1];
                v[2] = p2[2]-p1[2];
                dist += v.magnitude();
                v.normalize();
                vectors.push_back(v);
                if (c==j)
                    meanV += v;
                c++;
            }
            meanV.normalize();

            double dev = 0;
            for (unsigned int c=0; c<vectors.size(); c++)
            {
                double angle = dot_product(meanV, vectors.at(c));
                if (angle>1.0)
                    angle = 1.0;
                if (angle<-1.0)
                    angle = -1.0;
                dev += acos(angle)*180/M_PI;
            }
            if (vectors.size()>0)
                dev /= vectors.size();

            dev = 1.0-dev/180.0;
            values.push_back(dev);
            if (dev<min)
                min = dev;
            if (dev>max)
                max = dev;
        }
    }
    unsigned int count = 0;
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        for (int j=0; j<numPoints; j++)
        {
            double color[3];
            double dev = values.at(count);
            if (minMaxNorm)
                dev = (dev-min)/(max-min);
            //            double dev = values.at(count)*values.at(count);
            lookupTable->GetColor(dev, color);

            rgba[0] = (unsigned char) (255.0 * color[0]);
            rgba[1] = (unsigned char) (255.0 * color[1]);
            rgba[2] = (unsigned char) (255.0 * color[2]);
            rgba[3] = (unsigned char) (255.0);
            m_FiberColors->InsertTypedTuple(cell->GetPointId(j), rgba);
            count++;
        }
    }
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::SetFiberOpacity(vtkDoubleArray* FAValArray)
{
    for(long i=0; i<m_FiberColors->GetNumberOfTuples(); i++)
    {
        double faValue = FAValArray->GetValue(i);
        faValue = faValue * 255.0;
        m_FiberColors->SetComponent(i,3, (unsigned char) faValue );
    }
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::ResetFiberOpacity()
{
    for(long i=0; i<m_FiberColors->GetNumberOfTuples(); i++)
        m_FiberColors->SetComponent(i,3, 255.0 );
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::ColorFibersByScalarMap(mitk::Image::Pointer FAimage, bool opacity)
{
    mitkPixelTypeMultiplex2( ColorFibersByScalarMap, FAimage->GetPixelType(), FAimage, opacity );
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

template <typename TPixel>
void mitk::FiberBundle::ColorFibersByScalarMap(const mitk::PixelType, mitk::Image::Pointer image, bool opacity)
{
    m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    m_FiberColors->Allocate(m_FiberPolyData->GetNumberOfPoints() * 4);
    m_FiberColors->SetNumberOfComponents(4);
    m_FiberColors->SetName("FIBER_COLORS");

    mitk::ImageRegionAccessor readimage(image);

    unsigned char rgba[4] = {0,0,0,0};
    vtkPoints* pointSet = m_FiberPolyData->GetPoints();

    mitk::LookupTable::Pointer mitkLookup = mitk::LookupTable::New();
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetTableRange(0.0, 0.8);
    lookupTable->Build();
    mitkLookup->SetVtkLookupTable(lookupTable);
    mitkLookup->SetType(mitk::LookupTable::JET);

    for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
    {
        Point3D px;
        px[0] = pointSet->GetPoint(i)[0];
        px[1] = pointSet->GetPoint(i)[1];
        px[2] = pointSet->GetPoint(i)[2];
        itk::Index<3> idx;
        image->GetGeometry()->WorldToIndex(px, idx);
        double pixelValue = *(TPixel*)readimage.getPixel(idx);

        double color[3];
        lookupTable->GetColor(1-pixelValue, color);

        rgba[0] = (unsigned char) (255.0 * color[0]);
        rgba[1] = (unsigned char) (255.0 * color[1]);
        rgba[2] = (unsigned char) (255.0 * color[2]);
        if (opacity)
            rgba[3] = (unsigned char) (255.0 * pixelValue);
        else
            rgba[3] = (unsigned char) (255.0);
        m_FiberColors->InsertTypedTuple(i, rgba);
    }
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::SetFiberColors(float r, float g, float b, float alpha)
{
    m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    m_FiberColors->Allocate(m_FiberPolyData->GetNumberOfPoints() * 4);
    m_FiberColors->SetNumberOfComponents(4);
    m_FiberColors->SetName("FIBER_COLORS");

    unsigned char rgba[4] = {0,0,0,0};
    for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
    {
        rgba[0] = (unsigned char) r;
        rgba[1] = (unsigned char) g;
        rgba[2] = (unsigned char) b;
        rgba[3] = (unsigned char) alpha;
        m_FiberColors->InsertTypedTuple(i, rgba);
    }
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::GenerateFiberIds()
{
    if (m_FiberPolyData == nullptr)
        return;

    vtkSmartPointer<vtkIdFilter> idFiberFilter = vtkSmartPointer<vtkIdFilter>::New();
    idFiberFilter->SetInputData(m_FiberPolyData);
    idFiberFilter->CellIdsOn();
    //  idFiberFilter->PointIdsOn(); // point id's are not needed
    idFiberFilter->SetIdsArrayName(FIBER_ID_ARRAY);
    idFiberFilter->FieldDataOn();
    idFiberFilter->Update();

    m_FiberIdDataSet = idFiberFilter->GetOutput();

}

mitk::FiberBundle::Pointer mitk::FiberBundle::ExtractFiberSubset(ItkUcharImgType* mask, bool anyPoint, bool invert, bool bothEnds)
{
    vtkSmartPointer<vtkPolyData> polyData = m_FiberPolyData;
    if (anyPoint)
    {
        float minSpacing = 1;
        if(mask->GetSpacing()[0]<mask->GetSpacing()[1] && mask->GetSpacing()[0]<mask->GetSpacing()[2])
            minSpacing = mask->GetSpacing()[0];
        else if (mask->GetSpacing()[1] < mask->GetSpacing()[2])
            minSpacing = mask->GetSpacing()[1];
        else
            minSpacing = mask->GetSpacing()[2];

        mitk::FiberBundle::Pointer fibCopy = this->GetDeepCopy();
        fibCopy->ResampleSpline(minSpacing/5);
        polyData = fibCopy->GetFiberPolyData();
    }
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    MITK_INFO << "Extracting fibers";
    boost::progress_display disp(m_NumFibers);
    for (int i=0; i<m_NumFibers; i++)
    {
        ++disp;

        vtkCell* cell = polyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkCell* cellOriginal = m_FiberPolyData->GetCell(i);
        int numPointsOriginal = cellOriginal->GetNumberOfPoints();
        vtkPoints* pointsOriginal = cellOriginal->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

        if (numPoints>1 && numPointsOriginal)
        {
            if (anyPoint)
            {
                if (!invert)
                {
                    for (int j=0; j<numPoints; j++)
                    {
                        double* p = points->GetPoint(j);

                        itk::Point<float, 3> itkP;
                        itkP[0] = p[0]; itkP[1] = p[1]; itkP[2] = p[2];
                        itk::Index<3> idx;
                        mask->TransformPhysicalPointToIndex(itkP, idx);

                        if ( mask->GetPixel(idx)>0 && mask->GetLargestPossibleRegion().IsInside(idx) )
                        {
                            for (int k=0; k<numPointsOriginal; k++)
                            {
                                double* p = pointsOriginal->GetPoint(k);
                                vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                                container->GetPointIds()->InsertNextId(id);
                            }
                            break;
                        }
                    }
                }
                else
                {
                    bool includeFiber = true;
                    for (int j=0; j<numPoints; j++)
                    {
                        double* p = points->GetPoint(j);

                        itk::Point<float, 3> itkP;
                        itkP[0] = p[0]; itkP[1] = p[1]; itkP[2] = p[2];
                        itk::Index<3> idx;
                        mask->TransformPhysicalPointToIndex(itkP, idx);

                        if ( mask->GetPixel(idx)>0 && mask->GetLargestPossibleRegion().IsInside(idx) )
                        {
                            includeFiber = false;
                            break;
                        }
                    }
                    if (includeFiber)
                    {

                        for (int k=0; k<numPointsOriginal; k++)
                        {
                            double* p = pointsOriginal->GetPoint(k);
                            vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                            container->GetPointIds()->InsertNextId(id);
                        }
                    }
                }
            }
            else
            {
                double* start = pointsOriginal->GetPoint(0);
                itk::Point<float, 3> itkStart;
                itkStart[0] = start[0]; itkStart[1] = start[1]; itkStart[2] = start[2];
                itk::Index<3> idxStart;
                mask->TransformPhysicalPointToIndex(itkStart, idxStart);

                double* end = pointsOriginal->GetPoint(numPointsOriginal-1);
                itk::Point<float, 3> itkEnd;
                itkEnd[0] = end[0]; itkEnd[1] = end[1]; itkEnd[2] = end[2];
                itk::Index<3> idxEnd;
                mask->TransformPhysicalPointToIndex(itkEnd, idxEnd);

                if (invert)
                {
                    if (bothEnds)
                    {
                        if ( !mask->GetPixel(idxStart)>0 && !mask->GetPixel(idxEnd)>0 )
                        {
                            for (int j=0; j<numPointsOriginal; j++)
                            {
                                double* p = pointsOriginal->GetPoint(j);
                                vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                                container->GetPointIds()->InsertNextId(id);
                            }
                        }
                    }
                    else if ( !mask->GetPixel(idxStart)>0 || !mask->GetPixel(idxEnd)>0 )
                    {
                        for (int j=0; j<numPointsOriginal; j++)
                        {
                            double* p = pointsOriginal->GetPoint(j);
                            vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                            container->GetPointIds()->InsertNextId(id);
                        }
                    }
                }
                else
                {
                    if (bothEnds)
                    {
                        if ( mask->GetPixel(idxStart)>0 && mask->GetPixel(idxEnd)>0 && mask->GetLargestPossibleRegion().IsInside(idxStart) && mask->GetLargestPossibleRegion().IsInside(idxEnd) )
                        {
                            for (int j=0; j<numPointsOriginal; j++)
                            {
                                double* p = pointsOriginal->GetPoint(j);
                                vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                                container->GetPointIds()->InsertNextId(id);
                            }
                        }
                    }
                    else if ( (mask->GetPixel(idxStart)>0 && mask->GetLargestPossibleRegion().IsInside(idxStart)) || (mask->GetPixel(idxEnd)>0 && mask->GetLargestPossibleRegion().IsInside(idxEnd)) )
                    {
                        for (int j=0; j<numPointsOriginal; j++)
                        {
                            double* p = pointsOriginal->GetPoint(j);
                            vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                            container->GetPointIds()->InsertNextId(id);
                        }
                    }
                }
            }
        }

        vtkNewCells->InsertNextCell(container);
    }

    if (vtkNewCells->GetNumberOfCells()<=0)
        return nullptr;

    vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
    newPolyData->SetPoints(vtkNewPoints);
    newPolyData->SetLines(vtkNewCells);
    return mitk::FiberBundle::New(newPolyData);
}

mitk::FiberBundle::Pointer mitk::FiberBundle::RemoveFibersOutside(ItkUcharImgType* mask, bool invert)
{
    float minSpacing = 1;
    if(mask->GetSpacing()[0]<mask->GetSpacing()[1] && mask->GetSpacing()[0]<mask->GetSpacing()[2])
        minSpacing = mask->GetSpacing()[0];
    else if (mask->GetSpacing()[1] < mask->GetSpacing()[2])
        minSpacing = mask->GetSpacing()[1];
    else
        minSpacing = mask->GetSpacing()[2];

    mitk::FiberBundle::Pointer fibCopy = this->GetDeepCopy();
    fibCopy->ResampleSpline(minSpacing/10);
    vtkSmartPointer<vtkPolyData> polyData =fibCopy->GetFiberPolyData();

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    MITK_INFO << "Cutting fibers";
    boost::progress_display disp(m_NumFibers);
    for (int i=0; i<m_NumFibers; i++)
    {
        ++disp;

        vtkCell* cell = polyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        if (numPoints>1)
        {
            int newNumPoints = 0;
            for (int j=0; j<numPoints; j++)
            {
                double* p = points->GetPoint(j);

                itk::Point<float, 3> itkP;
                itkP[0] = p[0]; itkP[1] = p[1]; itkP[2] = p[2];
                itk::Index<3> idx;
                mask->TransformPhysicalPointToIndex(itkP, idx);

                if ( mask->GetPixel(idx)>0 && mask->GetLargestPossibleRegion().IsInside(idx) && !invert )
                {
                    vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                    container->GetPointIds()->InsertNextId(id);
                    newNumPoints++;
                }
                else if ( (mask->GetPixel(idx)<=0 || !mask->GetLargestPossibleRegion().IsInside(idx)) && invert )
                {
                    vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                    container->GetPointIds()->InsertNextId(id);
                    newNumPoints++;
                }
                else if (newNumPoints>0)
                {
                    vtkNewCells->InsertNextCell(container);

                    newNumPoints = 0;
                    container = vtkSmartPointer<vtkPolyLine>::New();
                }
            }

            if (newNumPoints>0)
                vtkNewCells->InsertNextCell(container);
        }

    }

    if (vtkNewCells->GetNumberOfCells()<=0)
        return nullptr;

    vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
    newPolyData->SetPoints(vtkNewPoints);
    newPolyData->SetLines(vtkNewCells);
    mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(newPolyData);
    newFib->Compress(0.1);
    return newFib;
}

mitk::FiberBundle::Pointer mitk::FiberBundle::ExtractFiberSubset(DataNode* roi, DataStorage* storage)
{
    if (roi==nullptr || !(dynamic_cast<PlanarFigure*>(roi->GetData()) || dynamic_cast<PlanarFigureComposite*>(roi->GetData())) )
        return nullptr;

    std::vector<long> tmp = ExtractFiberIdSubset(roi, storage);

    if (tmp.size()<=0)
        return mitk::FiberBundle::New();
    vtkSmartPointer<vtkPolyData> pTmp = GeneratePolyDataByIds(tmp);
    return mitk::FiberBundle::New(pTmp);
}

std::vector<long> mitk::FiberBundle::ExtractFiberIdSubset(DataNode *roi, DataStorage* storage)
{
    std::vector<long> result;
    if (roi==nullptr || roi->GetData()==nullptr)
        return result;

    mitk::PlanarFigureComposite::Pointer pfc = dynamic_cast<mitk::PlanarFigureComposite*>(roi->GetData());
    if (!pfc.IsNull()) // handle composite
    {
        DataStorage::SetOfObjects::ConstPointer children = storage->GetDerivations(roi);
        if (children->size()==0)
            return result;

        switch (pfc->getOperationType())
        {
        case 0: // AND
        {
            MITK_INFO << "AND";
            result = this->ExtractFiberIdSubset(children->ElementAt(0), storage);
            std::vector<long>::iterator it;
            for (unsigned int i=1; i<children->Size(); ++i)
            {
                std::vector<long> inRoi = this->ExtractFiberIdSubset(children->ElementAt(i), storage);

                std::vector<long> rest(std::min(result.size(),inRoi.size()));
                it = std::set_intersection(result.begin(), result.end(), inRoi.begin(), inRoi.end(), rest.begin() );
                rest.resize( it - rest.begin() );
                result = rest;
            }
            break;
        }
        case 1: // OR
        {
            MITK_INFO << "OR";
            result = ExtractFiberIdSubset(children->ElementAt(0), storage);
            std::vector<long>::iterator it;
            for (unsigned int i=1; i<children->Size(); ++i)
            {
                it = result.end();
                std::vector<long> inRoi = ExtractFiberIdSubset(children->ElementAt(i), storage);
                result.insert(it, inRoi.begin(), inRoi.end());
            }

            // remove duplicates
            sort(result.begin(), result.end());
            it = unique(result.begin(), result.end());
            result.resize( it - result.begin() );
            break;
        }
        case 2: // NOT
        {
            MITK_INFO << "NOT";
            for(long i=0; i<this->GetNumFibers(); i++)
                result.push_back(i);

            std::vector<long>::iterator it;
            for (long i=0; i<children->Size(); ++i)
            {
                std::vector<long> inRoi = ExtractFiberIdSubset(children->ElementAt(i), storage);

                std::vector<long> rest(result.size()-inRoi.size());
                it = std::set_difference(result.begin(), result.end(), inRoi.begin(), inRoi.end(), rest.begin() );
                rest.resize( it - rest.begin() );
                result = rest;
            }
            break;
        }
        }
    }
    else if ( dynamic_cast<mitk::PlanarFigure*>(roi->GetData()) )  // actual extraction
    {
        if ( dynamic_cast<mitk::PlanarPolygon*>(roi->GetData()) )
        {
            mitk::PlanarFigure::Pointer planarPoly = dynamic_cast<mitk::PlanarFigure*>(roi->GetData());

            //create vtkPolygon using controlpoints from planarFigure polygon
            vtkSmartPointer<vtkPolygon> polygonVtk = vtkSmartPointer<vtkPolygon>::New();
            for (unsigned int i=0; i<planarPoly->GetNumberOfControlPoints(); ++i)
            {
                itk::Point<double,3> p = planarPoly->GetWorldControlPoint(i);
                vtkIdType id = polygonVtk->GetPoints()->InsertNextPoint(p[0], p[1], p[2] );
                polygonVtk->GetPointIds()->InsertNextId(id);
            }

            MITK_INFO << "Extracting with polygon";
            boost::progress_display disp(m_NumFibers);
            for (int i=0; i<m_NumFibers; i++)
            {
                ++disp ;
                vtkCell* cell = m_FiberPolyData->GetCell(i);
                int numPoints = cell->GetNumberOfPoints();
                vtkPoints* points = cell->GetPoints();

                for (int j=0; j<numPoints-1; j++)
                {
                    // Inputs
                    double p1[3] = {0,0,0};
                    points->GetPoint(j, p1);
                    double p2[3] = {0,0,0};
                    points->GetPoint(j+1, p2);
                    double tolerance = 0.001;

                    // Outputs
                    double t = 0; // Parametric coordinate of intersection (0 (corresponding to p1) to 1 (corresponding to p2))
                    double x[3] = {0,0,0}; // The coordinate of the intersection
                    double pcoords[3] = {0,0,0};
                    int subId = 0;

                    int iD = polygonVtk->IntersectWithLine(p1, p2, tolerance, t, x, pcoords, subId);
                    if (iD!=0)
                    {
                        result.push_back(i);
                        break;
                    }
                }
            }
        }
        else if ( dynamic_cast<mitk::PlanarCircle*>(roi->GetData()) )
        {
            mitk::PlanarFigure::Pointer planarFigure = dynamic_cast<mitk::PlanarFigure*>(roi->GetData());
            Vector3D planeNormal = planarFigure->GetPlaneGeometry()->GetNormal();
            planeNormal.Normalize();

            //calculate circle radius
            mitk::Point3D V1w = planarFigure->GetWorldControlPoint(0); //centerPoint
            mitk::Point3D V2w  = planarFigure->GetWorldControlPoint(1); //radiusPoint

            double radius = V1w.EuclideanDistanceTo(V2w);
            radius *= radius;

            MITK_INFO << "Extracting with circle";
            boost::progress_display disp(m_NumFibers);
            for (int i=0; i<m_NumFibers; i++)
            {
                ++disp ;
                vtkCell* cell = m_FiberPolyData->GetCell(i);
                int numPoints = cell->GetNumberOfPoints();
                vtkPoints* points = cell->GetPoints();

                for (int j=0; j<numPoints-1; j++)
                {
                    // Inputs
                    double p1[3] = {0,0,0};
                    points->GetPoint(j, p1);
                    double p2[3] = {0,0,0};
                    points->GetPoint(j+1, p2);

                    // Outputs
                    double t = 0; // Parametric coordinate of intersection (0 (corresponding to p1) to 1 (corresponding to p2))
                    double x[3] = {0,0,0}; // The coordinate of the intersection

                    int iD = vtkPlane::IntersectWithLine(p1,p2,planeNormal.GetDataPointer(),V1w.GetDataPointer(),t,x);

                    if (iD!=0)
                    {
                        double dist = (x[0]-V1w[0])*(x[0]-V1w[0])+(x[1]-V1w[1])*(x[1]-V1w[1])+(x[2]-V1w[2])*(x[2]-V1w[2]);
                        if( dist <= radius)
                        {
                            result.push_back(i);
                            break;
                        }
                    }
                }
            }
        }
        return result;
    }

    return result;
}

void mitk::FiberBundle::UpdateFiberGeometry()
{
    vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
    cleaner->SetInputData(m_FiberPolyData);
    cleaner->PointMergingOff();
    cleaner->Update();
    m_FiberPolyData = cleaner->GetOutput();

    m_FiberLengths.clear();
    m_MeanFiberLength = 0;
    m_MedianFiberLength = 0;
    m_LengthStDev = 0;
    m_NumFibers = m_FiberPolyData->GetNumberOfCells();

    if (m_FiberColors==nullptr || m_FiberColors->GetNumberOfTuples()!=m_FiberPolyData->GetNumberOfPoints())
        this->ColorFibersByOrientation();

    if (m_FiberWeights->GetSize()!=m_NumFibers)
    {
        m_FiberWeights = vtkSmartPointer<vtkFloatArray>::New();
        m_FiberWeights->SetName("FIBER_WEIGHTS");
        m_FiberWeights->SetNumberOfValues(m_NumFibers);
        this->SetFiberWeights(1);
    }

    if (m_NumFibers<=0) // no fibers present; apply default geometry
    {
        m_MinFiberLength = 0;
        m_MaxFiberLength = 0;
        mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
        geometry->SetImageGeometry(false);
        float b[] = {0, 1, 0, 1, 0, 1};
        geometry->SetFloatBounds(b);
        SetGeometry(geometry);
        return;
    }
    double b[6];
    m_FiberPolyData->GetBounds(b);

    // calculate statistics
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int p = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();
        float length = 0;
        for (int j=0; j<p-1; j++)
        {
            double p1[3];
            points->GetPoint(j, p1);
            double p2[3];
            points->GetPoint(j+1, p2);

            float dist = std::sqrt((p1[0]-p2[0])*(p1[0]-p2[0])+(p1[1]-p2[1])*(p1[1]-p2[1])+(p1[2]-p2[2])*(p1[2]-p2[2]));
            length += dist;
        }
        m_FiberLengths.push_back(length);
        m_MeanFiberLength += length;
        if (i==0)
        {
            m_MinFiberLength = length;
            m_MaxFiberLength = length;
        }
        else
        {
            if (length<m_MinFiberLength)
                m_MinFiberLength = length;
            if (length>m_MaxFiberLength)
                m_MaxFiberLength = length;
        }
    }
    m_MeanFiberLength /= m_NumFibers;

    std::vector< float > sortedLengths = m_FiberLengths;
    std::sort(sortedLengths.begin(), sortedLengths.end());
    for (int i=0; i<m_NumFibers; i++)
        m_LengthStDev += (m_MeanFiberLength-sortedLengths.at(i))*(m_MeanFiberLength-sortedLengths.at(i));
    if (m_NumFibers>1)
        m_LengthStDev /= (m_NumFibers-1);
    else
        m_LengthStDev = 0;
    m_LengthStDev = std::sqrt(m_LengthStDev);
    m_MedianFiberLength = sortedLengths.at(m_NumFibers/2);

    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    geometry->SetFloatBounds(b);
    this->SetGeometry(geometry);

    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

float mitk::FiberBundle::GetFiberWeight(unsigned int fiber)
{
    return m_FiberWeights->GetValue(fiber);
}

void mitk::FiberBundle::SetFiberWeights(float newWeight)
{
    for (int i=0; i<m_FiberWeights->GetSize(); i++)
        m_FiberWeights->SetValue(i, newWeight);
}

void mitk::FiberBundle::SetFiberWeights(vtkSmartPointer<vtkFloatArray> weights)
{
    if (m_NumFibers!=weights->GetSize())
    {
        MITK_INFO << "Weights array not equal to number of fibers!";
        return;
    }

    for (int i=0; i<weights->GetSize(); i++)
        m_FiberWeights->SetValue(i, weights->GetValue(i));

    m_FiberWeights->SetName("FIBER_WEIGHTS");
}

void mitk::FiberBundle::SetFiberWeight(unsigned int fiber, float weight)
{
    m_FiberWeights->SetValue(fiber, weight);
}

void mitk::FiberBundle::SetFiberColors(vtkSmartPointer<vtkUnsignedCharArray> fiberColors)
{
    for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
    {
        unsigned char source[4] = {0,0,0,0};
        fiberColors->GetTypedTuple(i, source);

        unsigned char target[4] = {0,0,0,0};
        target[0] = source[0];
        target[1] = source[1];
        target[2] = source[2];
        target[3] = source[3];
        m_FiberColors->InsertTypedTuple(i, target);
    }
    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

itk::Matrix< double, 3, 3 > mitk::FiberBundle::TransformMatrix(itk::Matrix< double, 3, 3 > m, double rx, double ry, double rz)
{
    rx = rx*M_PI/180;
    ry = ry*M_PI/180;
    rz = rz*M_PI/180;

    itk::Matrix< double, 3, 3 > rotX; rotX.SetIdentity();
    rotX[1][1] = cos(rx);
    rotX[2][2] = rotX[1][1];
    rotX[1][2] = -sin(rx);
    rotX[2][1] = -rotX[1][2];

    itk::Matrix< double, 3, 3 > rotY; rotY.SetIdentity();
    rotY[0][0] = cos(ry);
    rotY[2][2] = rotY[0][0];
    rotY[0][2] = sin(ry);
    rotY[2][0] = -rotY[0][2];

    itk::Matrix< double, 3, 3 > rotZ; rotZ.SetIdentity();
    rotZ[0][0] = cos(rz);
    rotZ[1][1] = rotZ[0][0];
    rotZ[0][1] = -sin(rz);
    rotZ[1][0] = -rotZ[0][1];

    itk::Matrix< double, 3, 3 > rot = rotZ*rotY*rotX;

    m = rot*m;

    return m;
}

itk::Point<float, 3> mitk::FiberBundle::TransformPoint(vnl_vector_fixed< double, 3 > point, double rx, double ry, double rz, double tx, double ty, double tz)
{
    rx = rx*M_PI/180;
    ry = ry*M_PI/180;
    rz = rz*M_PI/180;

    vnl_matrix_fixed< double, 3, 3 > rotX; rotX.set_identity();
    rotX[1][1] = cos(rx);
    rotX[2][2] = rotX[1][1];
    rotX[1][2] = -sin(rx);
    rotX[2][1] = -rotX[1][2];

    vnl_matrix_fixed< double, 3, 3 > rotY; rotY.set_identity();
    rotY[0][0] = cos(ry);
    rotY[2][2] = rotY[0][0];
    rotY[0][2] = sin(ry);
    rotY[2][0] = -rotY[0][2];

    vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
    rotZ[0][0] = cos(rz);
    rotZ[1][1] = rotZ[0][0];
    rotZ[0][1] = -sin(rz);
    rotZ[1][0] = -rotZ[0][1];

    vnl_matrix_fixed< double, 3, 3 > rot = rotZ*rotY*rotX;

    mitk::BaseGeometry::Pointer geom = this->GetGeometry();
    mitk::Point3D center = geom->GetCenter();

    point[0] -= center[0];
    point[1] -= center[1];
    point[2] -= center[2];
    point = rot*point;
    point[0] += center[0]+tx;
    point[1] += center[1]+ty;
    point[2] += center[2]+tz;
    itk::Point<float, 3> out; out[0] = point[0]; out[1] = point[1]; out[2] = point[2];
    return out;
}

void mitk::FiberBundle::TransformFibers(double rx, double ry, double rz, double tx, double ty, double tz)
{
    rx = rx*M_PI/180;
    ry = ry*M_PI/180;
    rz = rz*M_PI/180;

    vnl_matrix_fixed< double, 3, 3 > rotX; rotX.set_identity();
    rotX[1][1] = cos(rx);
    rotX[2][2] = rotX[1][1];
    rotX[1][2] = -sin(rx);
    rotX[2][1] = -rotX[1][2];

    vnl_matrix_fixed< double, 3, 3 > rotY; rotY.set_identity();
    rotY[0][0] = cos(ry);
    rotY[2][2] = rotY[0][0];
    rotY[0][2] = sin(ry);
    rotY[2][0] = -rotY[0][2];

    vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
    rotZ[0][0] = cos(rz);
    rotZ[1][1] = rotZ[0][0];
    rotZ[0][1] = -sin(rz);
    rotZ[1][0] = -rotZ[0][1];

    vnl_matrix_fixed< double, 3, 3 > rot = rotZ*rotY*rotX;

    mitk::BaseGeometry::Pointer geom = this->GetGeometry();
    mitk::Point3D center = geom->GetCenter();

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    for (int i=0; i<m_NumFibers; i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double* p = points->GetPoint(j);
            vnl_vector_fixed< double, 3 > dir;
            dir[0] = p[0]-center[0];
            dir[1] = p[1]-center[1];
            dir[2] = p[2]-center[2];
            dir = rot*dir;
            dir[0] += center[0]+tx;
            dir[1] += center[1]+ty;
            dir[2] += center[2]+tz;
            vtkIdType id = vtkNewPoints->InsertNextPoint(dir.data_block());
            container->GetPointIds()->InsertNextId(id);
        }
        vtkNewCells->InsertNextCell(container);
    }

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::RotateAroundAxis(double x, double y, double z)
{
    x = x*M_PI/180;
    y = y*M_PI/180;
    z = z*M_PI/180;

    vnl_matrix_fixed< double, 3, 3 > rotX; rotX.set_identity();
    rotX[1][1] = cos(x);
    rotX[2][2] = rotX[1][1];
    rotX[1][2] = -sin(x);
    rotX[2][1] = -rotX[1][2];

    vnl_matrix_fixed< double, 3, 3 > rotY; rotY.set_identity();
    rotY[0][0] = cos(y);
    rotY[2][2] = rotY[0][0];
    rotY[0][2] = sin(y);
    rotY[2][0] = -rotY[0][2];

    vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
    rotZ[0][0] = cos(z);
    rotZ[1][1] = rotZ[0][0];
    rotZ[0][1] = -sin(z);
    rotZ[1][0] = -rotZ[0][1];

    mitk::BaseGeometry::Pointer geom = this->GetGeometry();
    mitk::Point3D center = geom->GetCenter();

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    for (int i=0; i<m_NumFibers; i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double* p = points->GetPoint(j);
            vnl_vector_fixed< double, 3 > dir;
            dir[0] = p[0]-center[0];
            dir[1] = p[1]-center[1];
            dir[2] = p[2]-center[2];
            dir = rotZ*rotY*rotX*dir;
            dir[0] += center[0];
            dir[1] += center[1];
            dir[2] += center[2];
            vtkIdType id = vtkNewPoints->InsertNextPoint(dir.data_block());
            container->GetPointIds()->InsertNextId(id);
        }
        vtkNewCells->InsertNextCell(container);
    }

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::ScaleFibers(double x, double y, double z, bool subtractCenter)
{
    MITK_INFO << "Scaling fibers";
    boost::progress_display disp(m_NumFibers);

    mitk::BaseGeometry* geom = this->GetGeometry();
    mitk::Point3D c = geom->GetCenter();

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    for (int i=0; i<m_NumFibers; i++)
    {
        ++disp ;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double* p = points->GetPoint(j);
            if (subtractCenter)
            {
                p[0] -= c[0]; p[1] -= c[1]; p[2] -= c[2];
            }
            p[0] *= x;
            p[1] *= y;
            p[2] *= z;
            if (subtractCenter)
            {
                p[0] += c[0]; p[1] += c[1]; p[2] += c[2];
            }
            vtkIdType id = vtkNewPoints->InsertNextPoint(p);
            container->GetPointIds()->InsertNextId(id);
        }
        vtkNewCells->InsertNextCell(container);
    }

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::TranslateFibers(double x, double y, double z)
{
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    for (int i=0; i<m_NumFibers; i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double* p = points->GetPoint(j);
            p[0] += x;
            p[1] += y;
            p[2] += z;
            vtkIdType id = vtkNewPoints->InsertNextPoint(p);
            container->GetPointIds()->InsertNextId(id);
        }
        vtkNewCells->InsertNextCell(container);
    }

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::MirrorFibers(unsigned int axis)
{
    if (axis>2)
        return;

    MITK_INFO << "Mirroring fibers";
    boost::progress_display disp(m_NumFibers);

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    for (int i=0; i<m_NumFibers; i++)
    {
        ++disp;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints; j++)
        {
            double* p = points->GetPoint(j);
            p[axis] = -p[axis];
            vtkIdType id = vtkNewPoints->InsertNextPoint(p);
            container->GetPointIds()->InsertNextId(id);
        }
        vtkNewCells->InsertNextCell(container);
    }

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::RemoveDir(vnl_vector_fixed<double,3> dir, double threshold)
{
    dir.normalize();
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        ++disp ;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        // calculate curvatures
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        bool discard = false;
        for (int j=0; j<numPoints-1; j++)
        {
            double p1[3];
            points->GetPoint(j, p1);
            double p2[3];
            points->GetPoint(j+1, p2);

            vnl_vector_fixed< double, 3 > v1;
            v1[0] = p2[0]-p1[0];
            v1[1] = p2[1]-p1[1];
            v1[2] = p2[2]-p1[2];
            if (v1.magnitude()>0.001)
            {
                v1.normalize();

                if (fabs(dot_product(v1,dir))>threshold)
                {
                    discard = true;
                    break;
                }
            }
        }
        if (!discard)
        {
            for (int j=0; j<numPoints; j++)
            {
                double p1[3];
                points->GetPoint(j, p1);

                vtkIdType id = vtkNewPoints->InsertNextPoint(p1);
                container->GetPointIds()->InsertNextId(id);
            }
            vtkNewCells->InsertNextCell(container);
        }
    }

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);

    this->SetFiberPolyData(m_FiberPolyData, true);

    //    UpdateColorCoding();
    //    UpdateFiberGeometry();
}

bool mitk::FiberBundle::ApplyCurvatureThreshold(float minRadius, bool deleteFibers)
{
    if (minRadius<0)
        return true;

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    MITK_INFO << "Applying curvature threshold";
    boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        ++disp ;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        // calculate curvatures
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for (int j=0; j<numPoints-2; j++)
        {
            double p1[3];
            points->GetPoint(j, p1);
            double p2[3];
            points->GetPoint(j+1, p2);
            double p3[3];
            points->GetPoint(j+2, p3);

            vnl_vector_fixed< float, 3 > v1, v2, v3;

            v1[0] = p2[0]-p1[0];
            v1[1] = p2[1]-p1[1];
            v1[2] = p2[2]-p1[2];

            v2[0] = p3[0]-p2[0];
            v2[1] = p3[1]-p2[1];
            v2[2] = p3[2]-p2[2];

            v3[0] = p1[0]-p3[0];
            v3[1] = p1[1]-p3[1];
            v3[2] = p1[2]-p3[2];

            float a = v1.magnitude();
            float b = v2.magnitude();
            float c = v3.magnitude();
            float r = a*b*c/std::sqrt((a+b+c)*(a+b-c)*(b+c-a)*(a-b+c)); // radius of triangle via Heron's formula (area of triangle)

            vtkIdType id = vtkNewPoints->InsertNextPoint(p1);
            container->GetPointIds()->InsertNextId(id);

            if (deleteFibers && r<minRadius)
                break;

            if (r<minRadius)
            {
                j += 2;
                vtkNewCells->InsertNextCell(container);
                container = vtkSmartPointer<vtkPolyLine>::New();
            }
            else if (j==numPoints-3)
            {
                id = vtkNewPoints->InsertNextPoint(p2);
                container->GetPointIds()->InsertNextId(id);
                id = vtkNewPoints->InsertNextPoint(p3);
                container->GetPointIds()->InsertNextId(id);
                vtkNewCells->InsertNextCell(container);
            }
        }
    }

    if (vtkNewCells->GetNumberOfCells()<=0)
        return false;

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
    return true;
}

bool mitk::FiberBundle::RemoveShortFibers(float lengthInMM)
{
    MITK_INFO << "Removing short fibers";
    if (lengthInMM<=0 || lengthInMM<m_MinFiberLength)
    {
        MITK_INFO << "No fibers shorter than " << lengthInMM << " mm found!";
        return true;
    }

    if (lengthInMM>m_MaxFiberLength)    // can't remove all fibers
    {
        MITK_WARN << "Process aborted. No fibers would be left!";
        return false;
    }

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();
    float min = m_MaxFiberLength;

    boost::progress_display disp(m_NumFibers);
    for (int i=0; i<m_NumFibers; i++)
    {
        ++disp;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        if (m_FiberLengths.at(i)>=lengthInMM)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            for (int j=0; j<numPoints; j++)
            {
                double* p = points->GetPoint(j);
                vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                container->GetPointIds()->InsertNextId(id);
            }
            vtkNewCells->InsertNextCell(container);
            if (m_FiberLengths.at(i)<min)
                min = m_FiberLengths.at(i);
        }
    }

    if (vtkNewCells->GetNumberOfCells()<=0)
        return false;

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
    return true;
}

bool mitk::FiberBundle::RemoveLongFibers(float lengthInMM)
{
    if (lengthInMM<=0 || lengthInMM>m_MaxFiberLength)
        return true;

    if (lengthInMM<m_MinFiberLength)    // can't remove all fibers
        return false;

    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    MITK_INFO << "Removing long fibers";
    boost::progress_display disp(m_NumFibers);
    for (int i=0; i<m_NumFibers; i++)
    {
        ++disp;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        if (m_FiberLengths.at(i)<=lengthInMM)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            for (int j=0; j<numPoints; j++)
            {
                double* p = points->GetPoint(j);
                vtkIdType id = vtkNewPoints->InsertNextPoint(p);
                container->GetPointIds()->InsertNextId(id);
            }
            vtkNewCells->InsertNextCell(container);
        }
    }

    if (vtkNewCells->GetNumberOfCells()<=0)
        return false;

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
    return true;
}

void mitk::FiberBundle::ResampleSpline(float pointDistance, double tension, double continuity, double bias )
{
    if (pointDistance<=0)
        return;

    vtkSmartPointer<vtkPoints> vtkSmoothPoints = vtkSmartPointer<vtkPoints>::New(); //in smoothpoints the interpolated points representing a fiber are stored.

    //in vtkcells all polylines are stored, actually all id's of them are stored
    vtkSmartPointer<vtkCellArray> vtkSmoothCells = vtkSmartPointer<vtkCellArray>::New(); //cellcontainer for smoothed lines
    vtkIdType pointHelperCnt = 0;

    MITK_INFO << "Smoothing fibers";
    boost::progress_display disp(m_NumFibers);
    for (int i=0; i<m_NumFibers; i++)
    {
        ++disp;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
        for (int j=0; j<numPoints; j++)
            newPoints->InsertNextPoint(points->GetPoint(j));

        float length = m_FiberLengths.at(i);
        int sampling = std::ceil(length/pointDistance);

        vtkSmartPointer<vtkKochanekSpline> xSpline = vtkSmartPointer<vtkKochanekSpline>::New();
        vtkSmartPointer<vtkKochanekSpline> ySpline = vtkSmartPointer<vtkKochanekSpline>::New();
        vtkSmartPointer<vtkKochanekSpline> zSpline = vtkSmartPointer<vtkKochanekSpline>::New();
        xSpline->SetDefaultBias(bias); xSpline->SetDefaultTension(tension); xSpline->SetDefaultContinuity(continuity);
        ySpline->SetDefaultBias(bias); ySpline->SetDefaultTension(tension); ySpline->SetDefaultContinuity(continuity);
        zSpline->SetDefaultBias(bias); zSpline->SetDefaultTension(tension); zSpline->SetDefaultContinuity(continuity);

        vtkSmartPointer<vtkParametricSpline> spline = vtkSmartPointer<vtkParametricSpline>::New();
        spline->SetXSpline(xSpline);
        spline->SetYSpline(ySpline);
        spline->SetZSpline(zSpline);
        spline->SetPoints(newPoints);

        vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
        functionSource->SetParametricFunction(spline);
        functionSource->SetUResolution(sampling);
        functionSource->SetVResolution(sampling);
        functionSource->SetWResolution(sampling);
        functionSource->Update();

        vtkPolyData* outputFunction = functionSource->GetOutput();
        vtkPoints* tmpSmoothPnts = outputFunction->GetPoints(); //smoothPoints of current fiber

        vtkSmartPointer<vtkPolyLine> smoothLine = vtkSmartPointer<vtkPolyLine>::New();
        smoothLine->GetPointIds()->SetNumberOfIds(tmpSmoothPnts->GetNumberOfPoints());

        for (int j=0; j<smoothLine->GetNumberOfPoints(); j++)
        {
            smoothLine->GetPointIds()->SetId(j, j+pointHelperCnt);
            vtkSmoothPoints->InsertNextPoint(tmpSmoothPnts->GetPoint(j));
        }
        vtkSmoothCells->InsertNextCell(smoothLine);
        pointHelperCnt += tmpSmoothPnts->GetNumberOfPoints();
    }

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkSmoothPoints);
    m_FiberPolyData->SetLines(vtkSmoothCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
    m_FiberSampling = 10/pointDistance;
}

void mitk::FiberBundle::ResampleSpline(float pointDistance)
{
    ResampleSpline(pointDistance, 0, 0, 0 );
}

unsigned long mitk::FiberBundle::GetNumberOfPoints()
{
    unsigned long points = 0;
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        points += cell->GetNumberOfPoints();
    }
    return points;
}

void mitk::FiberBundle::Compress(float error)
{
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    MITK_INFO << "Compressing fibers";
    unsigned long numRemovedPoints = 0;
    boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());

    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        ++disp;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        // calculate curvatures
        std::vector< int > removedPoints; removedPoints.resize(numPoints, 0);
        removedPoints[0]=-1; removedPoints[numPoints-1]=-1;

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

        bool pointFound = true;
        while (pointFound)
        {
            pointFound = false;
            double minError = error;
            int removeIndex = -1;

            for (int j=0; j<numPoints; j++)
            {
                if (removedPoints[j]==0)
                {
                    double cand[3];
                    points->GetPoint(j, cand);
                    vnl_vector_fixed< double, 3 > candV;
                    candV[0]=cand[0]; candV[1]=cand[1]; candV[2]=cand[2];

                    int validP = -1;
                    vnl_vector_fixed< double, 3 > pred;
                    for (int k=j-1; k>=0; k--)
                        if (removedPoints[k]<=0)
                        {
                            double ref[3];
                            points->GetPoint(k, ref);
                            pred[0]=ref[0]; pred[1]=ref[1]; pred[2]=ref[2];
                            validP = k;
                            break;
                        }
                    int validS = -1;
                    vnl_vector_fixed< double, 3 > succ;
                    for (int k=j+1; k<numPoints; k++)
                        if (removedPoints[k]<=0)
                        {
                            double ref[3];
                            points->GetPoint(k, ref);
                            succ[0]=ref[0]; succ[1]=ref[1]; succ[2]=ref[2];
                            validS = k;
                            break;
                        }

                    if (validP>=0 && validS>=0)
                    {
                        double a = (candV-pred).magnitude();
                        double b = (candV-succ).magnitude();
                        double c = (pred-succ).magnitude();
                        double s=0.5*(a+b+c);
                        double hc=(2.0/c)*sqrt(fabs(s*(s-a)*(s-b)*(s-c)));

                        if (hc<minError)
                        {
                            removeIndex = j;
                            minError = hc;
                            pointFound = true;
                        }
                    }
                }
            }

            if (pointFound)
            {
                removedPoints[removeIndex] = 1;
                numRemovedPoints++;
            }
        }

        for (int j=0; j<numPoints; j++)
        {
            if (removedPoints[j]<=0)
            {
                double cand[3];
                points->GetPoint(j, cand);
                vtkIdType id = vtkNewPoints->InsertNextPoint(cand);
                container->GetPointIds()->InsertNextId(id);
            }
        }

        vtkNewCells->InsertNextCell(container);
    }

    if (vtkNewCells->GetNumberOfCells()>0)
    {
        MITK_INFO << "Removed points: " << numRemovedPoints;
        m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
        m_FiberPolyData->SetPoints(vtkNewPoints);
        m_FiberPolyData->SetLines(vtkNewCells);
        this->SetFiberPolyData(m_FiberPolyData, true);
    }
}

// reapply selected colorcoding in case polydata structure has changed
bool mitk::FiberBundle::Equals(mitk::FiberBundle* fib, double eps)
{
    if (fib==nullptr)
    {
        MITK_INFO << "Reference bundle is NULL!";
        return false;
    }

    if (m_NumFibers!=fib->GetNumFibers())
    {
        MITK_INFO << "Unequal number of fibers!";
        MITK_INFO << m_NumFibers << " vs. " << fib->GetNumFibers();
        return false;
    }

    for (int i=0; i<m_NumFibers; i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        vtkCell* cell2 = fib->GetFiberPolyData()->GetCell(i);
        int numPoints2 = cell2->GetNumberOfPoints();
        vtkPoints* points2 = cell2->GetPoints();

        if (numPoints2!=numPoints)
        {
            MITK_INFO << "Unequal number of points in fiber " << i << "!";
            MITK_INFO << numPoints2 << " vs. " << numPoints;
            return false;
        }

        for (int j=0; j<numPoints; j++)
        {
            double* p1 = points->GetPoint(j);
            double* p2 = points2->GetPoint(j);
            if (fabs(p1[0]-p2[0])>eps || fabs(p1[1]-p2[1])>eps || fabs(p1[2]-p2[2])>eps)
            {
                MITK_INFO << "Unequal points in fiber " << i << " at position " << j << "!";
                MITK_INFO << "p1: " << p1[0] << ", " << p1[1] << ", " << p1[2];
                MITK_INFO << "p2: " << p2[0] << ", " << p2[1] << ", " << p2[2];
                return false;
            }
        }
    }

    return true;
}

/* ESSENTIAL IMPLEMENTATION OF SUPERCLASS METHODS */
void mitk::FiberBundle::UpdateOutputInformation()
{

}
void mitk::FiberBundle::SetRequestedRegionToLargestPossibleRegion()
{

}
bool mitk::FiberBundle::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}
bool mitk::FiberBundle::VerifyRequestedRegion()
{
    return true;
}
void mitk::FiberBundle::SetRequestedRegion(const itk::DataObject* )
{

}
