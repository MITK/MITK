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
  // XML Tags
  static const std::string xmlTagConfigRoot(); // = "config";
  static const std::string xmlTagParam(); // = "param";
  static const std::string xmlTagEventVariant(); // = "event_variant";
  static const std::string xmlTagAttribute(); // = "attribute";

  // XML Param
  static const std::string xmlParameterName(); // = "name";
  static const std::string xmlParameterValue(); // = "value";
  static const std::string xmlParameterEventVariant(); // = "event_variant";
  static const std::string xmlParameterEventClass(); // = "class";

  // Event Description
  static const std::string xmlEventPropertyModifier(); // = "Modifiers";
  static const std::string xmlEventPropertyEventButton(); // = "EventButton";
  static const std::string xmlEventPropertyButtonState(); // = "ButtonState";
  static const std::string xmlEventPropertyKey(); // = "Key";
  static const std::string xmlEventPropertyScrollDirection(); // = "ScrollDirection";
  static const std::string xmlEventPropertySignalName(); // = "SignalName";
};

}     //namespace mitk
#endif //ifndef MITKINTERACTEVENTCONST_H
