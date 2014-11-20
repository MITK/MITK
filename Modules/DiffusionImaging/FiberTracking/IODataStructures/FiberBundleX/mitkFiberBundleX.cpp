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
#include "mitkFiberBundleX.h"

#include <mitkPlanarCircle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarFigureComposite.h>
#include "mitkImagePixelReadAccessor.h"
#include <mitkPixelTypeMultiplex.h>

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

const char* mitk::FiberBundleX::COLORCODING_ORIENTATION_BASED = "Color_Orient";
//const char* mitk::FiberBundleX::COLORCODING_FA_AS_OPACITY = "Color_Orient_FA_Opacity";
const char* mitk::FiberBundleX::COLORCODING_FA_BASED = "FA_Values";
const char* mitk::FiberBundleX::COLORCODING_CUSTOM = "custom";
const char* mitk::FiberBundleX::FIBER_ID_ARRAY = "Fiber_IDs";

using namespace std;

mitk::FiberBundleX::FiberBundleX( vtkPolyData* fiberPolyData )
    : m_CurrentColorCoding(NULL)
    , m_NumFibers(0)
    , m_FiberSampling(0)
{
    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    if (fiberPolyData != NULL)
    {
        m_FiberPolyData = fiberPolyData;
        //m_FiberPolyData->DeepCopy(fiberPolyData);
        this->DoColorCodingOrientationBased();
    }

    this->UpdateFiberGeometry();
    this->SetColorCoding(COLORCODING_ORIENTATION_BASED);
    this->GenerateFiberIds();
}

mitk::FiberBundleX::~FiberBundleX()
{

}

mitk::FiberBundleX::Pointer mitk::FiberBundleX::GetDeepCopy()
{
    mitk::FiberBundleX::Pointer newFib = mitk::FiberBundleX::New(m_FiberPolyData);
    newFib->SetColorCoding(m_CurrentColorCoding);
    return newFib;
}

vtkSmartPointer<vtkPolyData> mitk::FiberBundleX::GeneratePolyDataByIds(std::vector<long> fiberIds)
{
    MITK_DEBUG << "\n=====FINAL RESULT: fib_id ======\n";
    MITK_DEBUG << "Number of new Fibers: " << fiberIds.size();
    // iterate through the vectorcontainer hosting all desired fiber Ids

    vtkSmartPointer<vtkPolyData> newFiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> newLineSet = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> newPointSet = vtkSmartPointer<vtkPoints>::New();

    // if FA array available, initialize fa double array
    // if color orient array is available init color array
    vtkSmartPointer<vtkDoubleArray> faValueArray;
    vtkSmartPointer<vtkUnsignedCharArray> colorsT;
    //colors and alpha value for each single point, RGBA = 4 components
    unsigned char rgba[4] = {0,0,0,0};
    int componentSize = sizeof(rgba);

    if (m_FiberIdDataSet->GetPointData()->HasArray(COLORCODING_FA_BASED)){
        MITK_DEBUG << "FA VALUES AVAILABLE, init array for new fiberbundle";
        faValueArray = vtkSmartPointer<vtkDoubleArray>::New();
    }

    if (m_FiberIdDataSet->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED)){
        MITK_DEBUG << "colorValues available, init array for new fiberbundle";
        colorsT = vtkUnsignedCharArray::New();
        colorsT->SetNumberOfComponents(componentSize);
        colorsT->SetName(COLORCODING_ORIENTATION_BASED);
    }



    std::vector<long>::iterator finIt = fiberIds.begin();
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
            //            MITK_DEBUG << "id: " << fiber->GetPointId(i);
            //            MITK_DEBUG << fibPoints->GetPoint(i)[0] << " | " << fibPoints->GetPoint(i)[1] << " | " << fibPoints->GetPoint(i)[2];
            newFiber->GetPointIds()->SetId(i, newPointSet->GetNumberOfPoints());
            newPointSet->InsertNextPoint(fibPoints->GetPoint(i)[0], fibPoints->GetPoint(i)[1], fibPoints->GetPoint(i)[2]);


            if (m_FiberIdDataSet->GetPointData()->HasArray(COLORCODING_FA_BASED)){
                //                MITK_DEBUG << m_FiberIdDataSet->GetPointData()->GetArray(FA_VALUE_ARRAY)->GetTuple(fiber->GetPointId(i));
            }

            if (m_FiberIdDataSet->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED)){
                //                MITK_DEBUG << "ColorValue: " << m_FiberIdDataSet->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED)->GetTuple(fiber->GetPointId(i))[0];
            }
        }

        newLineSet->InsertNextCell(newFiber);
        ++finIt;
    }

    newFiberPolyData->SetPoints(newPointSet);
    newFiberPolyData->SetLines(newLineSet);
    MITK_DEBUG << "new fiberbundle polydata points: " << newFiberPolyData->GetNumberOfPoints();
    MITK_DEBUG << "new fiberbundle polydata lines: " << newFiberPolyData->GetNumberOfLines();
    MITK_DEBUG << "=====================\n";

    //    mitk::FiberBundleX::Pointer newFib = mitk::FiberBundleX::New(newFiberPolyData);
    return newFiberPolyData;
}

