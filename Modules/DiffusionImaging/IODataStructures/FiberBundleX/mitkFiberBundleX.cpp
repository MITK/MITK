/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
 Version:   $Revision: 21975 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#include "mitkFiberBundleX.h"

#include <mitkPlanarCircle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarFigureComposite.h>


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


const char* mitk::FiberBundleX::COLORCODING_ORIENTATION_BASED = "Color_Orient";
//const char* mitk::FiberBundleX::COLORCODING_FA_AS_OPACITY = "Color_Orient_FA_Opacity";
const char* mitk::FiberBundleX::FA_VALUE_ARRAY = "FA_Values";
const char* mitk::FiberBundleX::COLORCODING_CUSTOM = "custom";
const char* mitk::FiberBundleX::FIBER_ID_ARRAY = "Fiber_IDs";


mitk::FiberBundleX::FiberBundleX( vtkPolyData* fiberPolyData )
    : m_currentColorCoding(NULL)
    , m_NumFibers(0)
{
    if (fiberPolyData == NULL)
        m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    else
        m_FiberPolyData = fiberPolyData;

    m_NumFibers = m_FiberPolyData->GetNumberOfLines();

    this->UpdateFiberGeometry();

    //generate colorcoding
    this->DoColorCodingOrientationbased();
    this->SetColorCoding(COLORCODING_ORIENTATION_BASED);
    this->DoGenerateFiberIds();


}

mitk::FiberBundleX::~FiberBundleX()
{

}

mitk::FiberBundleX::Pointer mitk::FiberBundleX::GetDeepCopy()
{
    mitk::FiberBundleX::Pointer newFib = mitk::FiberBundleX::New();

    //    newFib->m_FiberIdDataSet = vtkSmartPointer<vtkDataSet>::New();
    newFib->m_FiberIdDataSet->DeepCopy(m_FiberIdDataSet);
    newFib->m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    newFib->m_FiberPolyData->DeepCopy(m_FiberPolyData);
    newFib->SetColorCoding(m_currentColorCoding);
    newFib->m_NumFibers = m_NumFibers;
    newFib->UpdateFiberGeometry();

    return newFib;
}

vtkSmartPointer<vtkPolyData> mitk::FiberBundleX::GenerateNewFiberBundleByIds(std::vector<long> fiberIds)
{
    MITK_INFO << "\n=====FINAL RESULT: fib_id ======\n";
    MITK_INFO << "Number of new Fibers: " << fiberIds.size();
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

    if (m_FiberIdDataSet->GetPointData()->HasArray(FA_VALUE_ARRAY)){
        MITK_INFO << "FA VALUES AVAILABLE, init array for new fiberbundle";
        faValueArray = vtkSmartPointer<vtkDoubleArray>::New();
    }

    if (m_FiberIdDataSet->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED)){
        MITK_INFO << "colorValues available, init array for new fiberbundle";
        colorsT = vtkUnsignedCharArray::New();
        colorsT->SetNumberOfComponents(componentSize);
        colorsT->SetName(COLORCODING_ORIENTATION_BASED);
    }



    std::vector<long>::iterator finIt = fiberIds.begin();
    while ( finIt != fiberIds.end() )
    {
        if (*finIt < 0){
            MITK_INFO << "!!!!!! ERROR !!!!!! ERROR !!!!!\n=======================\nERROR, fiberID can not be negative!!! check id Extraction!" << *finIt;
            break;
        }
        vtkSmartPointer<vtkCell> fiber = m_FiberIdDataSet->GetCell(*finIt);//->DeepCopy(fiber);
        vtkSmartPointer<vtkPoints> fibPoints = fiber->GetPoints();

        vtkSmartPointer<vtkPolyLine> newFiber = vtkSmartPointer<vtkPolyLine>::New();
        newFiber->GetPointIds()->SetNumberOfIds( fibPoints->GetNumberOfPoints() );

        for(int i=0; i<fibPoints->GetNumberOfPoints(); i++)
        {
            //            MITK_INFO << "id: " << fiber->GetPointId(i);
            //            MITK_INFO << fibPoints->GetPoint(i)[0] << " | " << fibPoints->GetPoint(i)[1] << " | " << fibPoints->GetPoint(i)[2];
            newFiber->GetPointIds()->SetId(i, newPointSet->GetNumberOfPoints());
            newPointSet->InsertNextPoint(fibPoints->GetPoint(i)[0], fibPoints->GetPoint(i)[1], fibPoints->GetPoint(i)[2]);


            if (m_FiberIdDataSet->GetPointData()->HasArray(FA_VALUE_ARRAY)){
                //                MITK_INFO << m_FiberIdDataSet->GetPointData()->GetArray(FA_VALUE_ARRAY)->GetTuple(fiber->GetPointId(i));
            }

            if (m_FiberIdDataSet->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED)){
                //                MITK_INFO << "ColorValue: " << m_FiberIdDataSet->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED)->GetTuple(fiber->GetPointId(i))[0];
            }
        }

        newLineSet->InsertNextCell(newFiber);
        ++finIt;
    }

    newFiberPolyData->SetPoints(newPointSet);
    newFiberPolyData->SetLines(newLineSet);
    MITK_INFO << "new fiberbundle polydata points: " << newFiberPolyData->GetNumberOfPoints();
    MITK_INFO << "new fiberbundle polydata lines: " << newFiberPolyData->GetNumberOfLines();
    MITK_INFO << "=====================\n";

    //    mitk::FiberBundleX::Pointer newFib = mitk::FiberBundleX::New(newFiberPolyData);
    return newFiberPolyData;
}

