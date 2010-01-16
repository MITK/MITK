/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


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
    bool ReadElement(IConfigurationElement::Pointer element);

};

}

#endif /* BERRYPERSPECTIVEREGISTRYREADER_H_ */
