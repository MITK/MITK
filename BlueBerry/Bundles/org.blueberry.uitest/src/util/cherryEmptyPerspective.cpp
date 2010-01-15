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

#include "cherryEmptyPerspective.h"

namespace cherry
{

std::string EmptyPerspective::LastPerspective;
const std::string EmptyPerspective::PERSP_ID =
    "org.opencherry.uitest.util.EmptyPerspective";
const std::string EmptyPerspective::PERSP_ID2 =
    "org.opencherry.uitest.util.EmptyPerspective2";

std::string EmptyPerspective::GetLastPerspective()
{
  return LastPerspective;
}

void EmptyPerspective::SetLastPerspective(const std::string& perspId)
{
  LastPerspective = perspId;
}

EmptyPerspective::EmptyPerspective()
{

}

void EmptyPerspective::CreateInitialLayout(IPageLayout::Pointer layout)
{
  SetLastPerspective(layout->GetDescriptor()->GetId());
  // do no layout, this is the empty perspective
}

}
