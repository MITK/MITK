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

ErrorViewPart::ErrorViewPart(const std::string& title, const std::string& error) :
  title(title), error(error)
{

}

void ErrorViewPart::CreatePartControl(void* parent)
{
  if (!error.empty())
  {
    statusPart = Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreateStatusPart(
        parent, title, error);
  }
}

void ErrorViewPart::SetPartName(const std::string& newName)
{
  ViewPart::SetPartName(newName);
}

void ErrorViewPart::SetFocus()
{
}

}
