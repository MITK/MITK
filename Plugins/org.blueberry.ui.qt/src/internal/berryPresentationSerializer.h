/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPRESENTATIONSERIALIZER_H_
#define BERRYPRESENTATIONSERIALIZER_H_

#include "presentations/berryIPresentationSerializer.h"
#include "presentations/berryIPresentablePart.h"


namespace berry
{

/**
 * This class is used to map IPresentableParts onto string IDs
 */
class PresentationSerializer: public IPresentationSerializer
{

private:

  QList<IPresentablePart::Pointer> parts;

public:

  PresentationSerializer(
      const QList<IPresentablePart::Pointer>& presentableParts);

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentationSerializer#getId(org.blueberry.ui.presentations.IPresentablePart)
   */
  QString GetId(IPresentablePart::Pointer part) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentationSerializer#getPart(java.lang.String)
   */
  IPresentablePart::Pointer GetPart(const QString& id) override;

};

}

#endif /* BERRYPRESENTATIONSERIALIZER_H_ */
