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

#include <mitkVMTKTestModuleActivator.h>
#include <vtkSmartPointer.h>
#include <vtkvmtkPolyBall.h>

mitk::VMTKTestModuleActivator::VMTKTestModuleActivator()
{
  auto polyBall = vtkSmartPointer<vtkvmtkPolyBall>::New();
}

mitk::VMTKTestModuleActivator::~VMTKTestModuleActivator()
{
}

void mitk::VMTKTestModuleActivator::Load(us::ModuleContext*)
{
}

void mitk::VMTKTestModuleActivator::Unload(us::ModuleContext*)
{
}
