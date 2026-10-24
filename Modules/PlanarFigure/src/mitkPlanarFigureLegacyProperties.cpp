/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureLegacyProperties.h"

#include <mitkColorProperty.h>
#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkIPropertyMigration.h>
#include <mitkPlanarFigure.h>
#include <mitkProperties.h>
#include <mitkPropertyList.h>

#include <string>
#include <vector>

namespace
{
  // Previous versions wrote every default into their scenes. Values equal to
  // a previous default are therefore dropped instead of overriding the
  // current default.

  // Scene readers replace the defaults of a node by the list they read, and
  // lists of previous versions lack the properties added since. They get the
  // current defaults here, as new figures do, so that their settings exist and
  // can be changed.
  void AddMissingDefaults(const mitk::BaseData *data, mitk::PropertyList &properties)
  {
    const auto *figure = dynamic_cast<const mitk::PlanarFigure *>(data);

    if (nullptr == figure)
      return;

    // Setting the data applies the defaults of the mappers.
    auto defaults = mitk::DataNode::New();
    defaults->SetData(mitk::CreatePlanarFigure(figure->GetTypeId()));

    for (const auto &[key, property] : *defaults->GetPropertyList()->GetMap())
    {
      if (nullptr == properties.GetProperty(key))
        properties.SetProperty(key, property->Clone());
    }
  }

  void MigrateLineWidth(const mitk::BaseData *data, const mitk::BaseProperty &legacyProperty, mitk::PropertyList &properties)
  {
    const auto *lineWidth = dynamic_cast<const mitk::FloatProperty *>(&legacyProperty);

    if (nullptr != lineWidth && 2.0f != lineWidth->GetValue())
      properties.SetProperty("line width", mitk::FloatProperty::New(lineWidth->GetValue()));

    // Only lists of previous versions have this property, so the defaults they
    // wrote for properties that are still in use are dropped here, where they
    // cannot be mistaken for a choice made in a current scene. Their labels
    // were on by default, as they are now.
    float opacity = 1.0f;

    if (properties.GetFloatProperty("opacity", opacity) && 0.8f == opacity)
      properties.DeleteProperty("opacity");

    AddMissingDefaults(data, properties);
  }

  // Previous versions never wrote this property as a default. When it was set,
  // the figure was drawn in it instead of in its color.
  void MigrateLineColor(const mitk::BaseData *, const mitk::BaseProperty &legacyProperty, mitk::PropertyList &properties)
  {
    if (nullptr != dynamic_cast<const mitk::ColorProperty *>(&legacyProperty))
      properties.SetProperty("color", legacyProperty.Clone());
  }

  void MigrateFontSize(const mitk::BaseData *, const mitk::BaseProperty &legacyProperty, mitk::PropertyList &properties)
  {
    const auto *fontSize = dynamic_cast<const mitk::IntProperty *>(&legacyProperty);

    if (nullptr != fontSize && 12 != fontSize->GetValue())
      properties.SetProperty("font.size", mitk::IntProperty::New(fontSize->GetValue()));
  }

  std::vector<std::string> GetObsoleteKeys()
  {
    std::vector<std::string> keys = {
      "planarfigure.annotations.font.bold",
      "planarfigure.annotations.font.family",
      "planarfigure.annotations.font.italic",
      "planarfigure.annotations.shadow",
      "planarfigure.controlpointshape",
      "planarfigure.devicepixelratio",
      "planarfigure.drawcontrolpoints",
      "planarfigure.drawdashed",
      "planarfigure.drawoutline",
      "planarfigure.drawshadow",
      "planarfigure.fill",
      "planarfigure.helperline.drawdashed",
      "planarfigure.helperline.width",
      "planarfigure.hidecontrolpointsduringinteraction",
      "planarfigure.isdeletable",
      "planarfigure.iseditable",
      "planarfigure.isextendable",
      "planarfigure.ishovering",
      "planarfigure.outline.width",
      "planarfigure.shadow.widthmodifier",
      "ClosedPlanarPolygon",
      "closed",
      "initiallyplaced",
      "subdivision",
      "SingleLineMode"
    };

    for (const std::string state : { "default", "hover", "selected" })
    {
      keys.push_back("planarfigure." + state + ".annotation.color");

      for (const std::string element : { "line", "outline", "helperline", "markerline", "marker" })
      {
        keys.push_back("planarfigure." + state + "." + element + ".color");
        keys.push_back("planarfigure." + state + "." + element + ".opacity");
      }
    }

    return keys;
  }
}

void mitk::PlanarFigureLegacyProperties::Register()
{
  CoreServicePointer<IPropertyMigration> migration(CoreServices::GetPropertyMigration());

  migration->AddMigration<PlanarFigure>("planarfigure.line.width", MigrateLineWidth);
  migration->AddMigration<PlanarFigure>("planarfigure.annotations.font.size", MigrateFontSize);

  // Registered before the removal of the other colors, since the first rule for a key applies.
  migration->AddMigration<PlanarFigure>("planarfigure.default.line.color", MigrateLineColor);

  for (const auto &key : GetObsoleteKeys())
    migration->AddRemoval<PlanarFigure>(key);
}
