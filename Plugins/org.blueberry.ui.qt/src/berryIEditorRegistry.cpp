/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIEditorRegistry.h"

namespace berry
{

const int IEditorRegistry::PROP_CONTENTS = 0x01;
const QString IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID =
    "org.blueberry.ui.systemExternalEditor"; //$NON-NLS-1$
const QString IEditorRegistry::SYSTEM_INPLACE_EDITOR_ID =
    "org.blueberry.ui.systemInPlaceEditor"; //$NON-NLS-1$

IEditorRegistry::~IEditorRegistry()
{
}

}
