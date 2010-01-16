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


#ifndef BERRYINTROREGISTRY_H_
#define BERRYINTROREGISTRY_H_

#include "berryIIntroRegistry.h"

namespace berry {

struct IExtension;

/**
 * Registry for introduction elements.
 *
 */
class IntroRegistry : public IIntroRegistry {

private:

  static const std::string TAG_INTRO; // = "intro";
  static const std::string TAG_INTROPRODUCTBINDING; // = "introProductBinding";
  static const std::string ATT_INTROID; // = "introId";
  static const std::string ATT_PRODUCTID; // = "productId";

  /**
   * @param targetProductId
   * @param extensions
   * @return
   */
  std::string GetIntroForProduct(const std::string& targetProductId,
      const std::vector<const IExtension*>& extensions) const;

public:

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntroCount()
   */
  int GetIntroCount() const;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntros()
   */
  std::vector<IIntroDescriptor::Pointer> GetIntros() const;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntroForProduct(java.lang.String)
   */
  IIntroDescriptor::Pointer GetIntroForProduct(const std::string& targetProductId) const;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntro(java.lang.String)
   */
  IIntroDescriptor::Pointer GetIntro(const std::string& id) const;

};

}

#endif /* BERRYINTROREGISTRY_H_ */
