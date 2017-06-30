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

#include "mitkColorBarAnnotation.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include <vtkScalarBarActor.h>

mitk::ColorBarAnnotation::ColorBarAnnotation()
{
  SetDrawAnnotations(true);

  SetDrawTickLabels(true);

  SetOrientationToVertical();

  SetMaxNumberOfColors(100);
  SetNumberOfLabels(4);

  SetAnnotationTextScaling(false);

  SetLookupTable(nullptr);
}

mitk::ColorBarAnnotation::~ColorBarAnnotation()
{
  for (BaseRenderer *renderer : m_LSH.GetRegisteredBaseRenderer())
  {
    if (renderer)
    {
      this->RemoveFromBaseRenderer(renderer);
    }
  }
}

mitk::ColorBarAnnotation::LocalStorage::~LocalStorage()
{
}

mitk::ColorBarAnnotation::LocalStorage::LocalStorage()
{
  m_ScalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
}

void mitk::ColorBarAnnotation::UpdateVtkAnnotation(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  if (ls->IsGenerateDataRequired(renderer, this))
  {
    ls->m_ScalarBarActor->SetDrawAnnotations(this->GetDrawAnnotations());
    ls->m_ScalarBarActor->SetLookupTable(this->GetLookupTable());
    ls->m_ScalarBarActor->SetOrientation(this->GetOrientation());
    ls->m_ScalarBarActor->SetDrawTickLabels(this->GetDrawTickLabels());
    ls->m_ScalarBarActor->SetMaximumNumberOfColors(this->GetMaxNumberOfColors());
    ls->m_ScalarBarActor->SetNumberOfLabels(this->GetNumberOfLabels());
    ls->m_ScalarBarActor->SetAnnotationTextScaling(this->GetAnnotationTextScaling());
    // manually set position so there is no overlap with mitk logo in 3d renderwindow
    if (this->GetOrientation() == 1)
    {
      ls->m_ScalarBarActor->SetPosition(0.80, 0.15);
      ls->m_ScalarBarActor->SetWidth(0.15);
      ls->m_ScalarBarActor->SetHeight(0.85);
    }
    else
    {
      ls->m_ScalarBarActor->SetPosition(0.03, 0.03);
      ls->m_ScalarBarActor->SetWidth(0.8);
      ls->m_ScalarBarActor->SetHeight(0.15);
    }
  }
}

vtkProp *mitk::ColorBarAnnotation::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_ScalarBarActor;
}

void mitk::ColorBarAnnotation::SetDrawAnnotations(bool annotations)
{
  SetBoolProperty("ColorBarAnnotation.DrawAnnotations", annotations);
}

bool mitk::ColorBarAnnotation::GetDrawAnnotations() const
{
  bool annotations;
  GetPropertyList()->GetBoolProperty("ColorBarAnnotation.DrawAnnotations", annotations);
  return annotations;
}

void mitk::ColorBarAnnotation::SetLookupTable(vtkSmartPointer<vtkLookupTable> table)
{
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);
  lut->SetVtkLookupTable(table);
  prop->SetLookupTable(lut);
  SetProperty("ColorBarAnnotation.LookupTable", prop.GetPointer());
}

vtkSmartPointer<vtkLookupTable> mitk::ColorBarAnnotation::GetLookupTable() const
{
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
  lut = dynamic_cast<mitk::LookupTableProperty *>(GetPropertyList()->GetProperty("ColorBarAnnotation.LookupTable"))
          ->GetLookupTable();
  return lut->GetVtkLookupTable();
}

void mitk::ColorBarAnnotation::SetOrientation(int orientation)
{
  SetIntProperty("ColorBarAnnotation.Orientation", orientation);
}

int mitk::ColorBarAnnotation::GetOrientation() const
{
  int orientation;
  GetPropertyList()->GetIntProperty("ColorBarAnnotation.Orientation", orientation);
  return orientation;
}

void mitk::ColorBarAnnotation::SetDrawTickLabels(bool ticks)
{
  SetBoolProperty("ColorBarAnnotation.DrawTicks", ticks);
}

bool mitk::ColorBarAnnotation::GetDrawTickLabels() const
{
  bool ticks;
  GetPropertyList()->GetBoolProperty("ColorBarAnnotation.DrawTicks", ticks);
  return ticks;
}

void mitk::ColorBarAnnotation::SetOrientationToHorizontal()
{
  SetOrientation(0);
}

void mitk::ColorBarAnnotation::SetOrientationToVertical()
{
  SetOrientation(1);
}

void mitk::ColorBarAnnotation::SetMaxNumberOfColors(int numberOfColors)
{
  SetIntProperty("ColorBarAnnotation.MaximumNumberOfColors", numberOfColors);
}

int mitk::ColorBarAnnotation::GetMaxNumberOfColors() const
{
  int numberOfColors;
  GetPropertyList()->GetIntProperty("ColorBarAnnotation.MaximumNumberOfColors", numberOfColors);
  return numberOfColors;
}

void mitk::ColorBarAnnotation::SetNumberOfLabels(int numberOfLabels)
{
  SetIntProperty("ColorBarAnnotation.NumberOfLabels", numberOfLabels);
}

int mitk::ColorBarAnnotation::GetNumberOfLabels() const
{
  int numberOfLabels;
  GetPropertyList()->GetIntProperty("ColorBarAnnotation.NumberOfLabels", numberOfLabels);
  return numberOfLabels;
}

void mitk::ColorBarAnnotation::SetAnnotationTextScaling(bool scale)
{
  SetBoolProperty("ColorBarAnnotation.ScaleAnnotationText", scale);
}

bool mitk::ColorBarAnnotation::GetAnnotationTextScaling() const
{
  bool scale;
  GetPropertyList()->GetBoolProperty("ColorBarAnnotation.ScaleAnnotationText", scale);
  return scale;
}
