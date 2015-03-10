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
    PerspectiveRegistryReader();

    /**
     * Read the view extensions within a registry.
     *
     * @param out the perspective registry to use
     */
    void ReadPerspectives(PerspectiveRegistry* out);

protected:

    /**
     * readElement method comment.
     */
    // for dynamic UI - change access from protected to public
    bool ReadElement(const IConfigurationElement::Pointer& element);

};

}

#endif /* BERRYPERSPECTIVEREGISTRYREADER_H_ */
