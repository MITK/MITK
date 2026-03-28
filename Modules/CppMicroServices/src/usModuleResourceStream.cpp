/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleResourceStream.h>

#include <usModuleResource.h>

// 'this' used in base member initializer list
US_MSVC_PUSH_DISABLE_WARNING(4355)

namespace us {

ModuleResourceStream::ModuleResourceStream(const ModuleResource& resource, std::ios_base::openmode mode)
  : ModuleResourceBuffer(resource.GetData(), resource.GetSize(), mode | std::ios_base::in)
  , std::istream(this)
{
}

}

US_MSVC_POP_WARNING
