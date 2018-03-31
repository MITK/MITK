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



#include "mitkFiberBundleThreadMonitorMapper3D.h"
#include <mitkProperties.h>
#include <vtkPropAssembly.h>
//#include <vtkTextActor.h>
#include <vtkTextProperty.h>




mitk::FiberBundleThreadMonitorMapper3D::FiberBundleThreadMonitorMapper3D()
: m_FiberMonitorMapper(vtkSmartPointer<vtkPolyDataMapper>::New())
, m_TextActorClose(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorOpen(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorHeading(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorMask(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorStatus(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorStarted(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorFinished(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorTerminated(vtkSmartPointer<vtkTextActor>::New())
, m_FiberAssembly(vtkPropAssembly::New())
, m_lastModifiedMonitorNodeTime(-1)
{
  m_FiberAssembly->AddPart(m_TextActorClose);
  m_FiberAssembly->AddPart(m_TextActorOpen);
  m_FiberAssembly->AddPart(m_TextActorHeading);
  m_FiberAssembly->AddPart(m_TextActorMask);
  m_FiberAssembly->AddPart(m_TextActorStatus);
  m_FiberAssembly->AddPart(m_TextActorStarted);
  m_FiberAssembly->AddPart(m_TextActorFinished);
  m_FiberAssembly->AddPart(m_TextActorTerminated);

}


mitk::FiberBundleThreadMonitorMapper3D::~FiberBundleThreadMonitorMapper3D()
{
  m_FiberAssembly->Delete();
}


const mitk::FiberBundleThreadMonitor* mitk::FiberBundleThreadMonitorMapper3D::GetInput()
{
  return static_cast<const mitk::FiberBundleThreadMonitor * > ( GetDataNode()->GetData() );
}


/*
 This method is called once the mapper gets new input,
 for UI rotation or changes in colorcoding this method is NOT called
 */
void mitk::FiberBundleThreadMonitorMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if ( !visible ) return;

  const DataNode *node = this->GetDataNode();

  if (m_lastModifiedMonitorNodeTime >= node->GetMTime())
    return;

  m_lastModifiedMonitorNodeTime = node->GetMTime();

  //  MITK_INFO << m_LastUpdateTime;
  FiberBundleThreadMonitor* monitor = dynamic_cast<FiberBundleThreadMonitor * > ( GetDataNode()->GetData() );

//  m_TextActor->SetInput( monitor->getTextL1().toStdString().c_str() );
  m_TextActorClose->SetInput( monitor->getBracketClose().toStdString().c_str() );
  vtkTextProperty* tpropClose = m_TextActorClose->GetTextProperty();
  //tprop->SetFontFamilyToArial ();
  //tprop->SetLineSpacing(1.0);
  tpropClose->SetFontSize(16);
  tpropClose->SetColor(0.85,0.8,0.8);
  m_TextActorClose->SetDisplayPosition( monitor->getBracketClosePosition()[0], monitor->getBracketClosePosition()[1] );
  //m_TextActorClose->Modified();


  m_TextActorOpen->SetInput( monitor->getBracketOpen().toStdString().c_str() );
  vtkTextProperty* tpropOpen = m_TextActorOpen->GetTextProperty();
  //tprop->SetFontFamilyToArial ();
  //tprop->SetLineSpacing(1.0);
  tpropOpen->SetFontSize(16);
  tpropOpen->SetColor(0.85,0.8,0.8);
  m_TextActorOpen->SetDisplayPosition( monitor->getBracketOpenPosition()[0], monitor->getBracketOpenPosition()[1] );
  //m_TextActorOpen->Modified();


  m_TextActorHeading->SetInput(  monitor->getHeading().toStdString().c_str() );
  vtkTextProperty* tpropHeading = m_TextActorHeading->GetTextProperty();
  tpropHeading->SetFontSize(12);
  tpropHeading->SetOpacity( monitor->getHeadingOpacity() * 0.1 );
  tpropHeading->SetColor(0.85,0.8,0.8);
  m_TextActorHeading->SetDisplayPosition( monitor->getHeadingPosition()[0], monitor->getHeadingPosition()[1] );
  //m_TextActorHeading->Modified();


  m_TextActorMask->SetInput(  monitor->getMask().toStdString().c_str() );
  vtkTextProperty* tpropMask = m_TextActorMask->GetTextProperty();
  tpropMask->SetFontSize(12);
  tpropMask->SetOpacity( monitor->getMaskOpacity() * 0.1 );
  tpropMask->SetColor(1.0,1.0,1.0);
  m_TextActorMask->SetDisplayPosition( monitor->getMaskPosition()[0], monitor->getMaskPosition()[1] );
  //m_TextActorHeading->Modified();


  m_TextActorStatus->SetInput(monitor->getStatus().toStdString().c_str());
  vtkTextProperty* tpropStatus = m_TextActorStatus->GetTextProperty();
  tpropStatus->SetFontSize(10);
  tpropStatus->SetOpacity( monitor->getStatusOpacity() * 0.1 );
  tpropStatus->SetColor(0.85,0.8,0.8);
  m_TextActorStatus->SetDisplayPosition( monitor->getStatusPosition()[0], monitor->getStatusPosition()[1] );
  //m_TextActorStatus->Modified();

  m_TextActorStarted->SetInput(QString::number(monitor->getStarted()).toStdString().c_str());
  vtkTextProperty* tpropStarted = m_TextActorStarted->GetTextProperty();
  tpropStarted->SetFontSize(12);
  tpropStarted->SetOpacity( monitor->getStartedOpacity() * 0.1 );
  tpropStarted->SetColor(0.0,1.0,0.0);
  m_TextActorStarted->SetDisplayPosition( monitor->getStartedPosition()[0], monitor->getStartedPosition()[1] );
  //m_TextActorStarted->Modified();

  m_TextActorFinished->SetInput(QString::number(monitor->getFinished()).toStdString().c_str());
  vtkTextProperty* tpropFinished = m_TextActorFinished->GetTextProperty();
  tpropFinished->SetFontSize(12);
  tpropFinished->SetOpacity( monitor->getFinishedOpacity() * 0.1 );
  tpropFinished->SetColor(1.0,1.0,1.0);
  m_TextActorFinished->SetDisplayPosition( monitor->getFinishedPosition()[0], monitor->getFinishedPosition()[1] );
  //m_TextActorFinished->Modified();

  m_TextActorTerminated->SetInput(QString::number(monitor->getTerminated()).toStdString().c_str());
  vtkTextProperty* tpropTerminated = m_TextActorTerminated->GetTextProperty();
  tpropTerminated->SetFontSize(12);
  tpropTerminated->SetOpacity( monitor->getTerminatedOpacity() * 0.1 );
  tpropTerminated->SetColor(1.0,1.0,1.0);
  m_TextActorTerminated->SetDisplayPosition( monitor->getTerminatedPosition()[0], monitor->getTerminatedPosition()[1] );
  //m_TextActorTerminated->Modified();


  // Calculate time step of the input data for the specified renderer (integer value)
  // this method is implemented in mitkMapper
//  this->CalculateTimeStep( renderer );


}


void mitk::FiberBundleThreadMonitorMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{

//  MITK_INFO << "FiberBundlexXXMapper3D()SetDefaultProperties";


  Superclass::SetDefaultProperties(node, renderer, overwrite);



}

vtkProp* mitk::FiberBundleThreadMonitorMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  //MITK_INFO << "FiberBundlexXXMapper3D()GetVTKProp";
  //this->GenerateData();
  return m_FiberAssembly;

}

void mitk::FiberBundleThreadMonitorMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
//  MITK_INFO << "FiberBundleXXXMapper3D ApplyProperties(renderer)";
}

void mitk::FiberBundleThreadMonitorMapper3D::UpdateVtkObjects()
{
//  MITK_INFO << "FiberBundlexxXMapper3D UpdateVtkObjects()";


}
