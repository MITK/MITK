/*=========================================================================

 Program:   openCherry Platform
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

#ifndef CHERRYPRESENTATIONSERIALIZER_H_
#define CHERRYPRESENTATIONSERIALIZER_H_

#include "../presentations/cherryIPresentationSerializer.h"
#include "../presentations/cherryIPresentablePart.h"

#include <vector>

namespace cherry
{

/**
 * This class is used to map IPresentableParts onto string IDs
 */
class PresentationSerializer: public IPresentationSerializer
{

private:

  std::vector<IPresentablePart::Pointer> parts;

public:

  PresentationSerializer(
      const std::vector<IPresentablePart::Pointer>& presentableParts);

  /* (non-Javadoc)
   * @see org.opencherry.ui.presentations.IPresentationSerializer#getId(org.opencherry.ui.presentations.IPresentablePart)
   */
  std::string GetId(IPresentablePart::Pointer part);

  /* (non-Javadoc)
   * @see org.opencherry.ui.presentations.IPresentationSerializer#getPart(java.lang.String)
   */
  IPresentablePart::Pointer GetPart(const std::string& id);

};

}

#endif /* CHERRYPRESENTATIONSERIALIZER_H_ */
