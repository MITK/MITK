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


#ifndef BERRYPERSPECTIVEREGISTRYREADER_H_
#define BERRYPERSPECTIVEREGISTRYREADER_H_

#include "berryRegistryReader.h"

#include <berryIConfigurationElement.h>

namespace berry {

struct IExtensionRegistry;
class PerspectiveRegistry;

/**
 * A strategy to read perspective extensions from the registry.
 */
class PerspectiveRegistryReader : public RegistryReader {

private:

  PerspectiveRegistry* registry;

public:

  /**
   * RegistryViewReader constructor comment.
   */
  PerspectiveRegistryReader(PerspectiveRegistry* out);

  /**
   * Read the view extensions within a registry.
   *
   * @param out the perspective registry to use
   */
  void ReadPerspectives(IExtensionRegistry* in);

  /**
   * readElement method comment.
   */
  bool ReadElement(const IConfigurationElement::Pointer& element) override;

};

}

#endif /* BERRYPERSPECTIVEREGISTRYREADER_H_ */
