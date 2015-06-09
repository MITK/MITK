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

#include "berryEmptyPerspective.h"

namespace berry
{

QString EmptyPerspective::LastPerspective;
const QString EmptyPerspective::PERSP_ID =
    "org.blueberry.uitest.util.EmptyPerspective";
const QString EmptyPerspective::PERSP_ID2 =
    "org.blueberry.uitest.util.EmptyPerspective2";

EmptyPerspective::EmptyPerspective()
{

}

EmptyPerspective::EmptyPerspective(const EmptyPerspective& other)
  : QObject()
{
  Q_UNUSED(other)
}

QString EmptyPerspective::GetLastPerspective()
{
  return LastPerspective;
}

void EmptyPerspective::SetLastPerspective(const QString& perspId)
{
  LastPerspective = perspId;
}

void EmptyPerspective::CreateInitialLayout(IPageLayout::Pointer layout)
{
  SetLastPerspective(layout->GetDescriptor()->GetId());
  // do no layout, this is the empty perspective
}

}
