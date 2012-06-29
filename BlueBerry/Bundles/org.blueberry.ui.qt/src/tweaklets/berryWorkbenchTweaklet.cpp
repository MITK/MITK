/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryWorkbenchTweaklet.h"

namespace berry
{

Tweaklets::TweakKey<WorkbenchTweaklet> WorkbenchTweaklet::KEY =
  Tweaklets::TweakKey<WorkbenchTweaklet>();

const std::string WorkbenchTweaklet::DIALOG_ID_SHOW_VIEW =
    "org.blueberry.ui.dialogs.showview";

}
