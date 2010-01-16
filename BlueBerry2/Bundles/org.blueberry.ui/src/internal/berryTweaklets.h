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

#ifndef BERRYTWEAKLETS_H_
#define BERRYTWEAKLETS_H_

#include <string>
#include <vector>
#include <map>

#include <osgi/framework/Object.h>
#include <berryIConfigurationElement.h>
#include <berryIExtensionPointService.h>
#include <berryPlatform.h>

#include "../berryUiDll.h"

namespace berry
{

class BERRY_UI Tweaklets
{

public:

  struct TweakKey_base
  {
    std::string tweakClass;

    /**
     * @param tweakClass
     */
    TweakKey_base(const std::string& _tweakClass) :
      tweakClass(_tweakClass)
    {

    }

    /* (non-Javadoc)
     * @see java.lang.Object#hashCode()
     */
    //    int hashCode() {
    //      final int prime = 31;
    //      int result = 1;
    //      result = prime * result
    //          + ((tweakClass == null) ? 0 : tweakClass.hashCode());
    //      return result;
    //    }

    /* (non-Javadoc)
     * @see java.lang.Object#equals(java.lang.Object)
     */
    bool operator==(const TweakKey_base& obj) const
    {
      if (this == &obj)
        return true;

      return tweakClass == obj.tweakClass;
    }

    bool operator<(const TweakKey_base& obj) const
    {
      return tweakClass < obj.tweakClass;
    }
  };

  template<typename I>
  struct TweakKey: public TweakKey_base
  {
    TweakKey() :
      TweakKey_base("")
    {
      tweakClass = I::GetManifestName();
    }

    TweakKey(const std::string& _tweakClass) :
      TweakKey_base(_tweakClass)
    {
    }
  };

  static void SetDefault(const TweakKey_base& definition,
      Object::Pointer implementation)
  {
    defaults.insert(std::make_pair(definition, implementation));
  }

  template<typename I>
  static typename I::Pointer Get(const TweakKey<I>& definition)
  {
    TweakletMap::const_iterator iter = tweaklets.find(definition);
    typename I::Pointer result;
    if (iter == tweaklets.end())
    {
      result = CreateTweaklet(definition);
      if (result.IsNull())
      {
        result = GetDefault(definition);
      }
      poco_assert(result.IsNotNull());
      tweaklets.insert(std::make_pair(definition, result));
      return result;
    }

    return iter->second.Cast<I>();
  }

private:

  typedef std::map<TweakKey_base, Object::Pointer> TweakletMap;

  static TweakletMap defaults;
  static TweakletMap tweaklets;

  /**
   * @param definition
   * @return
   */
  template<typename I>
  static typename I::Pointer GetDefault(const TweakKey<I>& definition)
  {
    TweakletMap::const_iterator iter = defaults.find(definition);
    if (iter == defaults.end())
    return typename I::Pointer(0);

    return iter->second.Cast<I>();
  }

  /**
   * @param definition
   * @return
   */
  template<typename I>
  static typename I::Pointer CreateTweaklet(const TweakKey<I>& definition)
  {
    std::vector<IConfigurationElement::Pointer> elements = Platform
    ::GetExtensionPointService()
    ->GetConfigurationElementsFor("org.blueberry.ui.tweaklets"); //$NON-NLS-1$
    for (unsigned int i = 0; i < elements.size(); i++)
    {
      std::string attr;
      if (elements[i]->GetAttribute("definition", attr) &&
          definition.tweakClass == attr)
      {
        try
        {
          typename I::Pointer tweaklet(elements[i]->CreateExecutableExtension<I>("implementation")); //$NON-NLS-1$
          tweaklets.insert(std::make_pair(definition, tweaklet));
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
    return typename I::Pointer(0);
  }

};

}

#endif /* BERRYTWEAKLETS_H_ */