// merge two fiber bundles
mitk::FiberBundleX::Pointer mitk::FiberBundleX::operator+(mitk::FiberBundleX* fib)
{

    vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

    vtkSmartPointer<vtkCellArray> vLines = m_FiberPolyData->GetLines();
    vLines->InitTraversal();

    // add current fiber bundle
    int numFibers = GetNumFibers();
    for( int i=0; i<numFibers; i++ )
    {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for( int j=0; j<numPoints; j++)
        {
            vtkIdType id = vNewPoints->InsertNextPoint(m_FiberPolyData->GetPoint(points[j]));
            container->GetPointIds()->InsertNextId(id);
        }
        vNewLines->InsertNextCell(container);
    }

    vLines = fib->m_FiberPolyData->GetLines();
    vLines->InitTraversal();

    // add new fiber bundle
    numFibers = fib->GetNumFibers();
    for( int i=0; i<numFibers; i++ )
    {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        for( int j=0; j<numPoints; j++)
        {
            vtkIdType id = vNewPoints->InsertNextPoint(fib->m_FiberPolyData->GetPoint(points[j]));
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
mitk::FiberBundleX::Pointer mitk::FiberBundleX::operator-(mitk::FiberBundleX* fib)
{

    vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

    vtkSmartPointer<vtkCellArray> vLines = m_FiberPolyData->GetLines();
    vLines->InitTraversal();

    // iterate over current fibers
    int numFibers = GetNumFibers();
    for( int i=0; i<numFibers; i++ )
    {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        vtkSmartPointer<vtkCellArray> vLines2 = fib->m_FiberPolyData->GetLines();
        vLines2->InitTraversal();
        int numFibers2 = fib->GetNumFibers();
        bool contained = false;
        for( int i2=0; i2<numFibers2; i2++ )
        {
            vtkIdType   numPoints2(0);
            vtkIdType*  points2(NULL);
            vLines2->GetNextCell ( numPoints2, points2 );

            // check endpoints
            itk::Point<float, 3> point_start = GetItkPoint(m_FiberPolyData->GetPoint(points[0]));
            itk::Point<float, 3> point_end = GetItkPoint(m_FiberPolyData->GetPoint(points[numPoints-1]));
            itk::Point<float, 3> point2_start = GetItkPoint(fib->m_FiberPolyData->GetPoint(points2[0]));
            itk::Point<float, 3> point2_end = GetItkPoint(fib->m_FiberPolyData->GetPoint(points2[numPoints2-1]));

            if (point_start.SquaredEuclideanDistanceTo(point2_start)<=mitk::eps && point_end.SquaredEuclideanDistanceTo(point2_end)<=mitk::eps ||
                    point_start.SquaredEuclideanDistanceTo(point2_end)<=mitk::eps && point_end.SquaredEuclideanDistanceTo(point2_start)<=mitk::eps)
            {
                // further checking ???
                contained = true;
            }
        }

        // add to result because fiber is not subtracted
        if (!contained)
        {
            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            for( int j=0; j<numPoints; j++)
            {
                vtkIdType id = vNewPoints->InsertNextPoint(m_FiberPolyData->GetPoint(points[j]));
                container->GetPointIds()->InsertNextId(id);
            }
            vNewLines->InsertNextCell(container);
        }
    }

    // initialize polydata
    vNewPolyData->SetPoints(vNewPoints);
    vNewPolyData->SetLines(vNewLines);

    // initialize fiber bundle
    mitk::FiberBundleX::Pointer newFib = mitk::FiberBundleX::New(vNewPolyData);
    return newFib;
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
        this->m_FiberPolyData = fiberPD;

    if (updateGeometry)
        UpdateFiberGeometry();

    m_NumFibers = m_FiberPolyData->GetNumberOfLines();

    //    m_isModified = true;
}

/*
 * return vtkPolyData
 */
vtkSmartPointer<vtkPolyData> mitk::FiberBundleX::GetFiberPolyData()
{
    return m_FiberPolyData;
}

void mitk::FiberBundleX::DoColorCodingOrientationbased()
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
        MITK_INFO << " NO NEED TO REGENERATE COLORCODING! " ;
        this->ResetFiberColorOpacity();
        this->SetColorCoding(COLORCODING_ORIENTATION_BASED);
        return;
    }

    /* Finally, execute color calculation */
    vtkPoints* extrPoints = m_FiberPolyData->GetPoints();
    int numOfPoints = 0;
    if (!extrPoints)
      numOfPoints = extrPoints->GetNumberOfPoints();

    //colors and alpha value for each single point, RGBA = 4 components
    unsigned char rgba[4] = {0,0,0,0};
    //    int componentSize = sizeof(rgba);
    int componentSize = 4;

    vtkSmartPointer<vtkUnsignedCharArray> colorsT = vtkUnsignedCharArray::New();
    colorsT->Allocate(numOfPoints * componentSize);
    colorsT->SetNumberOfComponents(componentSize);
    colorsT->SetName(COLORCODING_ORIENTATION_BASED);



    /* checkpoint: does polydata contain any fibers */
    int numOfFibers = m_FiberPolyData->GetNumberOfLines();
    if (numOfFibers < 1) {
        MITK_INFO << "\n ========= Number of Fibers is 0 and below ========= \n";
        return;
    }


    /* extract single fibers of fiberBundle */
    vtkCellArray* fiberList = m_FiberPolyData->GetLines();
    fiberList->InitTraversal();
    for (int fi=0; fi<numOfFibers; ++fi) {

        vtkIdType* idList; // contains the point id's of the line
        vtkIdType pointsPerFiber; // number of points for current line
        fiberList->GetNextCell(pointsPerFiber, idList);

        //    MITK_INFO << "Fib#: " << fi << " of " << numOfFibers << " pnts in fiber: " << pointsPerFiber ;

        /* single fiber checkpoints: is number of points valid */
        if (pointsPerFiber > 1)
        {
            /* operate on points of single fiber */
            for (int i=0; i <pointsPerFiber; ++i)
            {
                /* process all points except starting and endpoint
         * for calculating color value take current point, previous point and next point */
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


                } else if (i==0) {
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


                } else if (i==pointsPerFiber-1) {
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

        } else if (pointsPerFiber == 1) {
            /* a single point does not define a fiber (use vertex mechanisms instead */
            continue;
            //      colorsT->InsertTupleValue(0, rgba);

        } else {
            MITK_INFO << "Fiber with 0 points detected... please check your tractography algorithm!" ;
            continue;

        }


    }//end for loop

    m_FiberPolyData->GetPointData()->AddArray(colorsT);

    /*=========================
      - this is more relevant for renderer than for fiberbundleX datastructure
      - think about sourcing this to a explicit method which coordinates colorcoding */
    this->SetColorCoding(COLORCODING_ORIENTATION_BASED);
    //  ===========================

    //mini test, shall be ported to MITK TESTINGS!
    if (colorsT->GetSize() != numOfPoints*componentSize)
        MITK_INFO << "ALLOCATION ERROR IN INITIATING COLOR ARRAY";


}

void mitk::FiberBundleX::DoColorCodingFAbased()
{
    if(m_FiberPolyData->GetPointData()->HasArray(FA_VALUE_ARRAY) != 1 )
        return;

    this->SetColorCoding(FA_VALUE_ARRAY);
    MITK_INFO << "FBX: done CC FA based";
    this->DoGenerateFiberIds();
}

void mitk::FiberBundleX::DoUseFAasColorOpacity()
{
    if(m_FiberPolyData->GetPointData()->HasArray(FA_VALUE_ARRAY) != 1 )
        return;

    if(m_FiberPolyData->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED) != 1 )
        return;

    vtkDoubleArray* FAValArray = (vtkDoubleArray*) m_FiberPolyData->GetPointData()->GetArray(FA_VALUE_ARRAY);
    vtkUnsignedCharArray* ColorArray = dynamic_cast<vtkUnsignedCharArray*>  (m_FiberPolyData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED));

    for(long i=0; i<ColorArray->GetNumberOfTuples(); i++) {
        double faValue = FAValArray->GetValue(i);
        faValue = faValue * 255.0;
        ColorArray->SetComponent(i,3, (unsigned char) faValue );
    }

    this->SetColorCoding(COLORCODING_ORIENTATION_BASED);
    MITK_INFO << "FBX: done CC OPACITY";
    this->DoGenerateFiberIds();
}

