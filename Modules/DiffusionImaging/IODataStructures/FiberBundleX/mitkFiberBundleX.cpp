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

/* musthave */
//#include <mitkGeometry3D.h> // without geometry, fibers are not rendered

#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkIdFilter.h>

// baptize array names
const char* mitk::FiberBundleX::COLORCODING_ORIENTATION_BASED = "Color_Orient";
const char* mitk::FiberBundleX::COLORCODING_FA_BASED = "Color_FA";
const char* mitk::FiberBundleX::FIBER_ID_ARRAY = "Fiber_IDs";




mitk::FiberBundleX::FiberBundleX(vtkSmartPointer<vtkPolyData> fiberPolyData )
    : m_currentColorCoding(NULL)
    , m_isModified(false)
{
    //generate geometry of passed polydata
    if (fiberPolyData == NULL)
        this->m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    else
        this->m_FiberPolyData = fiberPolyData;

    this->UpdateFiberGeometry();


}


mitk::FiberBundleX::~FiberBundleX()
{
    // Memory Management
    m_FiberPolyData->Delete();
}

/* === main input method ====
 * set computed fibers from tractography algorithms
 */
void mitk::FiberBundleX::SetFiberPolyData(vtkSmartPointer<vtkPolyData> fiberPD)
{ 
    if (fiberPD == NULL)
        this->m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    else
        this->m_FiberPolyData = fiberPD;

    m_isModified = true;
}


/* === main output method ===
 * return fiberbundle as vtkPolyData
 * Depending on processing of input fibers, this method returns
 * the latest processed fibers.
 */
vtkSmartPointer<vtkPolyData> mitk::FiberBundleX::GetFiberPolyData()
{
    return m_FiberPolyData;
}




