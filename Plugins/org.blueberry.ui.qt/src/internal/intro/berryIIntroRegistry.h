/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIINTROREGISTRY_H_
#define BERRYIINTROREGISTRY_H_

#include "berryIIntroDescriptor.h"

#include <vector>

namespace berry {

/**
 * Registry for introduction elements.
 *
 * @since 3.0
 */
struct BERRY_UI_QT IIntroRegistry {

    virtual ~IIntroRegistry();
    /**
     * Return the number of introduction extensions known by this registry.
     *
     * @return the number of introduction extensions known by this registry
     */
    virtual int GetIntroCount() const = 0;

    /**
     * Return the introduction extensions known by this registry.
     *
     * @return the introduction extensions known by this registry
     */
    virtual QList<IIntroDescriptor::Pointer> GetIntros() const = 0;

    /**
     * Return the introduction extension that is bound to the given product.
     *
     * @param productId the product identifier
     * @return the introduction extension that is bound to the given product,
     * or <code>null</code> if there is no such binding
     */
    virtual IIntroDescriptor::Pointer GetIntroForProduct(const QString& productId) const = 0;

    /**
     * Find an intro descriptor with the given identifier.
     *
     * @param id the id
     * @return the intro descriptor, or <code>null</code>
     */
    virtual IIntroDescriptor::Pointer GetIntro(const QString& id) const = 0;
};

}

#endif /* BERRYIINTROREGISTRY_H_ */