// merge two fiber bundles
mitk::FiberBundleX::Pointer mitk::FiberBundleX::AddBundle(mitk::FiberBundleX* fib)
{
    if (fib==NULL)
    {
        MITK_WARN << "trying to call AddBundle with NULL argument";
        return NULL;
    }
    MITK_INFO << "Adding fibers";

    vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

    // add current fiber bundle
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
        vNewLines->InsertNextCell(container);
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
        vNewLines->InsertNextCell(container);
    }

    // initialize polydata
    vNewPolyData->SetPoints(vNewPoints);
    vNewPolyData->SetLines(vNewLines);

    // initialize fiber bundle
    mitk::FiberBundleX::Pointer newFib = mitk::FiberBundleX::New(vNewPolyData);
    return newFib;
}

// subtract two fiber bundles
mitk::FiberBundleX::Pointer mitk::FiberBundleX::SubtractBundle(mitk::FiberBundleX* fib)
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

        if (points==NULL || numPoints<=0)
            continue;

        int numFibers2 = fib->GetNumFibers();
        bool contained = false;
        for( int i2=0; i2<numFibers2; i2++ )
        {
            vtkCell* cell2 = fib->GetFiberPolyData()->GetCell(i2);
            int numPoints2 = cell2->GetNumberOfPoints();
            vtkPoints* points2 = cell2->GetPoints();

            if (points2==NULL)// || numPoints2<=0)
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
        return NULL;
    // initialize polydata
    vNewPolyData->SetPoints(vNewPoints);
    vNewPolyData->SetLines(vNewLines);

    // initialize fiber bundle
    return mitk::FiberBundleX::New(vNewPolyData);
}

itk::Point<float, 3> mitk::FiberBundleX::GetItkPoint(double point[3])
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
void mitk::FiberBundleX::SetFiberPolyData(vtkSmartPointer<vtkPolyData> fiberPD, bool updateGeometry)
{
    if (fiberPD == NULL)
        this->m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    else
    {
        m_FiberPolyData->DeepCopy(fiberPD);
        DoColorCodingOrientationBased();
    }

    m_NumFibers = m_FiberPolyData->GetNumberOfLines();

    if (updateGeometry)
        UpdateFiberGeometry();
    SetColorCoding(COLORCODING_ORIENTATION_BASED);
    GenerateFiberIds();
}

/*
 * return vtkPolyData
 */
vtkSmartPointer<vtkPolyData> mitk::FiberBundleX::GetFiberPolyData() const
{
    return m_FiberPolyData;
}

