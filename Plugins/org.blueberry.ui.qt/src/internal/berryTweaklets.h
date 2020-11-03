/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYTWEAKLETS_H_
#define BERRYTWEAKLETS_H_


#include <org_blueberry_ui_qt_Export.h>

#include <QString>

namespace berry {

struct BERRY_UI_QT TweakKey_base
{
  QString tweakClass;

  /**
   * @param tweakClass
   */
  TweakKey_base(const QString& _tweakClass);

  bool operator==(const TweakKey_base& obj) const;
  bool operator<(const TweakKey_base& obj) const;
};

}

BERRY_UI_QT uint qHash(const berry::TweakKey_base& key);

#include <berryIConfigurationElement.h>
#include <berryPlatform.h>
#include <berryIExtensionRegistry.h>
#include <berryCoreException.h>


namespace berry {

class BERRY_UI_QT Tweaklets
{

public:

  template<typename I>
  struct TweakKey: public TweakKey_base
  {
    TweakKey() :
      TweakKey_base("")
    {

      tweakClass = QString(qobject_interface_iid<I*>());
    }

    TweakKey(const QString& _tweakClass) :
      TweakKey_base(_tweakClass)
    {
    }
  };

  static void SetDefault(const TweakKey_base& definition,
                         QObject* implementation);

  static void Clear();

  template<typename I>
  static I* Get(const TweakKey<I>& definition)
  {
    TweakletMap::const_iterator iter = tweaklets.find(definition);
    QObject* result;
    if (iter == tweaklets.end())
    {
      result = CreateTweaklet(definition);
      if (result == nullptr)
      {
        result = GetDefault(definition);
      }
      Q_ASSERT(result != nullptr);
      tweaklets.insert(definition, result);
      return qobject_cast<I*>(result);
    }

    return qobject_cast<I*>(iter.value());
  }

private:

  typedef QHash<TweakKey_base, QObject*> TweakletMap;

  static TweakletMap defaults;
  static TweakletMap tweaklets;

  /**
   * @param definition
   * @return
   */
  template<typename I>
  static QObject* GetDefault(const TweakKey<I>& definition)
  {
    TweakletMap::const_iterator iter = defaults.find(definition);
    if (iter == defaults.end())
      return nullptr;

    return iter.value();
  }

  /**
   * @param definition
   * @return
   */
  template<typename I>
  static QObject* CreateTweaklet(const TweakKey<I>& definition)
  {
    QList<IConfigurationElement::Pointer> elements =
        Platform::GetExtensionRegistry()
        ->GetConfigurationElementsFor("org.blueberry.ui.tweaklets");
    for (int i = 0; i < elements.size(); i++)
    {
      QString attr = elements[i]->GetAttribute("definition");
      if (!attr.isNull() && definition.tweakClass == attr)
      {
        try
        {
          QObject* tweaklet = elements[i]->CreateExecutableExtension("implementation");
          tweaklets.insert(definition, tweaklet);
          return tweaklet;
        }
        catch (const CoreException& e)
        {
          //StatusManager.getManager().handle(
          //    StatusUtil.newStatus(IStatus.ERR,
          BERRY_ERROR << "Error with extension " << elements[i] << e.what();
          //    StatusManager.LOG);
        }
      }
    }
    return nullptr;
  }

};

}


#endif /* BERRYTWEAKLETS_H_ */