/*==============================================
 *++++ PROCESSING WITH FIBER INFORMATION +++++++
 =============================================*/

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
        MITK_INFO << " NO NEED TO REGENERATE COLORCODING!";
        return;
    }


    /* Finally, execute color calculation */
    vtkPoints* extrPoints = m_FiberPolyData->GetPoints();
    int numOfPoints = extrPoints->GetNumberOfPoints();

    //colors and alpha value for each single point, RGBA = 4 components
    unsigned char rgba[4] = {0,0,0,0};
    int componentSize = sizeof(rgba);

    vtkUnsignedCharArray *colorsT = vtkUnsignedCharArray::New();
    colorsT->Allocate(numOfPoints * componentSize);
    colorsT->SetNumberOfComponents(componentSize);
    colorsT->SetName(COLORCODING_ORIENTATION_BASED);



    /* checkpoint: does polydata contain any fibers */
    int numOfFibers = m_FiberPolyData->GetNumberOfLines();
    if (numOfFibers < 1) {
        MITK_INFO << "\n ========= Number of Fibers is below 1 ========= \n";
        return;
    }


    /* extract single fibers of fiberBundle */
    vtkCellArray* fiberList = m_FiberPolyData->GetLines();
    for (int fi=0; fi<numOfFibers; ++fi) {

        vtkIdType* idList; // contains the point id's of the line
        vtkIdType numOfPoints; // number of points for current line
        fiberList->GetNextCell(numOfPoints, idList);

        /* single fiber checkpoints: is number of points valid */
        if (numOfPoints > 1)
        {
            /* operate on points of single fiber */
            for (int i=0; i <numOfPoints; ++i)
            {
                /* process all points except starting and endpoint
         * for calculating color value take current point, previous point and next point */
                if (i<numOfPoints-1 && i > 0)
                {
                    /* The color value of the current point is influenced by the previous point and next point. */
                    vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
                    vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(idList[i+1])[0], extrPoints->GetPoint(idList[i+1])[1], extrPoints->GetPoint(idList[i+1])[2]);
                    vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(idList[i-1])[0], extrPoints->GetPoint(idList[i-1])[1], extrPoints->GetPoint(idList[i-1])[2]);

                    vnl_vector_fixed< double, 3 > diff1;
                    diff1 = currentPntvtk - nextPntvtk;
                    diff1.normalize();

                    vnl_vector_fixed< double, 3 > diff2;
                    diff2 = currentPntvtk - prevPntvtk;
                    diff2.normalize();

                    vnl_vector_fixed< double, 3 > diff;
                    diff = (diff1 - diff2) / 2.0;

                    rgba[0] = (unsigned char) (255.0 * std::abs(diff[0]));
                    rgba[1] = (unsigned char) (255.0 * std::abs(diff[1]));
                    rgba[2] = (unsigned char) (255.0 * std::abs(diff[2]));
                    rgba[3] = (unsigned char) (255.0);


                } else if (i==0) {
                    /* First point has no previous point, therefore only diff1 is taken */

                    vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
                    vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(idList[i+1])[0], extrPoints->GetPoint(idList[i+1])[1], extrPoints->GetPoint(idList[i+1])[2]);

                    vnl_vector_fixed< double, 3 > diff1;
                    diff1 = currentPntvtk - nextPntvtk;
                    diff1.normalize();

                    rgba[0] = (unsigned char) (255.0 * std::abs(diff1[0]));
                    rgba[1] = (unsigned char) (255.0 * std::abs(diff1[1]));
                    rgba[2] = (unsigned char) (255.0 * std::abs(diff1[2]));
                    rgba[3] = (unsigned char) (255.0);

                } else if (i==numOfPoints-1) {
                    /* Last point has no next point, therefore only diff2 is taken */
                    vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
                    vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(idList[i-1])[0], extrPoints->GetPoint(idList[i-1])[1], extrPoints->GetPoint(idList[i-1])[2]);

                    vnl_vector_fixed< double, 3 > diff2;
                    diff2 = currentPntvtk - prevPntvtk;
                    diff2.normalize();

                    rgba[0] = (unsigned char) (255.0 * std::abs(diff2[0]));
                    rgba[1] = (unsigned char) (255.0 * std::abs(diff2[1]));
                    rgba[2] = (unsigned char) (255.0 * std::abs(diff2[2]));
                    rgba[3] = (unsigned char) (255.0);

                }

                colorsT->InsertTupleValue(idList[i], rgba);
            } //end for loop

        } else if (numOfPoints == 1) {
            /* a single point does not define a fiber (use vertex mechanisms instead */
            continue;
            //      colorsT->InsertTupleValue(0, rgba);

        } else {
            MITK_INFO << "Fiber with 0 points detected... please check your tractography algorithm!" ;
            continue;

        }


    }//end for loop



    m_FiberPolyData->GetPointData()->AddArray(colorsT);
    m_currentColorCoding = (char*) COLORCODING_ORIENTATION_BASED;
    m_isModified = true;

    //mini test, shall be ported to MITK TESTINGS!
    if (colorsT->GetSize() != numOfPoints*componentSize) {
        MITK_INFO << "ALLOCATION ERROR IN INITIATING COLOR ARRAY";
    }


    //===== clean memory =====
    colorsT->Delete();

    //========================
}

void mitk::FiberBundleX::DoGenerateFiberIds()
{
    if (m_FiberPolyData == NULL)
        return;

    //  for (int i=0; i<10000000; ++i)
    //  {
    //   if(i%500 == 0)
    //     MITK_INFO << i;
    //  }
    //  MITK_INFO << "Generating Fiber Ids";
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


void mitk::FiberBundleX::UpdateFiberGeometry()
{


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

    // provide some buffer space at borders

    for(int i=0; i<=4; i+=2){
        b[i] -=10;
    }

    for(int i=1; i<=5; i+=2){
        b[i] +=10;
      }

    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    geometry->SetImageGeometry(true);
    geometry->SetFloatBounds(b);
    this->SetGeometry(geometry);




}

char* mitk::FiberBundleX::getCurrentColorCoding()
{
    return m_currentColorCoding;
}

bool mitk::FiberBundleX::isFiberBundleXModified()
{
    return m_isModified;
}
void mitk::FiberBundleX::setFBXModificationDone()
{
    m_isModified = false;
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
