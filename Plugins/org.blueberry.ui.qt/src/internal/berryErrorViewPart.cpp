/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryErrorViewPart.h"

namespace berry
{

ErrorViewPart::ErrorViewPart()
{
}

ErrorViewPart::ErrorViewPart(const QString& title, const QString& error) :
  title(title), error(error)
{

}

void ErrorViewPart::CreatePartControl(QWidget* parent)
{
  if (!error.isEmpty())
  {
    statusPart = Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreateStatusPart(
        parent, title, error);
  }
}

void ErrorViewPart::SetPartName(const QString& newName)
{
  ViewPart::SetPartName(newName);
}

void ErrorViewPart::SetFocus()
{
}

}
