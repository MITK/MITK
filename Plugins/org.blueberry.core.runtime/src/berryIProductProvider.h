/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYIPRODUCTPROVIDER_H
#define BERRYIPRODUCTPROVIDER_H

#include <berrySmartPointer.h>

#include <QObject>

namespace berry {

struct IProduct;

/**
 * Product providers define products (units of branding) which have been installed in
 * the current system.  Typically, a configuration agent (i.e., plug-in installer) will
 * define a product provider so that it can report to the system the products it has installed.
 * <p>
 * Products are normally defined and installed in the system using extensions to the
 * <code>org.blueberry.core.runtime.products</code> extension point.  In cases where
 * the notion of product is defined by alternate means (e.g., primary feature), an <code>IProductProvider</code>
 * can be installed in this extension point using an executable extension.  The provider
 * then acts as a proxy for the product extensions that represent the products installed.
 * </p>
 *
 * @see IProduct
 */
struct IProductProvider
{
  virtual ~IProductProvider() {}

  /**
   * Returns the human-readable name of this product provider.
   *
   * @return the name of this product provider
   */
  virtual QString GetName() const = 0;

  /**
   * Returns the products provided by this provider.
   *
   * @return the products provided by this provider
   */
  virtual QList<SmartPointer<IProduct>> GetProducts() const = 0;
};

}

Q_DECLARE_INTERFACE(berry::IProductProvider, "org.blueberry.core.runtime.IProductProvider")

#endif // BERRYIPRODUCTPROVIDER_H

