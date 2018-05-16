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

#ifndef BERRYIPRODUCT_H
#define BERRYIPRODUCT_H

#include <org_blueberry_core_runtime_Export.h>

#include <berryObject.h>

#include <QString>
#include <QSharedPointer>

class ctkPlugin;

namespace berry {

/**
 * Products are the BlueBerry unit of branding.  From the runtime point of view they have
 * a name, id and description and identify the BlueBerry application to run.
 * <p>
 * Since the bulk of the branding related information is
 * specific to the UI, products also carry an arbitrary set of properties.  The valid set of
 * key-value pairs and their interpretation defined by the UI of the target environment.
 * For example, in the standard BlueBerry UI, <code>IProductConstants</code>
 * the properties of interest to the UI.  Other clients may specify additional properties.
 * </p><p>
 * Products can be defined directly using extensions to the <code>org.blueberry.core.runtime.products</code>
 * extension point or by using facilities provided by IProductProvider implementations.
 *
 * @see IProductProvider
 */
struct org_blueberry_core_runtime_EXPORT IProduct : public virtual Object
{
  berryObjectMacro(IProduct);

  ~IProduct() override;

  /**
   * Returns the application associated with this product.  This information is used
   * to guide the runtime as to what application extension to create and execute.
   *
   * @return this product's application or <code>null</code> if none
   */
  virtual QString GetApplication() const = 0;

  /**
   * Returns the name of this product.  The name is typically used in the title
   * bar of UI windows.
   *
   * @return the name of this product or <code>null</code> if none
   */
  virtual QString GetName() const = 0;

  /**
   * Returns the text description of this product
   *
   * @return the description of this product or <code>null</code> if none
   */
  virtual QString GetDescription() const = 0;

  /** Returns the unique product id of this product.
   *
   * @return the id of this product
   */
  virtual QString GetId() const = 0;

  /**
   * Returns the property of this product with the given key.
   * <code>null</code> is returned if there is no such key/value pair.
   *
   * @param key the name of the property to return
   * @return the value associated with the given key or <code>null</code> if none
   */
  virtual QString GetProperty(const QString& key) const = 0;

  /**
   * Returns the plug-in which is responsible for the definition of this product.
   * Typically this is used as a base for searching for images and other files
   * that are needed in presenting the product.
   *
   * @return the plug-in which defines this product or <code>null</code> if none
   */
  virtual QSharedPointer<ctkPlugin> GetDefiningPlugin() const = 0;
};

}

#endif // BERRYIPRODUCT_H
