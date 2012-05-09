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


#ifndef BERRYICONFIGURATIONELEMENTLEGACY_H
#define BERRYICONFIGURATIONELEMENTLEGACY_H

#include <berryBundleLoader.h>

#include <berryIExecutableExtension.h>
#include <berryIConfigurationElement.h>
#include <berryPlatformException.h>

namespace berry {

struct IConfigurationElementLegacy
{

  template<class C>
  C* CreateExecutableExtension(const QString& propertyName, const QString& manifestName)
  {
    QString className = this->GetAttribute(propertyName);
    if (!className.isNull())
    {
      try
      {
        C* cl = m_ClassLoader->LoadClass<C>(GetContributor(), className, manifestName);

        // check if we have extension adapter and initialize
        if (dynamic_cast<IExecutableExtension*>(cl) != 0) {
          // make the call even if the initialization string is null
          dynamic_cast<IExecutableExtension*>(cl)->SetInitializationData(
                IConfigurationElement::Pointer(dynamic_cast<IConfigurationElement*>(this)), propertyName, Object::Pointer(0));

        }

        if (cl == 0)
        {
          BERRY_WARN << "Could not load executable extension " << className.toStdString()
                     << " from " << GetContributor().toStdString();
        }

        return cl;
      }
      catch (const std::exception& e)
      {
        BERRY_ERROR << "Error loading class: " << e.what() << std::endl;
        throw e;
      }
    }

    throw CoreException(QString("Missing attribute: ") + propertyName);
  }

protected:

  IConfigurationElementLegacy(BundleLoader* loader)
    : m_ClassLoader(loader)
  {}

private:

  virtual QString GetContributor() const = 0;

  virtual QString GetAttribute(const QString& propertyName) const = 0;

  BundleLoader* m_ClassLoader;

};

}

#endif // BERRYICONFIGURATIONELEMENTLEGACY_H
