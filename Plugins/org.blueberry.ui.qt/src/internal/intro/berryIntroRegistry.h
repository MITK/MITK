/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
  int GetIntroCount() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntros()
   */
  QList<IIntroDescriptor::Pointer> GetIntros() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntroForProduct(java.lang.String)
   */
  IIntroDescriptor::Pointer GetIntroForProduct(const QString& targetProductId) const override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.intro.IIntroRegistry#getIntro(java.lang.String)
   */
  IIntroDescriptor::Pointer GetIntro(const QString& id) const override;

};

}

#endif /* BERRYINTROREGISTRY_H_ */
