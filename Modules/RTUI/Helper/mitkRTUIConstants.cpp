/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRTUIConstants.h"

const std::string mitk::RTUIConstants::ROOT_PREFERENCE_NODE_ID = "/RT/UI";
const std::string mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_AS_DEFAULT_ID = "unkownDoseHandlingStyle";
const std::string mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID = "unkownDoseHandlingValue";


const std::string mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID = "/RT/UI/doseVis";
const std::string mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID = "/RT/UI/doseVis/isoPresets";
const std::string mitk::RTUIConstants::REFERENCE_DOSE_ID = "referenceDose";
const std::string mitk::RTUIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID = "globalReferenceDoseSync";
const std::string mitk::RTUIConstants::DOSE_DISPLAY_ABSOLUTE_ID = "doseDisplayAbsolute";
const std::string mitk::RTUIConstants::GLOBAL_VISIBILITY_ISOLINES_ID = "globalVisibilityIsoLines";
const std::string mitk::RTUIConstants::GLOBAL_VISIBILITY_COLORWASH_ID = "globalVisibilityColorWash";
const std::string mitk::RTUIConstants::SELECTED_ISO_PRESET_ID = "selectedIsoPreset";

const std::string mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID = "doseValue";
const std::string mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID = "color_R";
const std::string mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID = "color_G";
const std::string mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID = "color_B";
const std::string mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID = "visibleIsoLines";
const std::string mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID = "visibleColorWash";

const mitk::DoseValueAbs mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE = 40.0;

const std::string mitk::RTCTKEventConstants::TOPIC_ISO_DOSE_LEVEL_PRESETS = "org/mitk/rt/dosevisualization/presets";
const std::string mitk::RTCTKEventConstants::TOPIC_ISO_DOSE_LEVEL_PRESETS_CHANGED = "org/mitk/rt/dosevisualization/presets/CHANGED";
const std::string mitk::RTCTKEventConstants::TOPIC_REFERENCE_DOSE = "org/mitk/rt/dosevisualization/referenceDose";
const std::string mitk::RTCTKEventConstants::TOPIC_REFERENCE_DOSE_CHANGED = "org/mitk/rt/dosevisualization/referenceDose/CHANGED";
const std::string mitk::RTCTKEventConstants::TOPIC_GLOBAL_VISIBILITY_CHANGED = "org/mitk/rt/dosevisualization/globalVis/CHANGED";