void mitk::FiberBundleX::DoColorCodingOrientationBased()
{
    //===== FOR WRITING A TEST ========================
    //  colorT size == tupelComponents * tupelElements
    //  compare color results
    //  to cover this code 100% also polydata needed, where colorarray already exists
    //  + one fiber with exactly 1 point
    //  + one fiber with 0 points
    //=================================================

    /*  make sure that processing colorcoding is only called when necessary */
    if ( m_FiberPolyData->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED) &&
         m_FiberPolyData->GetNumberOfPoints() ==
         m_FiberPolyData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED)->GetNumberOfTuples() )
    {
        // fiberstructure is already colorcoded
        MITK_DEBUG << " NO NEED TO REGENERATE COLORCODING! " ;
        this->ResetFiberOpacity();
        this->SetColorCoding(COLORCODING_ORIENTATION_BASED);
        return;
    }

    /* Finally, execute color calculation */
    vtkPoints* extrPoints = NULL;
    extrPoints = m_FiberPolyData->GetPoints();
    int numOfPoints = 0;
    if (extrPoints!=NULL)
        numOfPoints = extrPoints->GetNumberOfPoints();

    //colors and alpha value for each single point, RGBA = 4 components
    unsigned char rgba[4] = {0,0,0,0};
    //    int componentSize = sizeof(rgba);
    int componentSize = 4;

    vtkSmartPointer<vtkUnsignedCharArray> colorsT = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colorsT->Allocate(numOfPoints * componentSize);
    colorsT->SetNumberOfComponents(componentSize);
    colorsT->SetName(COLORCODING_ORIENTATION_BASED);

    /* checkpoint: does polydata contain any fibers */
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
                colorsT->InsertTupleValue(idList[i], rgba);
            } //end for loop
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
    }//end for loop

    m_FiberPolyData->GetPointData()->AddArray(colorsT);

    this->SetColorCoding(COLORCODING_ORIENTATION_BASED);

    //mini test, shall be ported to MITK TESTINGS!
    if (colorsT->GetSize() != numOfPoints*componentSize)
        MITK_DEBUG << "ALLOCATION ERROR IN INITIATING COLOR ARRAY";
}

void mitk::FiberBundleX::DoColorCodingFaBased()
{
    if(m_FiberPolyData->GetPointData()->HasArray(COLORCODING_FA_BASED) != 1 )
        return;

    this->SetColorCoding(COLORCODING_FA_BASED);
    //    this->GenerateFiberIds();
}

void mitk::FiberBundleX::DoUseFaFiberOpacity()
{
    if(m_FiberPolyData->GetPointData()->HasArray(COLORCODING_FA_BASED) != 1 )
        return;

    if(m_FiberPolyData->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED) != 1 )
        return;

    vtkDoubleArray* FAValArray = (vtkDoubleArray*) m_FiberPolyData->GetPointData()->GetArray(COLORCODING_FA_BASED);
    vtkUnsignedCharArray* ColorArray = dynamic_cast<vtkUnsignedCharArray*>  (m_FiberPolyData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED));

    for(long i=0; i<ColorArray->GetNumberOfTuples(); i++) {
        double faValue = FAValArray->GetValue(i);
        faValue = faValue * 255.0;
        ColorArray->SetComponent(i,3, (unsigned char) faValue );
    }

    this->SetColorCoding(COLORCODING_ORIENTATION_BASED);
    //    this->GenerateFiberIds();
}

void mitk::FiberBundleX::ResetFiberOpacity() {
    vtkUnsignedCharArray* ColorArray = dynamic_cast<vtkUnsignedCharArray*>  (m_FiberPolyData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED));
    if (ColorArray==NULL)
        return;
    for(long i=0; i<ColorArray->GetNumberOfTuples(); i++)
        ColorArray->SetComponent(i,3, 255.0 );
}

void mitk::FiberBundleX::SetFAMap(mitk::Image::Pointer FAimage)
{
    mitkPixelTypeMultiplex1( SetFAMap, FAimage->GetPixelType(), FAimage );
}

template <typename TPixel>
void mitk::FiberBundleX::SetFAMap(const mitk::PixelType, mitk::Image::Pointer FAimage)
{
    MITK_DEBUG << "SetFAMap";
    vtkSmartPointer<vtkDoubleArray> faValues = vtkSmartPointer<vtkDoubleArray>::New();
    faValues->SetName(COLORCODING_FA_BASED);
    faValues->Allocate(m_FiberPolyData->GetNumberOfPoints());
    faValues->SetNumberOfValues(m_FiberPolyData->GetNumberOfPoints());

    mitk::ImagePixelReadAccessor<TPixel,3> readFAimage (FAimage, FAimage->GetVolumeData(0));

    vtkPoints* pointSet = m_FiberPolyData->GetPoints();
    for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
    {
        Point3D px;
        px[0] = pointSet->GetPoint(i)[0];
        px[1] = pointSet->GetPoint(i)[1];
        px[2] = pointSet->GetPoint(i)[2];
        double faPixelValue = 1-readFAimage.GetPixelByWorldCoordinates(px);
        faValues->InsertValue(i, faPixelValue);
    }

    m_FiberPolyData->GetPointData()->AddArray(faValues);
    this->GenerateFiberIds();

    if(m_FiberPolyData->GetPointData()->HasArray(COLORCODING_FA_BASED))
        MITK_DEBUG << "FA VALUE ARRAY SET";

}


