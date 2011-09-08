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


#include "mitkFiberBundleXThreadMonitor.h"
#include <mitkGeometry3D.h> // without geometry, fibers are not rendered


mitk::FiberBundleXThreadMonitor::FiberBundleXThreadMonitor()
{

  
}

mitk::FiberBundleXThreadMonitor::~FiberBundleXThreadMonitor()
{

}


//setter textproperty opacity etc....
//setter color
//setter text



/* ESSENTIAL IMPLEMENTATION OF SUPERCLASS METHODS */
void mitk::FiberBundleXThreadMonitor::UpdateOutputInformation()
{
  
}
void mitk::FiberBundleXThreadMonitor::SetRequestedRegionToLargestPossibleRegion()
{
  
}
bool mitk::FiberBundleXThreadMonitor::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}
bool mitk::FiberBundleXThreadMonitor::VerifyRequestedRegion()
{
  return true;
}
void mitk::FiberBundleXThreadMonitor::SetRequestedRegion( itk::DataObject *data )
{
  
}