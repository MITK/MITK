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