void mitk::FiberBundleX::ResetFiberColorOpacity() {
    vtkUnsignedCharArray* ColorArray = dynamic_cast<vtkUnsignedCharArray*>  (m_FiberPolyData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED));
    for(long i=0; i<ColorArray->GetNumberOfTuples(); i++) {
        ColorArray->SetComponent(i,3, 255.0 );
    }
}

void mitk::FiberBundleX::SetFAMap(mitk::Image::Pointer FAimage)
{
    MITK_INFO << "SetFAMap";
    vtkSmartPointer<vtkDoubleArray> faValues = vtkDoubleArray::New();
    faValues->SetName(FA_VALUE_ARRAY);
    faValues->Allocate(m_FiberPolyData->GetNumberOfPoints());
    //    MITK_INFO << faValues->GetNumberOfTuples();
    //    MITK_INFO << faValues->GetSize();

    faValues->SetNumberOfValues(m_FiberPolyData->GetNumberOfPoints());
    //    MITK_INFO << faValues->GetNumberOfTuples();
    //    MITK_INFO << faValues->GetSize();

    vtkPoints* pointSet = m_FiberPolyData->GetPoints();
    for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
    {
        Point3D px;
        px[0] = pointSet->GetPoint(i)[0];
        px[1] = pointSet->GetPoint(i)[1];
        px[2] = pointSet->GetPoint(i)[2];
        double faPixelValue = FAimage->GetPixelValueByWorldCoordinate(px) * 0.01;
        //        faValues->InsertNextTuple1(faPixelValue);
        faValues->InsertValue(i, faPixelValue);
        //        MITK_INFO << faPixelValue;
        //        MITK_INFO << faValues->GetValue(i);

    }

    m_FiberPolyData->GetPointData()->AddArray(faValues);
    this->DoGenerateFiberIds();

    if(m_FiberPolyData->GetPointData()->HasArray(FA_VALUE_ARRAY))
        MITK_INFO << "FA VALUE ARRAY SET";

    //    vtkDoubleArray* valueArray = (vtkDoubleArray*) m_FiberPolyData->GetPointData()->GetArray(FA_VALUE_ARRAY);
    //    for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); i++)
    //    {
    //        MITK_INFO << "value at pos "<<  i << ": " << valueArray->GetValue(i);
    //    }
}

