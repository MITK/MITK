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

#include "berryQtFileImageDescriptor.h"

#include "tweaklets/berryImageTweaklet.h"

#include <ctkPlugin.h>

#include <QIcon>
#include <QPixmap>
#include <QByteArray>

namespace berry
{

QtFileImageDescriptor::QtFileImageDescriptor(const QString& filename,
    const QString& pluginid) :
  filename(filename), pluginid(pluginid)
{

}

void* QtFileImageDescriptor::CreateImage(bool returnMissingImageOnError)
{
  if (pluginid.isEmpty())
  {
    QFile f(filename);
    if (f.open(QFile::ReadOnly))
    {
      QByteArray ba = f.readAll();
      return this->CreateFromByteArray(ba);
    }
    else
    {
      BERRY_ERROR << "Could not open file: " << filename;
      if (returnMissingImageOnError)
        return GetMissingImageDescriptor()->CreateImage();

      return 0;
    }
  }
  else
  {
    QSharedPointer<ctkPlugin> plugin(Platform::GetCTKPlugin(pluginid));
    if (!plugin)
    {
      if (returnMissingImageOnError)
        return GetMissingImageDescriptor()->CreateImage();

      return 0;
    }

    QByteArray ba = plugin->getResource(filename);
    if (ba.isEmpty() && returnMissingImageOnError)
      return GetMissingImageDescriptor()->CreateImage();

    return this->CreateFromByteArray(ba);
  }
}

void QtFileImageDescriptor::DestroyImage(void* img)
{
  const QIcon* icon = static_cast<const QIcon*>(img);
  delete icon;
}

QIcon* QtFileImageDescriptor::CreateFromByteArray(const QByteArray& ba)
{
  QPixmap pixmap;
  pixmap.loadFromData(ba);
  QIcon* icon = new QIcon(pixmap);
  return icon;
}

bool QtFileImageDescriptor::operator ==(const Object* o) const
{
  if (const QtFileImageDescriptor* obj = dynamic_cast<const QtFileImageDescriptor*>(o))
  {
    return this->pluginid == obj->pluginid && this->filename == obj->filename;
  }

  return false;
}

}
