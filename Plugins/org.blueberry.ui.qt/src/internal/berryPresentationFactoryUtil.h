/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPRESENTATIONFACTORYUTIL_H_
#define BERRYPRESENTATIONFACTORYUTIL_H_

#include "presentations/berryIPresentationFactory.h"
#include "presentations/berryIStackPresentationSite.h"
#include "presentations/berryIPresentationSerializer.h"

namespace berry
{

class PresentationFactoryUtil
{

public:

  static const int ROLE_EDITOR; // = 0x01;

  static const int ROLE_VIEW; // = 0x02;

  static const int ROLE_STANDALONE; // = 0x03;

  static const int ROLE_STANDALONE_NOTITLE; // = 0x04;

  static StackPresentation::Pointer CreatePresentation(
      IPresentationFactory* factory, int role, QWidget* parent,
      IStackPresentationSite::Pointer site,
      IPresentationSerializer* serializer, IMemento::Pointer memento);

private:

  PresentationFactoryUtil();

};

}

#endif /* BERRYPRESENTATIONFACTORYUTIL_H_ */
