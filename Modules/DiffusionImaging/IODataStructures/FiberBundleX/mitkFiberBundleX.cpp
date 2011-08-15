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


// implementation of all essential methods from superclass

mitk::FiberBundleX::FiberBundleX()
{
 


}

mitk::FiberBundleX::~FiberBundleX()
{

}

/* === main input method ====
 * set computed fibers from tractography algorithms
 */
void mitk::FiberBundleX::SetFibers(vtkSmartPointer<vtkPolyData> fiberPD)
{
  m_OriginalFiberPolyData = fiberPD;
}

 
/* === main output method ===
 * return fiberbundle as vtkPolyData
 * Depending on processing of input fibers, this method returns
 * the latest processed fibers.
 */
vtkPolyData* mitk::FiberBundleX::GetFibers()
{
  vtkPolyData* returningFibers = m_FiberPolyData;
  
  if (returningFibers == NULL) {
    returningFibers = m_OriginalFiberPolyData;
  }
  
  return returningFibers;
}

/*
 * return original set of fiberdata
 */
vtkPolyData* mitk::FiberBundleX::GetOriginalFibers()
{
  return m_OriginalFiberPolyData;
}


/*=================================
 *++++ PROCESSING OF FIBERS +++++++
 ================================*/

void mitk::FiberBundleX::DoColorCodingOrientationbased()
{
  vtkPolyData* fiberSource; //this variable provides the source where operations process on
  if (m_FiberPolyData.GetPointer() == NULL) {
    fiberSource = m_OriginalFiberPolyData;
    vtkSmartPointer<vtkPolyData> m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  } else {
    fiberSource = m_FiberPolyData;
  }
  
  
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


/* TUTORIAL INSERT POINTS / FIBERS to TRACTCONTAINER */
// points and vectors do not need to be initiated but itkVectorContainer
/*ContainerPointType pnt1;
 pnt1[0] = 1.0;
 pnt1[1] = 2.0;
 pnt1[2] = 3.0;

 ContainerPointType pnt2;
 pnt2[0] = 4.0;
 pnt2[1] = 5.0;
 pnt2[2] = 6.0;

 ContainerTractType tract1;
 tract1.push_back(pnt1);
 tract1.push_back(pnt2);

 ContainerType::Pointer testContainer = ContainerType::New();
 unsigned int freeIdx = testContainer->Size();
 MITK_INFO << freeIdx << "\n";
 testContainer->InsertElement(freeIdx, tract1);

 //iterate through all fibers stored in container
 for(ContainerType::ConstIterator itCont = testContainer->Begin();
 itCont != testContainer->End();
 itCont++)
 {
 //get single tract
 ContainerTractType tmp_fiber = itCont->Value();
 //  MITK_INFO << tmp_fiber << "\n";

 //iterate through all points within a fibertract
 for(ContainerTractType::iterator itPnt = tmp_fiber.begin();
 itPnt != tmp_fiber.end();
 ++itPnt)
 {
 // get single point with its coordinates
 ContainerPointType tmp_pntEx = *itPnt;
 MITK_INFO << tmp_pntEx[0] << "\n";
 MITK_INFO << tmp_pntEx[1] << "\n";
 MITK_INFO << tmp_pntEx[2] << "\n";
 }

 }
 ################### DTI FIBERs TUTORIAL ###########################
 TUTORIAL HOW TO READ POINTS / FIBERS from DTIGroupSpatialObjectContainer
 assume our dti fibers are stored in m_GroupFiberBundle

 // all smartPointers to fibers stored in in a ChildrenList
 ChildrenListType * FiberList;
 FiberList =  m_GroupFiberBundle->GetChildren();

 // iterate through container, itkcontainer groupFiberBundle in one iteration step
 for(ChildrenListType::iterator itLst = FiberList->begin();
 itLst != FiberList->end();
 ++FiberList)
 {
 // lists output is SpatialObject, we know we have DTITubeSpacialObjects
 // dynamic cast only likes pointers, no smartpointers, so each dsmartpointer has membermethod .GetPointer()
 itk::SpatialObject<3>::Pointer tmp_fbr;
 tmp_fbr = *itLst;
 DTITubeType::Pointer dtiTract = dynamic_cast<DTITubeType * > (tmp_fbr.GetPointer());
 if (dtiTract.IsNull()) {
 return; }

 //get list of points
 int fibrNrPnts = dtiTract->GetNumberOfPoints();
 DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();

 }


 */

