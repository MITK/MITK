/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYVIEWREGISTRYREADER_H_
#define BERRYVIEWREGISTRYREADER_H_

#include "berryRegistryReader.h"

namespace berry
{

class ViewRegistry;

/**
 * \ingroup org_blueberry_ui_internal
 *
 * A strategy to read view extensions from the registry.
 */
class ViewRegistryReader : public RegistryReader
{

public:

  /**
   * General view category id
   */
  static QString GENERAL_VIEW_ID ;

  /**
   * RegistryViewReader constructor comment.
   */
  ViewRegistryReader();

  /**
   * Read the view extensions within a registry.
   * @param in the extension registry
   * @param out the view registry
   */
  void ReadViews(IExtensionRegistry* in, ViewRegistry* out);

protected:
  /**
   * Reads the category element.
   */
  void ReadCategory(const SmartPointer<IConfigurationElement>& element);

  /**
   * readElement method comment.
   */
  bool ReadElement(const SmartPointer<IConfigurationElement>& element) override;

  /**
   * Reads the sticky view element.
   */
  void ReadSticky(const SmartPointer<IConfigurationElement>& element);

  /**
   * Reads the view element.
   */
  void ReadView(const SmartPointer<IConfigurationElement>& element);

private:

  ViewRegistry* viewRegistry;

};

} // namespace berry

#endif /*BERRYVIEWREGISTRYREADER_H_*/
