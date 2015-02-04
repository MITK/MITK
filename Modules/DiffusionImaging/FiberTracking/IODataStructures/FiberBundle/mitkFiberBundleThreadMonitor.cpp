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


#include "mitkFiberBundleThreadMonitor.h"



mitk::FiberBundleThreadMonitor::FiberBundleThreadMonitor()
: m_monitorBracketOpen("[")
, m_monitorBracketClose("]")
, m_monitorHeading("Monitoring Fiberprocessing Threads")
, m_monitorMask("Thread Status:\nStarted:         Finished:         Terminated:")
, m_monitorStatus("_initialized")
, m_monitorStarted(0)
, m_monitorFinished(0)
, m_monitorTerminated(0)
{
  m_monitorBracketOpenPosition[0] = 0;
  m_monitorBracketOpenPosition[1] = 0;
  m_monitorBracketClosePosition[0] = 0;
  m_monitorBracketClosePosition[1] = 0;
  m_monitorHeadingPosition[0] =  0;
  m_monitorHeadingPosition[1] =  0;
  m_monitorMaskPosition[0] = 0;
  m_monitorMaskPosition[1] = 0;
  m_monitorStatusPosition[0] = 0;
  m_monitorStatusPosition[1] = 0;
  m_monitorStartedPosition[0] = 0;
  m_monitorStartedPosition[1] = 0;
  m_monitorFinishedPosition[0] = 0;
  m_monitorFinishedPosition[1] = 0;
  m_monitorTerminatedPosition[0] = 0;
  m_monitorTerminatedPosition[1] = 0;


  m_monitorHeadingOpacity = 0;
  m_monitorMaskOpacity = 0;
  m_monitorTerminatedOpacity = 0;
  m_monitorFinishedOpacity = 0;
  m_monitorStartedOpacity = 0;
  m_monitorStatusOpacity = 0;
}

mitk::FiberBundleThreadMonitor::~FiberBundleThreadMonitor()
{

}


