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


#include "mitkFiberBundleXThreadMonitor.h"



mitk::FiberBundleXThreadMonitor::FiberBundleXThreadMonitor()
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

mitk::FiberBundleXThreadMonitor::~FiberBundleXThreadMonitor()
{

}


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





QString mitk::FiberBundleXThreadMonitor::getStatus(){
  return m_monitorStatus;
}
void mitk::FiberBundleXThreadMonitor::setStatus(QString status){
  m_statusMutex.lock();
  m_monitorStatus = status;
  m_statusMutex.unlock();
}
mitk::Point2D mitk::FiberBundleXThreadMonitor::getStatusPosition(){
  return m_monitorStatusPosition;
}
void mitk::FiberBundleXThreadMonitor::setStatusPosition(mitk::Point2D pnt){
  m_monitorStatusPosition[0] = pnt[0];
  m_monitorStatusPosition[1] = pnt[1];
}
int mitk::FiberBundleXThreadMonitor::getStatusOpacity(){
  return m_monitorStatusOpacity;
}
void mitk::FiberBundleXThreadMonitor::setStatusOpacity(int opacity){
  m_monitorStatusOpacity = opacity;
}


int mitk::FiberBundleXThreadMonitor::getStarted(){
  return m_monitorStarted;
}
/* is thread safe :) */
void mitk::FiberBundleXThreadMonitor::setStarted(int val)
{
  m_startedMutex.lock();
  m_monitorStarted = val;
  m_startedMutex.unlock();
}
mitk::Point2D mitk::FiberBundleXThreadMonitor::getStartedPosition(){
  return m_monitorStartedPosition;
}
void mitk::FiberBundleXThreadMonitor::setStartedPosition(mitk::Point2D pnt){
  m_monitorStartedPosition[0] = pnt[0];
  m_monitorStartedPosition[1] = pnt[1];
}
int mitk::FiberBundleXThreadMonitor::getStartedOpacity(){
  return m_monitorStartedOpacity;
}
void mitk::FiberBundleXThreadMonitor::setStartedOpacity(int opacity){
  m_monitorStartedOpacity = opacity;
}




int mitk::FiberBundleXThreadMonitor::getFinished(){
  return m_monitorFinished;
}
void mitk::FiberBundleXThreadMonitor::setFinished(int val)
{
  m_finishedMutex.lock();
  m_monitorFinished = val;
  m_finishedMutex.unlock();
}
mitk::Point2D mitk::FiberBundleXThreadMonitor::getFinishedPosition(){
  return m_monitorFinishedPosition;
}
void mitk::FiberBundleXThreadMonitor::setFinishedPosition(mitk::Point2D pnt){
  m_monitorFinishedPosition[0] = pnt[0];
  m_monitorFinishedPosition[1] = pnt[1];
}
int mitk::FiberBundleXThreadMonitor::getFinishedOpacity(){
  return m_monitorFinishedOpacity;
}
void mitk::FiberBundleXThreadMonitor::setFinishedOpacity(int opacity){
  m_monitorFinishedOpacity = opacity;
}




int mitk::FiberBundleXThreadMonitor::getTerminated(){
  return m_monitorTerminated;
}
void mitk::FiberBundleXThreadMonitor::setTerminated(int val)
{
  m_terminatedMutex.lock();
  m_monitorTerminated = val;
  m_terminatedMutex.unlock();
}
mitk::Point2D mitk::FiberBundleXThreadMonitor::getTerminatedPosition(){
  return m_monitorTerminatedPosition;
}
void mitk::FiberBundleXThreadMonitor::setTerminatedPosition(mitk::Point2D pnt){
  m_monitorTerminatedPosition[0] = pnt[0];
  m_monitorTerminatedPosition[1] = pnt[1];
}
int mitk::FiberBundleXThreadMonitor::getTerminatedOpacity(){
  return m_monitorTerminatedOpacity;
}
void mitk::FiberBundleXThreadMonitor::setTerminatedOpacity(int opacity){
  m_monitorTerminatedOpacity = opacity;
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

