/*=========================================================================
 
 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
 Version:   $Revision: 17179 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/



#include "mitkFiberBundleXThreadMonitorMapper3D.h"
#include <mitkProperties.h>
#include <vtkPropAssembly.h>
//#include <vtkTextActor.h>
#include <vtkTextProperty.h>




mitk::FiberBundleXThreadMonitorMapper3D::FiberBundleXThreadMonitorMapper3D()
: m_FiberMonitorMapper(vtkSmartPointer<vtkPolyDataMapper>::New())
, m_TextActorClose(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorOpen(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorHeading(vtkSmartPointer<vtkTextActor>::New())
, m_TextActorMask(vtkSmartPointer<vtkTextActor>::New())
, m_FiberAssembly(vtkPropAssembly::New())
{
  m_FiberAssembly->AddPart(m_TextActorClose);
  m_FiberAssembly->AddPart(m_TextActorOpen);
  m_FiberAssembly->AddPart(m_TextActorHeading);
  m_FiberAssembly->AddPart(m_TextActorMask);
}


mitk::FiberBundleXThreadMonitorMapper3D::~FiberBundleXThreadMonitorMapper3D()
{
  m_FiberAssembly->Delete();
}


const mitk::FiberBundleXThreadMonitor* mitk::FiberBundleXThreadMonitorMapper3D::GetInput()
{
  return static_cast<const mitk::FiberBundleXThreadMonitor * > ( GetData() );
}


/* 
 This method is called once the mapper gets new input, 
 for UI rotation or changes in colorcoding this method is NOT called 
 */
void mitk::FiberBundleXThreadMonitorMapper3D::GenerateData()
{
  FiberBundleXThreadMonitor* monitor = dynamic_cast<FiberBundleXThreadMonitor * > ( GetData() );



  monitor->getBracketClosePosition();
  
 
  
//	m_TextActor->SetInput( monitor->getTextL1().toStdString().c_str() );
  m_TextActorClose->SetInput( monitor->getBracketClose().toStdString().c_str() );
  vtkTextProperty* tpropClose = m_TextActorClose->GetTextProperty();
  //tprop->SetFontFamilyToArial ();
  //tprop->SetLineSpacing(1.0);
  tpropClose->SetFontSize(18);
  tpropClose->SetColor(255.0,255.0,255.0);
  m_TextActorClose->SetDisplayPosition( monitor->getBracketClosePosition()[0], monitor->getBracketClosePosition()[1] );
  m_TextActorClose->Modified();
  
  
  m_TextActorOpen->SetInput( monitor->getBracketOpen().toStdString().c_str() );
  vtkTextProperty* tpropOpen = m_TextActorOpen->GetTextProperty();
  //tprop->SetFontFamilyToArial ();
  //tprop->SetLineSpacing(1.0);
  tpropOpen->SetFontSize(18);
  tpropOpen->SetColor(255.0,255.0,255.0);
  m_TextActorOpen->SetDisplayPosition( monitor->getBracketOpenPosition()[0], monitor->getBracketOpenPosition()[1] );
  m_TextActorOpen->Modified();
  
  

  m_TextActorHeading->SetInput(  monitor->getHeading().toStdString().c_str() );
  vtkTextProperty* tpropHeading = m_TextActorHeading->GetTextProperty();
  tpropHeading->SetFontSize(12);
  tpropHeading->SetOpacity( monitor->getHeadingOpacity() * 0.1 );
  tpropHeading->SetColor(255.0,255.0,255.0);
  m_TextActorHeading->SetDisplayPosition( monitor->getHeadingPosition()[0], monitor->getHeadingPosition()[1] );
  m_TextActorHeading->Modified();
  
  
  m_TextActorMask->SetInput(  monitor->getMask().toStdString().c_str() );
  vtkTextProperty* tpropMask = m_TextActorMask->GetTextProperty();
  tpropMask->SetFontSize(12);
  tpropMask->SetOpacity( monitor->getMaskOpacity() * 0.1 );
  tpropMask->SetColor(255.0,255.0,255.0);
  m_TextActorMask->SetDisplayPosition( monitor->getMaskPosition()[0], monitor->getMaskPosition()[1] );
  m_TextActorHeading->Modified();
  
  

}



void mitk::FiberBundleXThreadMonitorMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  //MITK_INFO << "FiberBundleXxXXMapper3D()DataForRenderer";
  //ToDo do update checks
  this->GenerateData();
}


void mitk::FiberBundleXThreadMonitorMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  
//  MITK_INFO << "FiberBundleXxXXMapper3D()SetDefaultProperties";
  
  
  Superclass::SetDefaultProperties(node, renderer, overwrite);
  
  
  
}

vtkProp* mitk::FiberBundleXThreadMonitorMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  //MITK_INFO << "FiberBundleXxXXMapper3D()GetVTKProp";
  //this->GenerateData();
  return m_FiberAssembly;
  
}

void mitk::FiberBundleXThreadMonitorMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
//  MITK_INFO << "FiberBundleXXXXMapper3D ApplyProperties(renderer)";
}

void mitk::FiberBundleXThreadMonitorMapper3D::UpdateVtkObjects()
{
//  MITK_INFO << "FiberBundleXxxXMapper3D UpdateVtkObjects()";
  
  
}

void mitk::FiberBundleXThreadMonitorMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{
  
  
  
}



