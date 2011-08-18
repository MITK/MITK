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


/*==============================================
 *++++ PROCESSING WITH FIBER INFORMATION +++++++
 =============================================*/

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
  
  //these variables are needed for some intelligence in handling colorarrays and already smoothed structures
  //both vars are sensing the "colorful" part of fiberbundles ;-)
//  bool hasSmoothedFibColors = true; 
  bool hasOriginalFibColors = true;
  bool hasSmoothedFibColors = false;
   
  //check if color array in original fiber dataset is valid
  if ( m_OriginalFiberPolyData != NULL ) 
  {
    if ( m_OriginalFiberPolyData->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED) )
    {
      // validate input, number of items must match number of points
      if ( m_OriginalFiberPolyData->GetNumberOfPoints() != 
           m_OriginalFiberPolyData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED)->GetNumberOfTuples() )
      { 
        //invalid color array, try to heal
        MITK_INFO << "NUMBER OF POINTS DOES NOT MATCH COLOR INFORMATION in m_OriginalFiberPolyData, ARRAY: " << COLORCODING_ORIENTATION_BASED;
        hasOriginalFibColors = doSelfHealingColorOrient(m_OriginalFiberPolyData); 
        
      }//no else, cuz pointset matches color set :-)
      
    } else {
      //so far no color array exists
      hasOriginalFibColors = false;
    }
    
  } else { // else of check (m_OriginalFiberPolyData != NULL)
    MITK_INFO << "NO FIBERS FROM TRACTOGRAPHY PASSED TO mitkFiberBundleX yet!! no colorcoding can be processed!";
// nothing to implement or return here because hasOriginalFibColors is set to true and the checking mechanism below will take care of this issue as well
  }
  
  // make sure colorcoding in smoothed fibers are OK, otherwise do selfHealing
  if (hasOriginalFibColors &&  
      m_FiberPolyData.GetPointer() != NULL && 
      m_FiberPolyData->GetPointData()->HasArray(COLORCODING_ORIENTATION_BASED) && 
      (m_FiberPolyData->GetNumberOfPoints() != m_FiberPolyData->GetPointData()->GetArray(COLORCODING_ORIENTATION_BASED)->GetNumberOfTuples()) ) 
  {
    hasSmoothedFibColors = doSelfHealingColorOrient(m_FiberPolyData);
  }
  
  
//  make sure that processing colorcoding is only called when necessary
  if (hasOriginalFibColors)
    return;
  if (hasOriginalFibColors && hasSmoothedFibColors)
    return;
  
      
  //colors and alpha value for each single point, RGBA = 4 components
  vtkUnsignedCharArray *colorsT = vtkUnsignedCharArray::New();
  colorsT->SetNumberOfComponents(4);
  colorsT->SetName(COLORCODING_ORIENTATION_BASED);
  
  
  
}

//private repairMechanism for orientationbased colorcoding
bool mitk::FiberBundleX::doSelfHealingColorOrient(vtkPolyData* healMe)
{
  bool hasHealingSucceeded = false;
  MITK_INFO << "FiberBundleX self repair mechanism is called, but not yet implemented";
//  check type of healMe
  if (healMe == m_FiberPolyData) 
  {
    //todo
  
  } else if (healMe ==  m_OriginalFiberPolyData)
  {
    //todo
  }
  
  return hasHealingSucceeded;
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