void mitk::FiberBundleX::DoGenerateFiberIds()
{
    if (m_FiberPolyData == NULL)
        return;

    vtkSmartPointer<vtkIdFilter> idFiberFilter = vtkSmartPointer<vtkIdFilter>::New();
    idFiberFilter->SetInput(m_FiberPolyData);
    idFiberFilter->CellIdsOn();
    //  idFiberFilter->PointIdsOn(); // point id's are not needed
    idFiberFilter->SetIdsArrayName(FIBER_ID_ARRAY);
    idFiberFilter->FieldDataOn();
    idFiberFilter->Update();

    m_FiberIdDataSet = idFiberFilter->GetOutput();

    MITK_INFO << "Generating Fiber Ids...[done] | " << m_FiberIdDataSet->GetNumberOfCells();

}


//==========================
std::vector<long> mitk::FiberBundleX::DoExtractFiberIds(mitk::PlanarFigure::Pointer pf)
{

    MITK_INFO << "Extracting fibers!";
    // vector which is returned, contains all extracted FiberIds
    std::vector<long> FibersInROI;

    /* Handle type of planarfigure */
    // if incoming pf is a pfc
    mitk::PlanarFigureComposite::Pointer pfcomp= dynamic_cast<mitk::PlanarFigureComposite*>(pf.GetPointer());
    if (!pfcomp.IsNull()) {
        // process requested boolean operation of PFC
        switch (pfcomp->getOperationType()) {
        case 0:
        {
            MITK_INFO << "AND PROCESSING";
            //AND
            //temporarly store results of the child in this vector, we need that to accumulate the
            std::vector<long> childResults = this->DoExtractFiberIds(pfcomp->getChildAt(0));
            MITK_INFO << "first roi got fibers in ROI: " << childResults.size();
            MITK_INFO << "sorting...";
            std::sort(childResults.begin(), childResults.end());
            MITK_INFO << "sorting done";
            std::vector<long> AND_Assamblage(childResults.size());
            //std::vector<unsigned long> AND_Assamblage;
            fill(AND_Assamblage.begin(), AND_Assamblage.end(), -1);
            //AND_Assamblage.reserve(childResults.size()); //max size AND can reach anyway

            std::vector<long>::iterator it;
            for (int i=1; i<pfcomp->getNumberOfChildren(); ++i)
            {
                std::vector<long> tmpChild = this->DoExtractFiberIds(pfcomp->getChildAt(i));
                MITK_INFO << "ROI " << i << " has fibers in ROI: " << tmpChild.size();
                sort(tmpChild.begin(), tmpChild.end());

                it = std::set_intersection(childResults.begin(), childResults.end(),
                                           tmpChild.begin(), tmpChild.end(),
                                           AND_Assamblage.begin() );
            }

            MITK_INFO << "resize Vector";
            long i=0;
            while (i < AND_Assamblage.size() && AND_Assamblage[i] != -1){ //-1 represents a placeholder in the array
                ++i;
            }
            AND_Assamblage.resize(i);

            MITK_INFO << "returning AND vector, size: " << AND_Assamblage.size();
            return AND_Assamblage;
            //            break;

        }
        case 1:
        {
            //OR
            std::vector<long> OR_Assamblage = this->DoExtractFiberIds(pfcomp->getChildAt(0));
            std::vector<long>::iterator it;
            MITK_INFO << OR_Assamblage.size();

            for (int i=1; i<pfcomp->getNumberOfChildren(); ++i) {
                it = OR_Assamblage.end();
                std::vector<long> tmpChild = this->DoExtractFiberIds(pfcomp->getChildAt(i));
                OR_Assamblage.insert(it, tmpChild.begin(), tmpChild.end());
                MITK_INFO << "ROI " << i << " has fibers in ROI: " << tmpChild.size() << " OR Assamblage: " << OR_Assamblage.size();
            }

            sort(OR_Assamblage.begin(), OR_Assamblage.end());
            it = unique(OR_Assamblage.begin(), OR_Assamblage.end());
            OR_Assamblage.resize( it - OR_Assamblage.begin() );
            MITK_INFO << "returning OR vector, size: " << OR_Assamblage.size();

            return OR_Assamblage;
        }
        case 2:
        {
            //NOT
            //get IDs of all fibers
            std::vector<long> childResults;
            childResults.reserve(this->GetNumFibers());
            vtkSmartPointer<vtkDataArray> idSet = m_FiberIdDataSet->GetCellData()->GetArray(FIBER_ID_ARRAY);
            MITK_INFO << "m_NumOfFib: " << this->GetNumFibers() << " cellIdNum: " << idSet->GetNumberOfTuples();
            for(long i=0; i<this->GetNumFibers(); i++)
            {
                MITK_INFO << "i: " << i << " idset: " << idSet->GetTuple(i)[0];
                childResults.push_back(idSet->GetTuple(i)[0]);
            }

            std::sort(childResults.begin(), childResults.end());
            std::vector<long> NOT_Assamblage(childResults.size());
            //fill it with -1, otherwise 0 will be stored and 0 can also be an ID of fiber!
            fill(NOT_Assamblage.begin(), NOT_Assamblage.end(), -1);
            std::vector<long>::iterator it;

            for (long i=0; i<pfcomp->getNumberOfChildren(); ++i)
            {
                std::vector<long> tmpChild = DoExtractFiberIds(pfcomp->getChildAt(i));
                sort(tmpChild.begin(), tmpChild.end());

                it = std::set_difference(childResults.begin(), childResults.end(),
                                         tmpChild.begin(), tmpChild.end(),
                                         NOT_Assamblage.begin() );

            }

            MITK_INFO << "resize Vector";
            long i=0;
            while (NOT_Assamblage[i] != -1){ //-1 represents a placeholder in the array
                ++i;
            }
            NOT_Assamblage.resize(i);

            return NOT_Assamblage;
        }
        default:
            MITK_INFO << "we have an UNDEFINED composition... ERROR" ;
            break;

        }
    } else
    {

        mitk::Geometry2D::ConstPointer pfgeometry = pf->GetGeometry2D();
        const mitk::PlaneGeometry* planeGeometry = dynamic_cast<const mitk::PlaneGeometry*> (pfgeometry.GetPointer());
        Vector3D planeNormal = planeGeometry->GetNormal();
        planeNormal.Normalize();
        Point3D planeOrigin = planeGeometry->GetOrigin();

        MITK_INFO << "planeOrigin: " << planeOrigin[0] << " | " << planeOrigin[1] << " | " << planeOrigin[2] << endl;
        MITK_INFO << "planeNormal: " << planeNormal[0] << " | " << planeNormal[1] << " | " << planeNormal[2] << endl;


        /* init necessary vectors hosting pointIds and FiberIds */
        // contains all pointIds which are crossing the cutting plane
        std::vector<int> PointsOnPlane;

        // based on PointsOnPlane, all ROI relevant point IDs are stored here
        std::vector<int> PointsInROI;


        /* Define cutting plane by ROI (PlanarFigure) */
        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(planeOrigin[0],planeOrigin[1],planeOrigin[2]);
        plane->SetNormal(planeNormal[0],planeNormal[1],planeNormal[2]);

        //same plane but opposite normal direction. so point cloud will be reduced -> better performance
        //        vtkSmartPointer<vtkPlane> planeR = vtkSmartPointer<vtkPlane>::New();

        //define new origin along the normal but close to the original one
        // OriginNew = OriginOld + 1*Normal
        //        Vector3D extendedNormal;
        //        int multiplyFactor = 1;
        //        extendedNormal[0] = planeNormal[0] * multiplyFactor;
        //        extendedNormal[1] = planeNormal[1] * multiplyFactor;
        //        extendedNormal[2] = planeNormal[2] * multiplyFactor;
        //        Point3D RplaneOrigin = planeOrigin - extendedNormal;
        //        planeR->SetOrigin(RplaneOrigin[0],RplaneOrigin[1],RplaneOrigin[2]);
        //        planeR->SetNormal(-planeNormal[0],-planeNormal[1],-planeNormal[2]);
        //        MITK_INFO << "RPlaneOrigin: " << RplaneOrigin[0] << " | " << RplaneOrigin[1]
        //                  << " | " << RplaneOrigin[2];

        /* get all points/fibers cutting the plane */
        MITK_INFO << "start clipping";
        vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
        clipper->SetInput(m_FiberIdDataSet);
        clipper->SetClipFunction(plane);
        clipper->GenerateClipScalarsOn();
        clipper->GenerateClippedOutputOn();
        vtkSmartPointer<vtkPolyData> clipperout = clipper->GetClippedOutput();
        MITK_INFO << "end clipping";

        /* for some reason clipperoutput is not initialized for futher processing
      * so far only writing out clipped polydata provides requested
      */
        //        MITK_INFO << "writing clipper output";
        //        vtkSmartPointer<vtkPolyDataWriter> writerC = vtkSmartPointer<vtkPolyDataWriter>::New();
        //        writerC->SetInput(clipperout1);
        //        writerC->SetFileName("/vtkOutput/Clipping.vtk");
        //        writerC->SetFileTypeToASCII();
        //        writerC->Write();
        //        MITK_INFO << "writing done";

        MITK_INFO << "init and update clipperoutput";
        clipperout->GetPointData()->Initialize();
        clipperout->Update();
        MITK_INFO << "init and update clipperoutput completed";

        //        MITK_INFO << "start clippingRecursive";
        //        vtkSmartPointer<vtkClipPolyData> Rclipper = vtkSmartPointer<vtkClipPolyData>::New();
        //        Rclipper->SetInput(clipperout1);
        //        Rclipper->SetClipFunction(planeR);
        //        Rclipper->GenerateClipScalarsOn();
        //        Rclipper->GenerateClippedOutputOn();
        //        vtkSmartPointer<vtkPolyData> clipperout = Rclipper->GetClippedOutput();
        //        MITK_INFO << "end clipping recursive";

        //        MITK_INFO << "writing clipper output 2";
        //        vtkSmartPointer<vtkPolyDataWriter> writerC1 = vtkSmartPointer<vtkPolyDataWriter>::New();
        //        writerC1->SetInput(clipperout);
        //        writerC1->SetFileName("/vtkOutput/RClipping.vtk");
        //        writerC1->SetFileTypeToASCII();
        //        writerC1->Write();
        //        MITK_INFO << "init and update clipperoutput";
        //        clipperout->GetPointData()->Initialize();
        //        clipperout->Update();
        //        MITK_INFO << "init and update clipperoutput completed";

        MITK_INFO << "STEP 1: find all points which have distance 0 to the given plane";
        /*======STEP 1======
      * extract all points, which are crossing the plane */
        // Scalar values describe the distance between each remaining point to the given plane. Values sorted by point index
        vtkSmartPointer<vtkDataArray> distanceList = clipperout->GetPointData()->GetScalars();
        vtkIdType sizeOfList =  distanceList->GetNumberOfTuples();
        PointsOnPlane.reserve(sizeOfList); /* use reserve for high-performant push_back, no hidden copy procedures are processed then!
                                         * size of list can be optimized by reducing allocation, but be aware of iterator and vector size*/

        for (int i=0; i<sizeOfList; ++i) {
            double *distance = distanceList->GetTuple(i);
            //std::cout << "distance of point " << i << " : " << distance[0] << std::endl;

            // check if point is on plane.
            // 0.01 due to some approximation errors when calculating distance
            if (distance[0] >= -0.01 && distance[0] <= 0.01)
            {
                //std::cout << "adding " << i << endl;
                PointsOnPlane.push_back(i); //push back in combination with reserve is fastest way to fill vector with various values

            }

        }

        // DEBUG print out all interesting points, stop where array starts with value -1. after -1 no more interesting idx are set!
        //        std::vector<int>::iterator rit = PointsOnPlane.begin();
        //        while (rit != PointsOnPlane.end() ) {
        //            std::cout << "interesting point: " << *rit << " coord: " << clipperout->GetPoint(*rit)[0] << " | " <<  clipperout->GetPoint(*rit)[1] << " | " << clipperout->GetPoint(*rit)[2] << endl;
        //            rit++;
        //        }


        MITK_INFO << "Num Of points on plane: " <<  PointsOnPlane.size();

        MITK_INFO << "Step 2: extract Interesting points with respect to given extraction planarFigure";

        PointsInROI.reserve(PointsOnPlane.size());
        /*=======STEP 2=====
     * extract ROI relevant pointIds */

        mitk::PlanarCircle::Pointer circleName = mitk::PlanarCircle::New();
        mitk::PlanarPolygon::Pointer polyName = mitk::PlanarPolygon::New();
        if (pf->GetNameOfClass() == circleName->GetNameOfClass() )
        {

            //calculate circle radius
            mitk::Point3D V1w = pf->GetWorldControlPoint(0); //centerPoint
            mitk::Point3D V2w  = pf->GetWorldControlPoint(1); //radiusPoint

            //calculate distance between those 2 and
            double distPF;
            distPF =  sqrt((double)  (V2w[0] - V1w[0]) * (V2w[0] - V1w[0]) +
                           (V2w[1] - V1w[1]) * (V2w[1] - V1w[1]) +
                           (V2w[2] - V1w[2]) * (V2w[2] - V1w[2]));

            //MITK_INFO << "Circle Radius: " << distPF;

            for (int i=0; i<PointsOnPlane.size(); i++)
            {

                //                MITK_INFO << clipperout->GetPoint(PointsOnPlane[i])[0] << " - " << V1w[0];
                //                MITK_INFO << clipperout->GetPoint(PointsOnPlane[i])[1] << " - " << V1w[1];
                //                MITK_INFO << clipperout->GetPoint(PointsOnPlane[i])[2] << " - " << V1w[2];

                //distance between circle radius and given point
                double XdistPnt =  sqrt((double) (clipperout->GetPoint(PointsOnPlane[i])[0] - V1w[0]) * (clipperout->GetPoint(PointsOnPlane[i])[0] - V1w[0]) +
                                        (clipperout->GetPoint(PointsOnPlane[i])[1] - V1w[1]) * (clipperout->GetPoint(PointsOnPlane[i])[1] - V1w[1]) +
                                        (clipperout->GetPoint(PointsOnPlane[i])[2] - V1w[2]) * (clipperout->GetPoint(PointsOnPlane[i])[2] - V1w[2])) ;

                //   MITK_INFO << "PntDistance to Radius: " << XdistPnt;
                if( XdistPnt <= distPF)
                {
                    // MITK_INFO << "point in Circle";
                    PointsInROI.push_back(PointsOnPlane[i]);
                }

            }//end for(i)
            // MITK_INFO << "Points inside circle radius: " << PointsInROI.size();
        }

        MITK_INFO << "Step3: Identify fibers";

        /*======STEP 3=======
     * identify fiberIds for points in ROI */

        // we need to access the fiberId Array, so make sure that this array is available
        if (!clipperout->GetCellData()->HasArray(FIBER_ID_ARRAY))
        {
            MITK_INFO << "ERROR: FiberID array does not exist, no correlation between points and fiberIds possible! Make sure calling GenerateFiberIds()";
            return FibersInROI; // FibersInRoi is empty then
        }

        // prepare a structure where each point id is represented as an indexId.
        // vector looks like: | pntId | fiberIdx |
        std::vector< long > pointindexFiberMap;

        // walk through the whole subline section and create an vector sorted by point index
        vtkCellArray *clipperlines = clipperout->GetLines();
        clipperlines->InitTraversal();
        long numOfLineCells = clipperlines->GetNumberOfCells();
        long numofClippedPoints = clipperout->GetNumberOfPoints();
        pointindexFiberMap.reserve(numofClippedPoints);


        //prepare resulting vector
        FibersInROI.reserve(PointsInROI.size());

        MITK_INFO << "\n===== Pointindex based structure initialized ======\n";

        // go through resulting "sub"lines which are stored as cells, "i" corresponds to current line id.
        for (int i=0, ic=0 ; i<numOfLineCells; i++, ic+=3)
        { //ic is the index counter for the cells hosting the desired information, eg. 2 | 45 | 46. each cell consits of 3 items.

            vtkIdType npts;
            vtkIdType *pts;
            clipperlines->GetCell(ic, npts, pts);

            // go through point ids in hosting subline, "j" corresponds to current pointindex in current line i. eg. idx[0]=45; idx[1]=46
            for (long j=0; j<npts; j++)
            {
                // MITK_INFO << "writing fiber id: " << clipperout->GetCellData()->GetArray(FIBER_ID_ARRAY)->GetTuple(i)[0] << " to pointId: " << pts[j];
                pointindexFiberMap[ pts[j] ] = clipperout->GetCellData()->GetArray(FIBER_ID_ARRAY)->GetTuple(i)[0];
                // MITK_INFO << "in array: " << pointindexFiberMap[ pts[j] ];
            }

        }

        MITK_INFO << "\n===== Pointindex based structure finalized ======\n";

        // get all Points in ROI with according fiberID
        for (long k = 0; k < PointsInROI.size(); k++)
        {
            //MITK_INFO << "point " << PointsInROI[k] << " belongs to fiber " << pointindexFiberMap[ PointsInROI[k] ];
            FibersInROI.push_back(pointindexFiberMap[ PointsInROI[k] ]);
        }

    }

    //  detecting fiberId duplicates
    MITK_INFO << "check for duplicates";

    sort(FibersInROI.begin(), FibersInROI.end());
    bool hasDuplicats = false;
    for(long i=0; i<FibersInROI.size()-1; ++i)
    {
        if(FibersInROI[i] == FibersInROI[i+1])
            hasDuplicats = true;
    }

    if(hasDuplicats)
    {
        std::vector<long>::iterator it;
        it = unique (FibersInROI.begin(), FibersInROI.end());
        FibersInROI.resize( it - FibersInROI.begin() );
    }

    return FibersInROI;
}

