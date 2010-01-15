/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryErrorViewPart.h"

namespace cherry
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
