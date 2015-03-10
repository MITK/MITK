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

  static const QString TAG_INTRO; // = "intro";
  static const QString TAG_INTROPRODUCTBINDING; // = "introProductBinding";
  static const QString ATT_INTROID; // = "introId";
  static const QString ATT_PRODUCTID; // = "productId";

  /**
   * @param targetProductId
   * @param extensions
   * @return
   */
  QString GetIntroForProduct(const QString& targetProductId,
                             const QList<SmartPointer<IExtension> >& extensions) const;

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
  QList<IIntroDescriptor::Pointer> GetIntros() const;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntroForProduct(java.lang.String)
   */
  IIntroDescriptor::Pointer GetIntroForProduct(const QString& targetProductId) const;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntro(java.lang.String)
   */
  IIntroDescriptor::Pointer GetIntro(const QString& id) const;

};

}

#endif /* BERRYINTROREGISTRY_H_ */
