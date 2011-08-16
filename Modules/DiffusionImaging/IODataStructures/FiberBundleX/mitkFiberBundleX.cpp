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
  /* === decide which polydata to choose ===
   * usually you call this method when u received fresh fibers from an tracking algorithm.
   * In this case the variable m_OriginalFiberPolyData will act as source for creating color
   * information for each point. However, if u process on fibers and forgot calling colorcoding 
   * before calling any other method (e.g. linesmoothing), then - for performance reason - it makes
   * sense not to process on the "original" pointset, but on the smoothed one (well, this might lack
   * in performance anyway ;-) ).
   *
   * It might occur that u call this method again - u must be drunk then - but this algorithm takes
   * care of ur incapability by checking if there already exists a color array for orientation based 
   * color information
   */
  vtkPolyData* fiberSource; //this variable provides the source where operations actually process on
  

  if ( m_FiberPolyData.GetPointer() == NULL ) 
  {
    fiberSource = m_OriginalFiberPolyData;
    // if there exists already a colorarray with orientationbased colors...just use it, otherwise it is not original data anymore :-)
    if (fiberSource->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED)) 
      return;
    

  } else if (m_FiberPolyData->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED) ) {
    fiberSource = m_FiberPolyData;
  }
  
  
  
  //colors and alpha value for each single point, RGBA = 4 components
  vtkUnsignedCharArray *colorsT = vtkUnsignedCharArray::New();
  colorsT->SetNumberOfComponents(4);
  colorsT->SetName("ColorcodingOrient");
  
  
  
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