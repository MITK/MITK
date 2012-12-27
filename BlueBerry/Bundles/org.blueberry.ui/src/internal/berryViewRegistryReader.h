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
  static std::string GENERAL_VIEW_ID ;

  /**
   * RegistryViewReader constructor comment.
   */
  ViewRegistryReader();

  /**
   * Read the view extensions within a registry.
   * @param in the extension registry
   * @param out the view registry
   */
  void ReadViews(ViewRegistry* out);

protected:
  /**
   * Reads the category element.
   */
  void ReadCategory(IConfigurationElement::Pointer element);

  /**
   * readElement method comment.
   */
  bool ReadElement(IConfigurationElement::Pointer element);

  /**
   * Reads the sticky view element.
   */
  void ReadSticky(IConfigurationElement::Pointer element);

  /**
   * Reads the view element.
   */
  void ReadView(IConfigurationElement::Pointer element);

private:

  ViewRegistry* viewRegistry;

};

} // namespace berry

#endif /*BERRYVIEWREGISTRYREADER_H_*/
