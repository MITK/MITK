/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULERESOURCESTREAM_H
#define USMODULERESOURCESTREAM_H

#include <usModuleResourceBuffer_p.h>

#include <fstream>

namespace us {

class ModuleResource;

/**
 * \ingroup MicroServices
 *
 * An input stream class for ModuleResource objects.
 *
 * This class provides access to the resource data embedded in a module's
 * shared library via a STL input stream interface.
 *
 * \see ModuleResource for an example how to use this class.
 */
class MITKCPPMICROSERVICES_EXPORT ModuleResourceStream : private ModuleResourceBuffer, public std::istream
{

public:

  /**
   * Construct a %ModuleResourceStream object.
   *
   * @param resource The ModuleResource object for which an input stream
   * should be constructed.
   * @param mode The open mode of the stream. If \c std::ios_base::binary
   * is used, the resource data will be treated as binary data, otherwise
   * the data is interpreted as text data and the usual platform specific
   * end-of-line translations take place.
   */
  ModuleResourceStream(const ModuleResource& resource,
                       std::ios_base::openmode mode = std::ios_base::in);

private:

  // purposely not implemented
  ModuleResourceStream(const ModuleResourceStream&);
  ModuleResourceStream& operator=(const ModuleResourceStream&);
};

}

#endif // USMODULERESOURCESTREAM_H