void mitk::FiberBundleX::UpdateFiberGeometry()
{
    if (m_NumFibers<=0)
    {
        mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
        geometry->SetImageGeometry(true);
        float b[] = {0, 1, 0, 1, 0, 1};
        geometry->SetFloatBounds(b);
        SetGeometry(geometry);
        return;
    }
    float min = itk::NumericTraits<float>::min();
    float max = itk::NumericTraits<float>::max();
    float b[] = {max, min, max, min, max, min};

    vtkCellArray* cells = m_FiberPolyData->GetLines();
    cells->InitTraversal();
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int p = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();
        for (int j=0; j<p; j++)
        {
            double p[3];
            points->GetPoint(j, p);

            if (p[0]<b[0])
                b[0]=p[0];
            if (p[0]>b[1])
                b[1]=p[0];

            if (p[1]<b[2])
                b[2]=p[1];
            if (p[1]>b[3])
                b[3]=p[1];

            if (p[2]<b[4])
                b[4]=p[2];
            if (p[2]>b[5])
                b[5]=p[2];
        }
    }

    // provide some border margin
    for(int i=0; i<=4; i+=2)
        b[i] -=10;
    for(int i=1; i<=5; i+=2)
        b[i] +=10;

    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    geometry->SetImageGeometry(true);
    geometry->SetFloatBounds(b);
    this->SetGeometry(geometry);
}

