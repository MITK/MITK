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
