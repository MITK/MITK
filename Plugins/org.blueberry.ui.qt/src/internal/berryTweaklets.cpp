/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryTweaklets.h"

namespace berry
{

QHash<TweakKey_base, QObject*> Tweaklets::defaults;
QHash<TweakKey_base, QObject*> Tweaklets::tweaklets;


TweakKey_base::TweakKey_base(const QString& _tweakClass) :
  tweakClass(_tweakClass)
{ }

bool TweakKey_base::operator==(const TweakKey_base& obj) const
{
  if (this == &obj)
    return true;

  return tweakClass == obj.tweakClass;
}

bool TweakKey_base::operator<(const TweakKey_base& obj) const
{
  return tweakClass < obj.tweakClass;
}

void Tweaklets::SetDefault(const TweakKey_base& definition,
                           QObject* implementation)
{
  defaults.insert(definition, implementation);
}

void Tweaklets::Clear()
{
  // BERRY_DEBUG << "Clearing tweaklets";
  tweaklets.clear();
  defaults.clear();
}

}

uint qHash(const berry::TweakKey_base& key)
{
  return qHash(key.tweakClass);
}