QStringList mitk::FiberBundleX::GetAvailableColorCodings()
{
    QStringList availableColorCodings;
    int numColors = m_FiberPolyData->GetPointData()->GetNumberOfArrays();
    for(int i=0; i<numColors; i++)
    {
        availableColorCodings.append(m_FiberPolyData->GetPointData()->GetArrayName(i));
    }

    //this controlstructure shall be implemented by the calling method
    if (availableColorCodings.isEmpty())
        MITK_INFO << "no colorcodings available in fiberbundleX";

    //    for(int i=0; i<availableColorCodings.size(); i++)
    //    {
    //            MITK_INFO << availableColorCodings.at(i).toLocal8Bit().constData();
    //    }

    return availableColorCodings;
}


char* mitk::FiberBundleX::GetCurrentColorCoding()
{
    return m_currentColorCoding;
}

void mitk::FiberBundleX::SetColorCoding(const char* requestedColorCoding)
{

    if (requestedColorCoding==NULL)
        return;
    MITK_INFO << "SetColorCoding:" << requestedColorCoding;

    if( strcmp (COLORCODING_ORIENTATION_BASED,requestedColorCoding) == 0 )    {
        this->m_currentColorCoding = (char*) COLORCODING_ORIENTATION_BASED;

    } else if( strcmp (FA_VALUE_ARRAY,requestedColorCoding) == 0 ) {
        this->m_currentColorCoding = (char*) FA_VALUE_ARRAY;

    } else if( strcmp (COLORCODING_CUSTOM,requestedColorCoding) == 0 ) {
        this->m_currentColorCoding = (char*) COLORCODING_CUSTOM;

    } else {
        MITK_INFO << "FIBERBUNDLE X: UNKNOWN COLORCODING in FIBERBUNDLEX Datastructure";
        this->m_currentColorCoding = (char*) COLORCODING_CUSTOM; //will cause blank colorcoding of fibers
    }
}


