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

#include "mitkInteractionEventConst.h"

namespace mitk {

// XML Tags
const std::string InteractionEventConst::xmlTagConfigRoot = "config";
const std::string InteractionEventConst::xmlTagParam = "param";
const std::string InteractionEventConst::xmlTagEventVariant = "event_variant";
const std::string InteractionEventConst::xmlTagAttribute = "attribute";

// XML Param
const std::string InteractionEventConst::xmlParameterName = "name";
const std::string InteractionEventConst::xmlParameterValue = "value";
const std::string InteractionEventConst::xmlParameterEventVariant = "event_variant";
const std::string InteractionEventConst::xmlParameterEventClass = "class";

// Event Description
const std::string InteractionEventConst::xmlEventPropertyModifier = "Modifiers";
const std::string InteractionEventConst::xmlEventPropertyEventButton = "EventButton";
const std::string InteractionEventConst::xmlEventPropertyButtonState = "ButtonState";
const std::string InteractionEventConst::xmlEventPropertyKey = "Key";
const std::string InteractionEventConst::xmlEventPropertyScrollDirection = "ScrollDirection";
const std::string InteractionEventConst::xmlEventPropertySignalName = "SignalName";

}
