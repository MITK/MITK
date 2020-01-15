/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __DOSE_VIS_PREFERENCE_HELPER_H
#define __DOSE_VIS_PREFERENCE_HELPER_H

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include "mitkIsoDoseLevelCollections.h"

class ctkPluginContext;

namespace mitk
{
    typedef std::map<std::string, mitk::IsoDoseLevelSet::Pointer> PresetMapType;

    /** Stores the given preset map in the application preferences. */
    void StorePresetsMap(const PresetMapType& presetMap);

    /** Loads all defined iso dose level presets from the preferences and stores them
    in a map. Key is the name of the preset.
    @remark If no presets are stored in the preferences this function will generate default presets
    (using GenerateDefaultPresetsMap()) and the stores it in the preferences (to guarantee a consistent state)
    before passing the default presets back.*/
    PresetMapType LoadPresetsMap();

    /** Generate the default presets map.*/
    PresetMapType GenerateDefaultPresetsMap();

    /**Retrieves the name of the preset, that is currently selected for the application, from the preferences.*/
    std::string GetSelectedPresetName();

    /** checks if the passed name exists in the preset map. If not an exception is thrown. If it exists, the new
    * value will be set.*/
    void SetSelectedPresetName(const std::string& presetName);

    /** retrieves the reference dose from the preferences and indicates if global sync for reference dose is activated
    or not.
    @param value The reference dose value stored in the preferences.
    @return Indicator for global sync. True: global sync activated -> preference value is relevant.
    False: global sync deactivated. Value is irrelevant. Each node has its own value.*/
    bool GetReferenceDoseValue(DoseValueAbs& value);
    /** Sets the global sync setting and dose reference value in the preferences.
    * @param globalSync Indicator if global sync should be set active (true) or inactive (false). If it is true, the value is irrelevant (but will be set).
    * @param value The dose reference value that should be stored in the preferences.If set to <0 it will be ignored and not changed/set.*/
    void SetReferenceDoseValue(bool globalSync, DoseValueAbs value = -1);

    bool GetDoseDisplayAbsolute();
    void SetDoseDisplayAbsolute(bool isAbsolute);

    bool GetGlobalIsolineVis();
    bool GetGlobalColorwashVis();

    void SignalReferenceDoseChange(bool globalSync, mitk::DoseValueAbs value, ctkPluginContext* context);
    void SignalPresetMapChange(ctkPluginContext* context);
    void SignalGlobalVisChange(bool globalSync, bool isolineVis, bool colorwashVis, ctkPluginContext* context);
}

#endif