void mitk::FiberBundleX::ResampleFibers()
{
    mitk::Geometry3D::Pointer geometry = GetGeometry();
    mitk::Vector3D spacing = geometry->GetSpacing();

    float minSpacing = 1;
    if(spacing[0]<spacing[1] && spacing[0]<spacing[2])
        minSpacing = spacing[0];
    else if (spacing[1] < spacing[2])
        minSpacing = spacing[1];
    else
        minSpacing = spacing[2];

    ResampleFibers(minSpacing);
}

// Resample fiber to get equidistant points
void mitk::FiberBundleX::ResampleFibers(float len)
{
    vtkSmartPointer<vtkPolyData> newPoly = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> newCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    newPoints = vtkSmartPointer<vtkPoints>::New();

    vtkSmartPointer<vtkCellArray> vLines = m_FiberPolyData->GetLines();
    vLines->InitTraversal();
    int numberOfLines = m_NumFibers;

    for (int i=0; i<numberOfLines; i++)
    {
        vtkIdType   numPoints(0);
        vtkIdType*  points(NULL);
        vLines->GetNextCell ( numPoints, points );

        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();

        double* point = m_FiberPolyData->GetPoint(points[0]);
        vtkIdType pointId = newPoints->InsertNextPoint(point);
        container->GetPointIds()->InsertNextId(pointId);

        float dtau = 0;
        int cur_p = 1;
        itk::Vector<float,3> dR;
        float normdR = 0;

        for (;;)
        {
            while (dtau <= len && cur_p < numPoints)
            {
                itk::Vector<float,3> v1;
                point = m_FiberPolyData->GetPoint(points[cur_p-1]);
                v1[0] = point[0];
                v1[1] = point[1];
                v1[2] = point[2];
                itk::Vector<float,3> v2;
                point = m_FiberPolyData->GetPoint(points[cur_p]);
                v2[0] = point[0];
                v2[1] = point[1];
                v2[2] = point[2];

                dR  = v2 - v1;
                normdR = std::sqrt(dR.GetSquaredNorm());
                dtau += normdR;
                cur_p++;
            }

            if (dtau >= len)
            {
                itk::Vector<float,3> v1;
                point = m_FiberPolyData->GetPoint(points[cur_p-1]);
                v1[0] = point[0];
                v1[1] = point[1];
                v1[2] = point[2];

                itk::Vector<float,3> v2 = v1 - dR*( (dtau-len)/normdR );
                pointId = newPoints->InsertNextPoint(v2.GetDataPointer());
                container->GetPointIds()->InsertNextId(pointId);
            }
            else
            {
                point = m_FiberPolyData->GetPoint(points[numPoints-1]);
                pointId = newPoints->InsertNextPoint(point);
                container->GetPointIds()->InsertNextId(pointId);
                break;
            }
            dtau = dtau-len;
        }

        newCellArray->InsertNextCell(container);
    }

    newPoly->SetPoints(newPoints);
    newPoly->SetLines(newCellArray);
    m_FiberPolyData = newPoly;
    UpdateFiberGeometry();
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
void mitk::FiberBundleX::SetRequestedRegion( itk::DataObject *data )
{

}
