/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
