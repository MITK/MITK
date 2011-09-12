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


mitk::FiberBundleXThreadMonitor::FiberBundleXThreadMonitor()
: m_monitorBracketOpen("["),
  m_monitorBracketClose("]"),
  m_monitorHeading("Monitoring Fiberprocessing Threads")
{
  m_monitorBracketOpenPosition[0] = 10;
  m_monitorBracketOpenPosition[1] = 10;
  m_monitorBracketClosePosition[0] = m_monitorBracketOpenPosition[0];
  m_monitorBracketClosePosition[1] = m_monitorBracketOpenPosition[1];
}

mitk::FiberBundleXThreadMonitor::~FiberBundleXThreadMonitor()
{
 
}

void mitk::FiberBundleXThreadMonitor::setTextL1(QString txt)
{
  m_Label1 = txt;
}

QString mitk::FiberBundleXThreadMonitor::getTextL1()
{
  return m_Label1;
}
//setter textproperty opacity etc....
//setter color
//setter text




QString mitk::FiberBundleXThreadMonitor::getBracketOpen(){
  return m_monitorBracketOpen;
}
mitk::Point2D mitk::FiberBundleXThreadMonitor::getBracketOpenPosition(){
  return m_monitorBracketOpenPosition;
}
QString mitk::FiberBundleXThreadMonitor::getBracketClose(){
  return m_monitorBracketClose;
}
mitk::Point2D mitk::FiberBundleXThreadMonitor::getBracketClosePosition(){
  return m_monitorBracketClosePosition;
}
void mitk::FiberBundleXThreadMonitor::setBracketClosePosition(mitk::Point2D pnt){
  m_monitorBracketClosePosition[0] = pnt[0];
  m_monitorBracketClosePosition[1] = pnt[1];
}



QString mitk::FiberBundleXThreadMonitor::getHeading(){
  return m_monitorHeading;
}



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

