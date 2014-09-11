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

#ifndef MITKINTERACTEVENTCONST_H
#define MITKINTERACTEVENTCONST_H

#include <string>
#include <MitkCoreExports.h>

namespace mitk
{

/**
 * @brief Constants to describe Mouse Events and special Key Events.
 */
struct MITK_CORE_EXPORT InteractionEventConst
{
  static const std::string xmlHead(); // = "<?xml version='1.0'?>";

  // XML Tags
  static const std::string xmlTagConfigRoot(); // = "config";
  static const std::string xmlTagEvents(); // = "events";
  static const std::string xmlTagInteractions(); // = "interactions";
  static const std::string xmlTagParam(); // = "param";
  static const std::string xmlTagEventVariant(); // = "event_variant";
  static const std::string xmlTagAttribute(); // = "attribute";
  static const std::string xmlTagRenderer(); // = "renderer";

  // XML Param
  static const std::string xmlParameterName(); // = "name";
  static const std::string xmlParameterValue(); // = "value";
  static const std::string xmlParameterEventVariant(); // = "event_variant";
  static const std::string xmlParameterEventClass(); // = "class";

  // Event Description
  static const std::string xmlEventPropertyModifier(); // = "Modifiers";
  static const std::string xmlEventPropertyEventButton(); // = "EventButton";
  static const std::string xmlEventPropertyButtonState(); // = "ButtonState";
  static const std::string xmlEventPropertyPositionInWorld(); // = "PositionInWorld";
  static const std::string xmlEventPropertyPositionOnScreen(); // = "PositionOnScreen";
  static const std::string xmlEventPropertyKey(); // = "Key";
  static const std::string xmlEventPropertyScrollDirection(); // = "ScrollDirection";
  static const std::string xmlEventPropertyWheelDelta(); // = "WheelDelta";
  static const std::string xmlEventPropertySignalName(); // = "SignalName";
  static const std::string xmlEventPropertyRendererName(); // = "RendererName";
  static const std::string xmlEventPropertyViewDirection(); // = "ViewDirection";
  static const std::string xmlEventPropertyMapperID(); // = "MapperID";

  static const std::string xmlRenderSizeX(); // = "RenderSizeX";
  static const std::string xmlRenderSizeY(); // = "RenderSizeY";
  static const std::string xmlRenderSizeZ(); // = "RenderSizeZ";
};

}     //namespace mitk
#endif //ifndef MITKINTERACTEVENTCONST_H
