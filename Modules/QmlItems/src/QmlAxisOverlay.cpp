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


#include "QmlAxisOverlay.h"
#include <vtkProperty.h>
#include <vtkActor2D.h>
#include <vtkPropCollection.h>
//#include "Util_Debug.h"
#include "mitkPoint.h"

mitk::AxisOverlay::AxisOverlay()
{
}

mitk::AxisOverlay::~AxisOverlay()
{
}

mitk::AxisOverlay::LocalStorage::~LocalStorage()
{
}

mitk::AxisOverlay::LocalStorage::LocalStorage()
{
  this->m_axesActor = vtkSmartPointer<vtkAxesActor>::New();
  this->m_widget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->m_dummyActor = vtkSmartPointer<vtkActor>::New();
  this->m_dummyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->m_dummySource = vtkSmartPointer<vtkSphereSource>::New();
  this->m_dummyMapper->SetInputConnection(m_dummySource->GetOutputPort());
  this->m_dummyActor->SetMapper(this->m_dummyMapper);
  this->m_dummyActor->SetVisibility(false);
  this->m_dummyActor->GetProperty()->SetOpacity(0.0);
}

void mitk::AxisOverlay::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  if (ls->IsGenerateDataRequired(renderer, this))
  {
    ls->m_widget->SetOrientationMarker(ls->m_axesActor);
    ls->m_widget->SetInteractor(renderer->GetRenderWindow()->GetInteractor());
    ls->m_widget->SetEnabled(true);
    ls->m_widget->SetOutlineColor(0.3, 0, 0);
    ls->m_widget->SetInteractive(true);
    ls->m_initialized = true;
  }
}

vtkProp* mitk::AxisOverlay::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_dummyActor;
}