void mitk::FiberBundleX::GenerateFiberIds()
{
    if (m_FiberPolyData == NULL)
        return;

    vtkSmartPointer<vtkIdFilter> idFiberFilter = vtkSmartPointer<vtkIdFilter>::New();
    idFiberFilter->SetInputData(m_FiberPolyData);
    idFiberFilter->CellIdsOn();
    //  idFiberFilter->PointIdsOn(); // point id's are not needed
    idFiberFilter->SetIdsArrayName(FIBER_ID_ARRAY);
    idFiberFilter->FieldDataOn();
    idFiberFilter->Update();

    m_FiberIdDataSet = idFiberFilter->GetOutput();

    MITK_DEBUG << "Generating Fiber Ids...[done] | " << m_FiberIdDataSet->GetNumberOfCells();

}

mitk::FiberBundleX::Pointer mitk::FiberBundleX::ExtractFiberSubset(ItkUcharImgType* mask, bool anyPoint, bool invert)
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

        mitk::FiberBundleX::Pointer fibCopy = this->GetDeepCopy();
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
        }

        vtkNewCells->InsertNextCell(container);
    }

    if (vtkNewCells->GetNumberOfCells()<=0)
        return NULL;

    vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
    newPolyData->SetPoints(vtkNewPoints);
    newPolyData->SetLines(vtkNewCells);
    return mitk::FiberBundleX::New(newPolyData);
}

mitk::FiberBundleX::Pointer mitk::FiberBundleX::RemoveFibersOutside(ItkUcharImgType* mask, bool invert)
{
    float minSpacing = 1;
    if(mask->GetSpacing()[0]<mask->GetSpacing()[1] && mask->GetSpacing()[0]<mask->GetSpacing()[2])
        minSpacing = mask->GetSpacing()[0];
    else if (mask->GetSpacing()[1] < mask->GetSpacing()[2])
        minSpacing = mask->GetSpacing()[1];
    else
        minSpacing = mask->GetSpacing()[2];

    mitk::FiberBundleX::Pointer fibCopy = this->GetDeepCopy();
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
        return NULL;

    vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
    newPolyData->SetPoints(vtkNewPoints);
    newPolyData->SetLines(vtkNewCells);
    mitk::FiberBundleX::Pointer newFib = mitk::FiberBundleX::New(newPolyData);
    newFib->ResampleSpline(minSpacing/2);
    return newFib;
}

mitk::FiberBundleX::Pointer mitk::FiberBundleX::ExtractFiberSubset(BaseData* roi)
{
    if (roi==NULL || !(dynamic_cast<PlanarFigure*>(roi) || dynamic_cast<PlanarFigureComposite*>(roi)) )
        return NULL;

    std::vector<long> tmp = ExtractFiberIdSubset(roi);

    if (tmp.size()<=0)
        return mitk::FiberBundleX::New();
    vtkSmartPointer<vtkPolyData> pTmp = GeneratePolyDataByIds(tmp);
    return mitk::FiberBundleX::New(pTmp);
}

