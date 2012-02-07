/*=========================================================================

 Program:   BlueBerry Platform
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
