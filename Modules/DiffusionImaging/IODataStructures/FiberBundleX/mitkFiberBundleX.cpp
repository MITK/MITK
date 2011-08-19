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

#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

// baptize array names
const char* mitk::FiberBundleX::COLORCODING_ORIENTATION_BASED = "Color_Orient";
const char* mitk::FiberBundleX::COLORCODING_FA_BASED = "Color_FA";


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
  m_FiberStructureData = fiberPD;
}


/* === main output method ===
 * return fiberbundle as vtkPolyData
 * Depending on processing of input fibers, this method returns
 * the latest processed fibers.
 */
vtkPolyData* mitk::FiberBundleX::GetFibers()
{
  return m_FiberStructureData;
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
  
  /* === decide which polydata to choose ===
   ** ALL REDESIGNED
   */
  
  bool hasFiberDataColor = false;
  
  // check if color array in original fiber dataset is valid
  if ( m_FiberStructureData != NULL ) 
  {
    if ( m_FiberStructureData->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED) && 
        m_FiberStructureData->GetNumberOfPoints() == 
        m_FiberStructureData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED)->GetNumberOfTuples() )
    {
      hasFiberDataColor = true; 
    }
    
  } else {
    MITK_INFO << "NO FIBERS FROM TRACTOGRAPHY PASSED TO mitkFiberBundleX yet!! no colorcoding can be processed!";
    hasFiberDataColor = true; // "true" will return later on
  }
  
  /*  make sure that processing colorcoding is only called when necessary */
  if (hasFiberDataColor)
    return;
  
  /* Finally, execute color calculation */
  vtkPoints* extrPoints = m_FiberStructureData->GetPoints();
  int numOfPoints = extrPoints->GetNumberOfPoints();
  
  //colors and alpha value for each single point, RGBA = 4 components
  unsigned char rgba[4] = {0,0,0,0};
  int componentSize = sizeof(rgba);
  
  vtkUnsignedCharArray *colorsT = vtkUnsignedCharArray::New();
  colorsT->Allocate(numOfPoints * componentSize);
  colorsT->SetNumberOfComponents(componentSize);
  colorsT->SetName(COLORCODING_ORIENTATION_BASED);
  
  /* catch case: fiber consists of only 1 point */
  if (numOfPoints > 1) 
  {
    
    for (int i=0; i <numOfPoints; ++i)
    {
      /* process all points except starting and endpoint
       * for calculating color value take current point, previous point and next point */
      if (i<numOfPoints-1 && i > 0)
      {
        /* The color value of the current point is influenced by the previous point and next point. */
        vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(i)[0], extrPoints->GetPoint(i)[1],extrPoints->GetPoint(i)[2]);
        vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(i+1)[0], extrPoints->GetPoint(i+1)[1], extrPoints->GetPoint(i+1)[2]);
        vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(i-1)[0], extrPoints->GetPoint(i-1)[1], extrPoints->GetPoint(i-1)[2]);
        
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
        
        vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(i)[0], extrPoints->GetPoint(i)[1],extrPoints->GetPoint(i)[2]);
        vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(i+1)[0], extrPoints->GetPoint(i+1)[1], extrPoints->GetPoint(i+1)[2]);
        
        vnl_vector_fixed< double, 3 > diff1;
        diff1 = currentPntvtk - nextPntvtk;
        diff1.normalize();
        
        rgba[0] = (unsigned char) (255.0 * std::abs(diff1[0]));
        rgba[1] = (unsigned char) (255.0 * std::abs(diff1[1]));
        rgba[2] = (unsigned char) (255.0 * std::abs(diff1[2]));
        rgba[3] = (unsigned char) (255.0); 
        
      } else if (i==numOfPoints-1) {
        /* Last point has no next point, therefore only diff2 is taken */
        vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(i)[0], extrPoints->GetPoint(i)[1],extrPoints->GetPoint(i)[2]);
        vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(i-1)[0], extrPoints->GetPoint(i-1)[1], extrPoints->GetPoint(i-1)[2]);
        
        vnl_vector_fixed< double, 3 > diff2;
        diff2 = currentPntvtk - prevPntvtk;
        diff2.normalize();
        
        rgba[0] = (unsigned char) (255.0 * std::abs(diff2[0]));
        rgba[1] = (unsigned char) (255.0 * std::abs(diff2[1]));
        rgba[2] = (unsigned char) (255.0 * std::abs(diff2[2]));
        rgba[3] = (unsigned char) (255.0); 
        
      }
      
      colorsT->InsertTupleValue(i, rgba);
    } //end for loop
    
  } else if (numOfPoints == 1) {
    /* Fiber consists of 1 point only, color that point as you wish :) */
    colorsT->InsertTupleValue(0, rgba);
    
  } else {
    MITK_INFO << "Fiber with 0 points detected... please check your tractography algorithm!" ; 
  }
  
  
  m_FiberStructureData->GetPointData()->AddArray(colorsT);
  
  //mini test, shall be ported to MITK TESTINGS!
  if (colorsT->GetSize() != numOfPoints*componentSize) {
    MITK_INFO << "ALLOCATION ERROR IN INITIATING COLOR ARRAY";
  }
  
  
//===== clean memory =====
  colorsT->Delete();

//========================
}

////private repairMechanism for orientationbased colorcoding
//bool mitk::FiberBundleX::doSelfHealingColorOrient(vtkPolyData* healMe)
//{
//  bool hasHealingSucceeded = false;
//  MITK_INFO << "FiberBundleX self repair mechanism is called, but not yet implemented";
////  check type of healMe
//  if (healMe ==  m_ImportedFiberData)
//  {
//    //todo
//  }
//  
//  return hasHealingSucceeded;
//}

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