/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtEditorPart.h"

namespace berry
{

void QtEditorPart::CreatePartControl(QWidget* parent)
{
  this->CreateQtPartControl(static_cast<QWidget*>(parent));
}

}