QString mitk::FiberBundleThreadMonitor::getBracketOpen(){
  return m_monitorBracketOpen;
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getBracketOpenPosition(){
  return m_monitorBracketOpenPosition;
}
void mitk::FiberBundleThreadMonitor::setBracketOpenPosition(mitk::Point2D pnt){
  m_monitorBracketOpenPosition[0] = pnt[0];
  m_monitorBracketOpenPosition[1] = pnt[1];
}




QString mitk::FiberBundleThreadMonitor::getBracketClose(){
  return m_monitorBracketClose;
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getBracketClosePosition(){
  return m_monitorBracketClosePosition;
}
void mitk::FiberBundleThreadMonitor::setBracketClosePosition(mitk::Point2D pnt){
  m_monitorBracketClosePosition[0] = pnt[0];
  m_monitorBracketClosePosition[1] = pnt[1];
}



QString mitk::FiberBundleThreadMonitor::getHeading(){
  return m_monitorHeading;
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getHeadingPosition(){
  return m_monitorHeadingPosition;
}
void mitk::FiberBundleThreadMonitor::setHeadingPosition(mitk::Point2D pnt){
  m_monitorHeadingPosition[0] = pnt[0];
  m_monitorHeadingPosition[1] = pnt[1];
}
int mitk::FiberBundleThreadMonitor::getHeadingOpacity(){
  return m_monitorHeadingOpacity;
}
void mitk::FiberBundleThreadMonitor::setHeadingOpacity(int opacity){
  m_monitorHeadingOpacity = opacity;
}




QString mitk::FiberBundleThreadMonitor::getMask(){
  return m_monitorMask;
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getMaskPosition(){
  return m_monitorMaskPosition;
}
void mitk::FiberBundleThreadMonitor::setMaskPosition(mitk::Point2D pnt){
  m_monitorMaskPosition[0] = pnt[0];
  m_monitorMaskPosition[1] = pnt[1];
}
int mitk::FiberBundleThreadMonitor::getMaskOpacity(){
  return m_monitorMaskOpacity;
}
void mitk::FiberBundleThreadMonitor::setMaskOpacity(int opacity){
  m_monitorMaskOpacity = opacity;
}





QString mitk::FiberBundleThreadMonitor::getStatus(){
  return m_monitorStatus;
}
void mitk::FiberBundleThreadMonitor::setStatus(QString status){
  m_statusMutex.lock();
  m_monitorStatus = status;
  m_statusMutex.unlock();
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getStatusPosition(){
  return m_monitorStatusPosition;
}
void mitk::FiberBundleThreadMonitor::setStatusPosition(mitk::Point2D pnt){
  m_monitorStatusPosition[0] = pnt[0];
  m_monitorStatusPosition[1] = pnt[1];
}
int mitk::FiberBundleThreadMonitor::getStatusOpacity(){
  return m_monitorStatusOpacity;
}
void mitk::FiberBundleThreadMonitor::setStatusOpacity(int opacity){
  m_monitorStatusOpacity = opacity;
}


int mitk::FiberBundleThreadMonitor::getStarted(){
  return m_monitorStarted;
}
/* is thread safe :) */
void mitk::FiberBundleThreadMonitor::setStarted(int val)
{
  m_startedMutex.lock();
  m_monitorStarted = val;
  m_startedMutex.unlock();
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getStartedPosition(){
  return m_monitorStartedPosition;
}
void mitk::FiberBundleThreadMonitor::setStartedPosition(mitk::Point2D pnt){
  m_monitorStartedPosition[0] = pnt[0];
  m_monitorStartedPosition[1] = pnt[1];
}
int mitk::FiberBundleThreadMonitor::getStartedOpacity(){
  return m_monitorStartedOpacity;
}
void mitk::FiberBundleThreadMonitor::setStartedOpacity(int opacity){
  m_monitorStartedOpacity = opacity;
}




int mitk::FiberBundleThreadMonitor::getFinished(){
  return m_monitorFinished;
}
void mitk::FiberBundleThreadMonitor::setFinished(int val)
{
  m_finishedMutex.lock();
  m_monitorFinished = val;
  m_finishedMutex.unlock();
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getFinishedPosition(){
  return m_monitorFinishedPosition;
}
void mitk::FiberBundleThreadMonitor::setFinishedPosition(mitk::Point2D pnt){
  m_monitorFinishedPosition[0] = pnt[0];
  m_monitorFinishedPosition[1] = pnt[1];
}
int mitk::FiberBundleThreadMonitor::getFinishedOpacity(){
  return m_monitorFinishedOpacity;
}
void mitk::FiberBundleThreadMonitor::setFinishedOpacity(int opacity){
  m_monitorFinishedOpacity = opacity;
}




int mitk::FiberBundleThreadMonitor::getTerminated(){
  return m_monitorTerminated;
}
void mitk::FiberBundleThreadMonitor::setTerminated(int val)
{
  m_terminatedMutex.lock();
  m_monitorTerminated = val;
  m_terminatedMutex.unlock();
}
mitk::Point2D mitk::FiberBundleThreadMonitor::getTerminatedPosition(){
  return m_monitorTerminatedPosition;
}
void mitk::FiberBundleThreadMonitor::setTerminatedPosition(mitk::Point2D pnt){
  m_monitorTerminatedPosition[0] = pnt[0];
  m_monitorTerminatedPosition[1] = pnt[1];
}
int mitk::FiberBundleThreadMonitor::getTerminatedOpacity(){
  return m_monitorTerminatedOpacity;
}
void mitk::FiberBundleThreadMonitor::setTerminatedOpacity(int opacity){
  m_monitorTerminatedOpacity = opacity;
}



/* ESSENTIAL IMPLEMENTATION OF SUPERCLASS METHODS */
void mitk::FiberBundleThreadMonitor::UpdateOutputInformation()
{

}
void mitk::FiberBundleThreadMonitor::SetRequestedRegionToLargestPossibleRegion()
{

}
bool mitk::FiberBundleThreadMonitor::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}
bool mitk::FiberBundleThreadMonitor::VerifyRequestedRegion()
{
  return true;
}
void mitk::FiberBundleThreadMonitor::SetRequestedRegion(const itk::DataObject *data )
{

}

