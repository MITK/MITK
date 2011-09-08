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
:m_FiberMonitorMapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
 m_TextActor(vtkSmartPointer<vtkTextActor>::New()),
 m_FiberAssembly(vtkPropAssembly::New())
{
m_FiberAssembly->AddPart(m_TextActor);
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

	m_TextActor->SetInput("FIBER MONITOR\n[activated]");
  vtkTextProperty* tprop = m_TextActor->GetTextProperty();
  tprop->SetFontFamilyToArial ();
  tprop->SetLineSpacing(1.0);
  tprop->SetFontSize(20);
  tprop->SetColor(1.0,0.0,0.0);
  m_TextActor->SetDisplayPosition( 20, 20 );
  m_TextActor->Modified();
  
  
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



