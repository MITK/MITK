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

#ifndef BERRYIEXTENSIONELEMENT_H_
#define BERRYIEXTENSIONELEMENT_H_

#include "berryLog.h"

#include <org_blueberry_osgi_Export.h>

#include "berryBundleLoader.h"
#include "berryPlatformException.h"

#include "berryExtensionType.h"
#include "berryIExecutableExtension.h"
#include "berryIExtension.h"

#include <vector>
#include <string>

namespace berry {

struct IExtension;

struct BERRY_OSGI IConfigurationElement : public Object
{

  berryObjectMacro(IConfigurationElement);

public:

  typedef std::vector<IConfigurationElement::Pointer> vector;

  template<class C>
  C* CreateExecutableExtension(const std::string& propertyName, const std::string& manifestName)
  {
    std::string className;
    if (this->GetAttribute(propertyName, className))
    {
      try
      {
        C* cl = m_ClassLoader->LoadClass<C>(m_Contributor, className, manifestName);

        // check if we have extension adapter and initialize
        if (dynamic_cast<IExecutableExtension*>(cl) != 0) {
          // make the call even if the initialization string is null
          dynamic_cast<IExecutableExtension*>(cl)->SetInitializationData(Pointer(this), propertyName, Object::Pointer(0));

        }

        if (cl == 0)
        {
          BERRY_WARN << "Could not load executable extension " << className << " from " << GetContributor();
        }

        return cl;
      }
      catch (Poco::Exception& e)
      {
        BERRY_ERROR << "Error loading class: " << e.displayText() << std::endl;
        throw e;
      }
    }

    throw CoreException("Missing attribute", propertyName);
  }

  template<class C>
  C* CreateExecutableExtension(const std::string& propertyName)
  {
    std::string className;
    if (this->GetAttribute(propertyName, className))
    {
      std::string contributor = this->GetContributor();
      QSharedPointer<ctkPlugin> plugin = Platform::GetCTKPlugin(QString::fromStdString(contributor));
      if (!plugin.isNull())
      {
        // immediately start the plugin but do not change the plugins autostart setting
        plugin->start(ctkPlugin::START_TRANSIENT);

        QString typeName = plugin->getSymbolicName() + "_" + QString::fromStdString(className);
        int extensionTypeId = ExtensionType::type(typeName.toLatin1().data());
        if (extensionTypeId == 0)
        {
          BERRY_WARN << "The class " << className << " was not registered as an Extension Type using BERRY_REGISTER_EXTENSION_CLASS(type, pluginContext) or you forgot to run Qt's moc on the header file. "
                        "Legacy BlueBerry bundles should use CreateExecutableExtension<C>(propertyName, C::GetManifestName()) instead.";
        }
        else
        {
          QObject* obj = ExtensionType::construct(extensionTypeId);
          // check if we have extension adapter and initialize
          if (IExecutableExtension* execExt = qobject_cast<IExecutableExtension*>(obj))
          {
            // make the call even if the initialization string is null
            execExt->SetInitializationData(Pointer(this), propertyName, Object::Pointer(0));
          }

          C* interface = qobject_cast<C*>(obj);
          if (interface == 0)
          {
            BERRY_WARN << "The QObject subclass " << className << " does not seem to implement the required interface class, or you forgot the Q_INTERFACES macro.";
          }
          return interface;
        }
      }
      else
      {
        BERRY_WARN << "Trying to create an executable extension (from "
                   << this->GetDeclaringExtension()->GetExtensionPointIdentifier()
                   << " in " << contributor << ") from a non-CTK plug-in. "
                      "Use the CreateExecutableExtension<C>(propertyName, manifestName) method instead.";
      }
    }
    return 0;
  }

  virtual bool GetAttribute(const std::string& name, std::string& value) const = 0;
  virtual bool GetBoolAttribute(const std::string& name, bool& value) const = 0;

  virtual const std::vector<IConfigurationElement::Pointer> GetChildren() const = 0;
  virtual const std::vector<IConfigurationElement::Pointer> GetChildren(const std::string& name) const = 0;

  virtual std::string GetValue() const = 0;

  virtual std::string GetName() const = 0;
  virtual const IConfigurationElement* GetParent() const = 0;

  virtual const std::string& GetContributor() const = 0;
  virtual const IExtension* GetDeclaringExtension() const = 0;

  virtual ~IConfigurationElement() {};


protected:
  BundleLoader* m_ClassLoader;
  std::string m_Contributor;

};

}  // namespace berry


#endif /*BERRYIEXTENSIONELEMENT_H_*/
