/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
