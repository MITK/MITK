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

#ifndef BERRYTWEAKLETS_H_
#define BERRYTWEAKLETS_H_

#include <org_blueberry_ui_Export.h>

#include <QString>

namespace berry {

struct BERRY_UI TweakKey_base
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

BERRY_UI uint qHash(const berry::TweakKey_base& key);

#include <berryIConfigurationElement.h>
#include <berryIExtensionPointService.h>
#include <berryPlatform.h>


namespace berry {

class BERRY_UI Tweaklets
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
      if (result == 0)
      {
        result = GetDefault(definition);
      }
      Q_ASSERT(result != 0);
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
      return 0;

    return iter.value();
  }

  /**
   * @param definition
   * @return
   */
  template<typename I>
  static QObject* CreateTweaklet(const TweakKey<I>& definition)
  {
    std::vector<IConfigurationElement::Pointer> elements =
        Platform::GetExtensionPointService()
        ->GetConfigurationElementsFor("org.blueberry.ui.tweaklets"); //$NON-NLS-1$
    for (unsigned int i = 0; i < elements.size(); i++)
    {
      std::string attr;
      if (elements[i]->GetAttribute("definition", attr) &&
          definition.tweakClass == QString::fromStdString(attr))
      {
        try
        {
          QObject* tweaklet = elements[i]->CreateExecutableExtension<QObject>("implementation");
          tweaklets.insert(definition, tweaklet);
          return tweaklet;
        }
        catch (CoreException e)
        {
          //StatusManager.getManager().handle(
          //    StatusUtil.newStatus(IStatus.ERR,
          BERRY_ERROR << "Error with extension " << elements[i] << e.what();
          //    StatusManager.LOG);
        }
      }
    }
    return 0;
  }

};

}


#endif /* BERRYTWEAKLETS_H_ */