std::vector<long> mitk::FiberBundleX::ExtractFiberIdSubset(BaseData* roi)
{
    std::vector<long> result;
    if (roi==NULL)
        return result;

    mitk::PlanarFigureComposite::Pointer pfc = dynamic_cast<mitk::PlanarFigureComposite*>(roi);
    if (!pfc.IsNull()) // handle composite
    {
        switch (pfc->getOperationType())
        {
        case 0: // AND
        {
            result = this->ExtractFiberIdSubset(pfc->getChildAt(0));
            std::vector<long>::iterator it;
            for (int i=1; i<pfc->getNumberOfChildren(); ++i)
            {
                std::vector<long> inRoi = this->ExtractFiberIdSubset(pfc->getChildAt(i));

                std::vector<long> rest(std::min(result.size(),inRoi.size()));
                it = std::set_intersection(result.begin(), result.end(), inRoi.begin(), inRoi.end(), rest.begin() );
                rest.resize( it - rest.begin() );
                result = rest;
            }
            break;
        }
        case 1: // OR
        {
            result = ExtractFiberIdSubset(pfc->getChildAt(0));
            std::vector<long>::iterator it;
            for (int i=1; i<pfc->getNumberOfChildren(); ++i)
            {
                it = result.end();
                std::vector<long> inRoi = ExtractFiberIdSubset(pfc->getChildAt(i));
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
            for(long i=0; i<this->GetNumFibers(); i++)
                result.push_back(i);

            std::vector<long>::iterator it;
            for (long i=0; i<pfc->getNumberOfChildren(); ++i)
            {
                std::vector<long> inRoi = ExtractFiberIdSubset(pfc->getChildAt(i));

                std::vector<long> rest(result.size()-inRoi.size());
                it = std::set_difference(result.begin(), result.end(), inRoi.begin(), inRoi.end(), rest.begin() );
                rest.resize( it - rest.begin() );
                result = rest;
            }
            break;
        }
        }
    }
    else if ( dynamic_cast<mitk::PlanarFigure*>(roi) )  // actual extraction
    {
        mitk::PlanarFigure::Pointer planarFigure = dynamic_cast<mitk::PlanarFigure*>(roi);
        Vector3D planeNormal = planarFigure->GetPlaneGeometry()->GetNormal();
        planeNormal.Normalize();
        Point3D planeOrigin = planarFigure->GetPlaneGeometry()->GetOrigin();

        // define cutting plane by ROI geometry (PlanarFigure)
        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(planeOrigin[0],planeOrigin[1],planeOrigin[2]);
        plane->SetNormal(planeNormal[0],planeNormal[1],planeNormal[2]);

        // get all fiber/plane intersection points
        vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
        clipper->SetInputData(m_FiberIdDataSet);
        clipper->SetClipFunction(plane);
        clipper->GenerateClipScalarsOn();
        clipper->GenerateClippedOutputOn();
        clipper->Update();
        vtkSmartPointer<vtkPolyData> clipperout = clipper->GetClippedOutput();
        if (!clipperout->GetCellData()->HasArray(FIBER_ID_ARRAY))
            return result;

        vtkSmartPointer<vtkDataArray> distanceList = clipperout->GetPointData()->GetScalars();
        vtkIdType numPoints =  distanceList->GetNumberOfTuples();

        std::vector<int> pointsOnPlane;
        pointsOnPlane.reserve(numPoints);
        for (int i=0; i<numPoints; ++i)
        {
            double distance = distanceList->GetTuple(i)[0];
            // check if point is on plane
            if (distance >= -0.01 && distance <= 0.01)
                pointsOnPlane.push_back(i);
        }
        if (pointsOnPlane.empty())
            return result;

        // get all point IDs inside the ROI
        std::vector<int> pointsInROI;
        pointsInROI.reserve(pointsOnPlane.size());
        mitk::PlanarCircle::Pointer circleName = mitk::PlanarCircle::New();
        mitk::PlanarPolygon::Pointer polyName = mitk::PlanarPolygon::New();
        if ( planarFigure->GetNameOfClass() == circleName->GetNameOfClass() )
        {
            //calculate circle radius
            mitk::Point3D V1w = planarFigure->GetWorldControlPoint(0); //centerPoint
            mitk::Point3D V2w  = planarFigure->GetWorldControlPoint(1); //radiusPoint

            double radius = V1w.EuclideanDistanceTo(V2w);
            radius *= radius;

            for (unsigned int i=0; i<pointsOnPlane.size(); i++)
            {
                double p[3]; clipperout->GetPoint(pointsOnPlane[i], p);
                double dist = (p[0]-V1w[0])*(p[0]-V1w[0])+(p[1]-V1w[1])*(p[1]-V1w[1])+(p[2]-V1w[2])*(p[2]-V1w[2]);
                if( dist <= radius)
                    pointsInROI.push_back(pointsOnPlane[i]);
            }
        }
        else if ( planarFigure->GetNameOfClass() == polyName->GetNameOfClass() )
        {
            //create vtkPolygon using controlpoints from planarFigure polygon
            vtkSmartPointer<vtkPolygon> polygonVtk = vtkSmartPointer<vtkPolygon>::New();
            for (unsigned int i=0; i<planarFigure->GetNumberOfControlPoints(); ++i)
            {
                itk::Point<double,3> p = planarFigure->GetWorldControlPoint(i);
                polygonVtk->GetPoints()->InsertNextPoint(p[0], p[1], p[2] );
            }
            //prepare everything for using pointInPolygon function
            double n[3];
            polygonVtk->ComputeNormal(polygonVtk->GetPoints()->GetNumberOfPoints(), static_cast<double*>(polygonVtk->GetPoints()->GetData()->GetVoidPointer(0)), n);
            double bounds[6];
            polygonVtk->GetPoints()->GetBounds(bounds);

            for (unsigned int i=0; i<pointsOnPlane.size(); i++)
            {
                double p[3]; clipperout->GetPoint(pointsOnPlane[i], p);
                int isInPolygon = polygonVtk->PointInPolygon(p, polygonVtk->GetPoints()->GetNumberOfPoints(), static_cast<double*>(polygonVtk->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n);
                if( isInPolygon )
                    pointsInROI.push_back(pointsOnPlane[i]);
            }
        }
        if (pointsInROI.empty())
            return result;

        // get the fiber IDs corresponding to all clipped points
        std::vector< long > pointToFiberMap; // pointToFiberMap[PointID] = FiberIndex
        pointToFiberMap.resize(clipperout->GetNumberOfPoints());

        vtkCellArray* clipperlines = clipperout->GetLines();
        clipperlines->InitTraversal();
        for (int i=0, ic=0 ; i<clipperlines->GetNumberOfCells(); i++, ic+=3)
        {
            // ic is the index counter for the cells hosting the desired information. each cell consits of 3 items.
            long fiberID = clipperout->GetCellData()->GetArray(FIBER_ID_ARRAY)->GetTuple(i)[0];
            vtkIdType numPoints;
            vtkIdType* pointIDs;
            clipperlines->GetCell(ic, numPoints, pointIDs);

            for (long j=0; j<numPoints; j++)
                pointToFiberMap[ pointIDs[j] ] = fiberID;
        }

        // get the fiber IDs corresponding to the ID of a point inside the ROI
        result.reserve(pointsInROI.size());
        for (unsigned long k = 0; k < pointsInROI.size(); k++)
        {
            if (pointToFiberMap[pointsInROI[k]]<=GetNumFibers() && pointToFiberMap[pointsInROI[k]]>=0)
                result.push_back( pointToFiberMap[pointsInROI[k]] );
            else
                MITK_INFO << "ERROR in ExtractFiberIdSubset; impossible fiber id detected";
        }

        // remove duplicates
        std::vector<long>::iterator it;
        sort(result.begin(), result.end());
        it = unique (result.begin(), result.end());
        result.resize( it - result.begin() );
    }

    return result;
}

void mitk::FiberBundleX::UpdateFiberGeometry()
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

std::vector<std::string> mitk::FiberBundleX::GetAvailableColorCodings()
{
    std::vector<std::string> availableColorCodings;
    int numColors = m_FiberPolyData->GetPointData()->GetNumberOfArrays();
    for(int i=0; i<numColors; i++)
    {
        availableColorCodings.push_back(m_FiberPolyData->GetPointData()->GetArrayName(i));
    }

    //this controlstructure shall be implemented by the calling method
    if (availableColorCodings.empty())
        MITK_DEBUG << "no colorcodings available in fiberbundleX";

    return availableColorCodings;
}


char* mitk::FiberBundleX::GetCurrentColorCoding()
{
    return m_CurrentColorCoding;
}

void mitk::FiberBundleX::SetColorCoding(const char* requestedColorCoding)
{
    if (requestedColorCoding==NULL)
        return;

    if( strcmp (COLORCODING_ORIENTATION_BASED,requestedColorCoding) == 0 )    {
        this->m_CurrentColorCoding = (char*) COLORCODING_ORIENTATION_BASED;

    } else if( strcmp (COLORCODING_FA_BASED,requestedColorCoding) == 0 ) {
        this->m_CurrentColorCoding = (char*) COLORCODING_FA_BASED;

    } else if( strcmp (COLORCODING_CUSTOM,requestedColorCoding) == 0 ) {
        this->m_CurrentColorCoding = (char*) COLORCODING_CUSTOM;

    } else {
        MITK_DEBUG << "FIBERBUNDLE X: UNKNOWN COLORCODING in FIBERBUNDLEX Datastructure";
        this->m_CurrentColorCoding = (char*) COLORCODING_CUSTOM; //will cause blank colorcoding of fibers
    }

    m_UpdateTime3D.Modified();
    m_UpdateTime2D.Modified();
}

itk::Matrix< double, 3, 3 > mitk::FiberBundleX::TransformMatrix(itk::Matrix< double, 3, 3 > m, double rx, double ry, double rz)
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

itk::Point<float, 3> mitk::FiberBundleX::TransformPoint(vnl_vector_fixed< double, 3 > point, double rx, double ry, double rz, double tx, double ty, double tz)
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

void mitk::FiberBundleX::TransformFibers(double rx, double ry, double rz, double tx, double ty, double tz)
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
    UpdateColorCoding();
    UpdateFiberGeometry();
}

void mitk::FiberBundleX::RotateAroundAxis(double x, double y, double z)
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
    UpdateColorCoding();
    UpdateFiberGeometry();
}

