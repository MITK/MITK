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

#include "mitkColorBarOverlay.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include <vtkScalarBarActor.h>

mitk::ColorBarOverlay::ColorBarOverlay()
{
  SetDrawAnnotations(true);

  SetDrawTickLabels(true);

  SetOrientationToVertical();

  SetMaxNumberOfColors(100);
  SetNumberOfLabels(4);

  SetAnnotationTextScaling(false);

  SetLookupTable(NULL);
}


mitk::ColorBarOverlay::~ColorBarOverlay()
{
}

mitk::ColorBarOverlay::LocalStorage::~LocalStorage()
{
}

mitk::ColorBarOverlay::LocalStorage::LocalStorage()
{
  m_ScalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
}

void mitk::ColorBarOverlay::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);

  if(ls->IsGenerateDataRequired(renderer,this))
  {
    ls->m_ScalarBarActor->SetDrawAnnotations(this->GetDrawAnnotations());
    ls->m_ScalarBarActor->SetLookupTable(this->GetLookupTable());
    ls->m_ScalarBarActor->SetOrientation(this->GetOrientation());
    ls->m_ScalarBarActor->SetDrawTickLabels(this->GetDrawTickLabels());
    ls->m_ScalarBarActor->SetMaximumNumberOfColors(this->GetMaxNumberOfColors());
    ls->m_ScalarBarActor->SetNumberOfLabels(this->GetNumberOfLabels());
    ls->m_ScalarBarActor->SetAnnotationTextScaling(this->GetAnnotationTextScaling());
    //manually set position so there is no overlap with mitk logo in 3d renderwindow
    if (this->GetOrientation()==1)
    {
      ls->m_ScalarBarActor->SetPosition(0.80,0.15);
      ls->m_ScalarBarActor->SetWidth(0.15);
      ls->m_ScalarBarActor->SetHeight(0.85);
    }else
    {
      ls->m_ScalarBarActor->SetPosition(0.03,0.03);
      ls->m_ScalarBarActor->SetWidth(0.8);
      ls->m_ScalarBarActor->SetHeight(0.15);
    }
  }

}

vtkProp* mitk::ColorBarOverlay::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_ScalarBarActor;
}

void mitk::ColorBarOverlay::SetDrawAnnotations(bool annotations)
{
    SetBoolProperty("ColorBarOverlay.DrawAnnotations", annotations);
}

bool mitk::ColorBarOverlay::GetDrawAnnotations() const
{
  bool annotations;
  GetPropertyList()->GetBoolProperty("ColorBarOverlay.DrawAnnotations", annotations);
  return annotations;
}

void mitk::ColorBarOverlay::SetLookupTable(vtkSmartPointer<vtkLookupTable> table)
{
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);
    lut->SetVtkLookupTable(table);
    prop->SetLookupTable(lut);
    SetProperty("ColorBarOverlay.LookupTable", prop.GetPointer());
}

vtkSmartPointer<vtkLookupTable> mitk::ColorBarOverlay::GetLookupTable() const
{
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  lut = dynamic_cast<mitk::LookupTableProperty*>(GetPropertyList()->GetProperty("ColorBarOverlay.LookupTable"))->GetLookupTable();
  return lut->GetVtkLookupTable();
}

void mitk::ColorBarOverlay::SetOrientation(int orientation)
{
    SetIntProperty("ColorBarOverlay.Orientation", orientation);
}

int mitk::ColorBarOverlay::GetOrientation() const
{
  int orientation;
  GetPropertyList()->GetIntProperty("ColorBarOverlay.Orientation", orientation);
  return orientation;
}

void mitk::ColorBarOverlay::SetDrawTickLabels(bool ticks)
{
    SetBoolProperty("ColorBarOverlay.DrawTicks", ticks);
}

bool mitk::ColorBarOverlay::GetDrawTickLabels() const
{
  bool ticks;
  GetPropertyList()->GetBoolProperty("ColorBarOverlay.DrawTicks", ticks);
  return ticks;
}

void mitk::ColorBarOverlay::SetOrientationToHorizontal()
{
  SetOrientation(0);
}

void mitk::ColorBarOverlay::SetOrientationToVertical()
{
  SetOrientation(1);
}

void mitk::ColorBarOverlay::SetMaxNumberOfColors(int numberOfColors)
{
    SetIntProperty("ColorBarOverlay.MaximumNumberOfColors", numberOfColors);
}

int mitk::ColorBarOverlay::GetMaxNumberOfColors() const
{
  int numberOfColors;
  GetPropertyList()->GetIntProperty("ColorBarOverlay.MaximumNumberOfColors", numberOfColors);
  return numberOfColors;
}

void mitk::ColorBarOverlay::SetNumberOfLabels(int numberOfLabels)
{
    SetIntProperty("ColorBarOverlay.NumberOfLabels", numberOfLabels);
}

int mitk::ColorBarOverlay::GetNumberOfLabels() const
{
  int numberOfLabels;
  GetPropertyList()->GetIntProperty("ColorBarOverlay.NumberOfLabels", numberOfLabels);
  return numberOfLabels;
}

void mitk::ColorBarOverlay::SetAnnotationTextScaling(bool scale)
{
    SetBoolProperty("ColorBarOverlay.ScaleAnnotationText", scale);
}

bool mitk::ColorBarOverlay::GetAnnotationTextScaling() const
{
  bool scale;
  GetPropertyList()->GetBoolProperty("ColorBarOverlay.ScaleAnnotationText", scale);
  return scale;
}
