/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIQtPreferencePage.h"

namespace berry {

void IQtPreferencePage::CreateControl(void* parent)
{
  this->CreateQtControl(static_cast<QWidget*>(parent));
}

void* IQtPreferencePage::GetControl() const
{
  return (void*)this->GetQtControl();
}

}