void mitk::FiberBundleX::ScaleFibers(double x, double y, double z, bool subtractCenter)
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
    UpdateColorCoding();
    UpdateFiberGeometry();
}

void mitk::FiberBundleX::TranslateFibers(double x, double y, double z)
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
    UpdateColorCoding();
    UpdateFiberGeometry();
}

void mitk::FiberBundleX::MirrorFibers(unsigned int axis)
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
    UpdateColorCoding();
    UpdateFiberGeometry();
}

bool mitk::FiberBundleX::ApplyCurvatureThreshold(float minRadius, bool deleteFibers)
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

    UpdateColorCoding();
    UpdateFiberGeometry();
    return true;
}

bool mitk::FiberBundleX::RemoveShortFibers(float lengthInMM)
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

    UpdateColorCoding();
    UpdateFiberGeometry();
    return true;
}

bool mitk::FiberBundleX::RemoveLongFibers(float lengthInMM)
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
    UpdateColorCoding();
    UpdateFiberGeometry();
    return true;
}

void mitk::FiberBundleX::ResampleSpline(float pointDistance, double tension, double continuity, double bias )
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
    UpdateColorCoding();
    UpdateFiberGeometry();
    m_FiberSampling = 10/pointDistance;
}

void mitk::FiberBundleX::ResampleSpline(float pointDistance)
{
    ResampleSpline(pointDistance, 0, 0, 0 );
}

