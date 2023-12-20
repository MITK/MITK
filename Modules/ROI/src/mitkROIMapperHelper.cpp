/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkROIMapperHelper.h"

#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <boost/algorithm/string.hpp>

#include <regex>

void mitk::ROIMapperHelper::ApplyIndividualProperties(const ROI::Element& roi, TimeStepType t, vtkActor* actor)
{
  auto* property = actor->GetProperty();

  property->SetRepresentationToWireframe();
  property->LightingOff();

  if (auto colorProperty = GetConstProperty<ColorProperty>("color", roi, t); colorProperty != nullptr)
  {
    const auto color = colorProperty->GetColor();
    property->SetColor(color[0], color[1], color[2]);
  }

  if (auto opacityProperty = GetConstProperty<FloatProperty>("opacity", roi, t); opacityProperty != nullptr)
  {
    const auto opacity = opacityProperty->GetValue();
    property->SetOpacity(property->GetOpacity() * opacity);
  }

  if (auto lineWidthProperty = GetConstProperty<FloatProperty>("lineWidth", roi, t); lineWidthProperty != nullptr)
  {
    const auto lineWidth = lineWidthProperty->GetValue();
    property->SetLineWidth(lineWidth);
  }
}

vtkSmartPointer<vtkCaptionActor2D> mitk::ROIMapperHelper::CreateCaptionActor(const std::string& caption, const Point3D& attachmentPoint, vtkProperty* property, const DataNode* dataNode, const BaseRenderer* renderer)
{
  auto captionActor = vtkSmartPointer<vtkCaptionActor2D>::New();
  captionActor->SetPosition(property->GetLineWidth() * 0.5, property->GetLineWidth() * 0.5);
  captionActor->GetTextActor()->SetTextScaleModeToNone();
  captionActor->SetAttachmentPoint(attachmentPoint[0], attachmentPoint[1], attachmentPoint[2]);
  captionActor->SetCaption(caption.c_str());
  captionActor->BorderOff();
  captionActor->LeaderOff();

  auto* textProperty = captionActor->GetCaptionTextProperty();
  textProperty->SetColor(property->GetColor());
  textProperty->SetOpacity(property->GetOpacity());
  textProperty->ShadowOff();

  int fontSize = 16;
  dataNode->GetIntProperty("font.size", fontSize, renderer);
  textProperty->SetFontSize(fontSize);

  bool bold = false;
  dataNode->GetBoolProperty("font.bold", bold, renderer);
  textProperty->SetBold(bold);

  bool italic = false;
  dataNode->GetBoolProperty("font.italic", italic, renderer);
  textProperty->SetItalic(italic);

  return captionActor;
}

std::string mitk::ROIMapperHelper::ParseCaption(const std::string& captionTemplate, const ROI::Element& roi, TimeStepType t)
{
  std::regex regex(R"(\{([^}]*)\})"); // Anything between curly braces (considered as placeholder).

  auto start = captionTemplate.cbegin();
  bool hasPlaceholders = false;
  std::string caption;
  std::smatch match;

  // Iterate through the caption template and substitute all
  // placeholders with corresponding data or property values.

  while (std::regex_search(start, captionTemplate.cend(), match, regex))
  {
    hasPlaceholders = true;

    caption.append(match.prefix().first, match.prefix().second);

    if (match[1] == "ID")
    {
      caption.append(std::to_string(roi.GetID()));
    }
    else
    {
      auto property = roi.GetConstProperty(match[1], t);

      if (property.IsNotNull())
        caption.append(property->GetValueAsString());
    }

    start = match.suffix().first;
  }

  if (match.suffix().matched)
    caption.append(match.suffix().first, match.suffix().second);

  if (hasPlaceholders)
  {
    boost::trim(caption);
    return caption;
  }

  return captionTemplate;
}

void mitk::ROIMapperHelper::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override)
{
  node->AddProperty("opacity", FloatProperty::New(1.0f), renderer, override);
  node->AddProperty("font.bold", BoolProperty::New(false), renderer, override);
  node->AddProperty("font.italic", BoolProperty::New(false), renderer, override);
  node->AddProperty("font.size", IntProperty::New(16), renderer, override);
  node->AddProperty("caption", StringProperty::New("{ID}\n{name}"), renderer, override);
}
