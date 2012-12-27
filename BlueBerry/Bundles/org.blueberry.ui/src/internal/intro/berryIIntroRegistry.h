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
struct BERRY_UI IIntroRegistry {

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
    virtual std::vector<IIntroDescriptor::Pointer> GetIntros() const = 0;

    /**
     * Return the introduction extension that is bound to the given product.
     *
     * @param productId the product identifier
     * @return the introduction extension that is bound to the given product,
     * or <code>null</code> if there is no such binding
     */
    virtual IIntroDescriptor::Pointer GetIntroForProduct(const std::string& productId) const = 0;

    /**
     * Find an intro descriptor with the given identifier.
     *
     * @param id the id
     * @return the intro descriptor, or <code>null</code>
     */
    virtual IIntroDescriptor::Pointer GetIntro(const std::string& id) const = 0;
};

}

#endif /* BERRYIINTROREGISTRY_H_ */
