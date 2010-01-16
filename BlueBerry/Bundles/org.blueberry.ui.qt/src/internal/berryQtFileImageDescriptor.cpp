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

#include "berryQtFileImageDescriptor.h"

#include "tweaklets/berryImageTweaklet.h"

#include <QIcon>
#include <QPixmap>
#include <QByteArray>

#include <Poco/FileStream.h>
#include <Poco/Exception.h>

namespace berry
{

QtFileImageDescriptor::QtFileImageDescriptor(const std::string& filename,
    const std::string& pluginid) :
  filename(filename), pluginid(pluginid)
{

}

void* QtFileImageDescriptor::CreateImage(bool returnMissingImageOnError)
{
  if (pluginid.empty())
  {
    try
    {
      Poco::FileInputStream fs(filename);
      return this->CreateFromStream(&fs);
    } catch (const Poco::FileNotFoundException* e)
    {
      BERRY_ERROR << e->displayText() << std::endl;
      if (returnMissingImageOnError)
        return GetMissingImageDescriptor()->CreateImage();
    }
  }
  else
  {
    IBundle::Pointer bundle(Platform::GetBundle(pluginid));
    if (!bundle)
    {
      if (returnMissingImageOnError)
        return GetMissingImageDescriptor()->CreateImage();

      return 0;
    }

    std::istream* s = bundle->GetResource(filename);
    if (!s && returnMissingImageOnError)
      return GetMissingImageDescriptor()->CreateImage();

    if (s) {
      void* image = this->CreateFromStream(s);
      delete s;
      return image;
    }
  }

  return 0;
}

void QtFileImageDescriptor::DestroyImage(void* img)
{
  const QIcon* icon = static_cast<const QIcon*>(img);
  delete icon;
}

QIcon* QtFileImageDescriptor::CreateFromStream(std::istream* s)
{
  s->seekg(0, std::ios::end);
  std::ios::pos_type length = s->tellg();
  s->seekg(0, std::ios::beg);

  char* data = new char[length];
  s->read(data, length);
  QPixmap pixmap;
  pixmap.loadFromData(QByteArray::fromRawData(data, length));
  QIcon* icon = new QIcon(pixmap);
  delete[] data;

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
