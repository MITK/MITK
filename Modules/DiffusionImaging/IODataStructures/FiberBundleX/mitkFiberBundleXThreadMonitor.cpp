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
: m_monitorBracketOpen("[")
, m_monitorBracketClose("]")
, m_monitorHeading("Monitoring Fiberprocessing Threads")
, m_monitorMask("Mask Line1:\nLine2         Item2          Item3")
{
  m_monitorBracketOpenPosition[0] = 0;
  m_monitorBracketOpenPosition[1] = 0;
  m_monitorBracketClosePosition[0] = 0;
  m_monitorBracketClosePosition[1] = 0;
  m_monitorHeadingPosition[0] =  0;
  m_monitorHeadingPosition[1] =  0;
  m_monitorMaskPosition[0] = 0;
  m_monitorMaskPosition[1] = 0;
  
  
  m_monitorHeadingOpacity = 0;
  m_monitorMaskOpacity = 0;
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


void mitk::FiberBundleXThreadMonitor::setBracketOpenPosition(mitk::Point2D pnt){
  m_monitorBracketOpenPosition[0] = pnt[0];
  m_monitorBracketOpenPosition[1] = pnt[1];
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

mitk::Point2D mitk::FiberBundleXThreadMonitor::getHeadingPosition(){
  return m_monitorHeadingPosition;
}

void mitk::FiberBundleXThreadMonitor::setHeadingPosition(mitk::Point2D pnt){
  m_monitorHeadingPosition[0] = pnt[0];
  m_monitorHeadingPosition[1] = pnt[1];
}

int mitk::FiberBundleXThreadMonitor::getHeadingOpacity(){
  return m_monitorHeadingOpacity;
}

void mitk::FiberBundleXThreadMonitor::setHeadingOpacity(int opacity){
  m_monitorHeadingOpacity = opacity;
}

QString mitk::FiberBundleXThreadMonitor::getMask(){
  return m_monitorMask;
}

mitk::Point2D mitk::FiberBundleXThreadMonitor::getMaskPosition(){
  return m_monitorMaskPosition;
}

void mitk::FiberBundleXThreadMonitor::setMaskPosition(mitk::Point2D pnt){
  m_monitorMaskPosition[0] = pnt[0];
  m_monitorMaskPosition[1] = pnt[1];
}

int mitk::FiberBundleXThreadMonitor::getMaskOpacity(){
  return m_monitorMaskOpacity;
}

void mitk::FiberBundleXThreadMonitor::setMaskOpacity(int opacity){
  m_monitorMaskOpacity = opacity;
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