unsigned long mitk::FiberBundleX::GetNumberOfPoints()
{
    unsigned long points = 0;
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        points += cell->GetNumberOfPoints();
    }
    return points;
}

void mitk::FiberBundleX::Compress(float error)
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

        UpdateColorCoding();
        UpdateFiberGeometry();
    }
}

// reapply selected colorcoding in case polydata structure has changed
void mitk::FiberBundleX::UpdateColorCoding()
{
    char* cc = GetCurrentColorCoding();

    if( strcmp (COLORCODING_ORIENTATION_BASED,cc) == 0 )
        DoColorCodingOrientationBased();
    else if( strcmp (COLORCODING_FA_BASED,cc) == 0 )
        DoColorCodingFaBased();
}

// reapply selected colorcoding in case polydata structure has changed
bool mitk::FiberBundleX::Equals(mitk::FiberBundleX* fib, double eps)
{
    if (fib==NULL)
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
void mitk::FiberBundleX::UpdateOutputInformation()
{

}
void mitk::FiberBundleX::SetRequestedRegionToLargestPossibleRegion()
{

}
bool mitk::FiberBundleX::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}
bool mitk::FiberBundleX::VerifyRequestedRegion()
{
    return true;
}
void mitk::FiberBundleX::SetRequestedRegion(const itk::DataObject* )
{

}
