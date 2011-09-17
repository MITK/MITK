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

#ifndef BERRYPRESENTATIONFACTORYUTIL_H_
#define BERRYPRESENTATIONFACTORYUTIL_H_

#include "../presentations/berryIPresentationFactory.h"
#include "../presentations/berryIStackPresentationSite.h"
#include "../presentations/berryIPresentationSerializer.h"

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
      IPresentationFactory* factory, int role, void* parent,
      IStackPresentationSite::Pointer site,
      IPresentationSerializer* serializer, IMemento::Pointer memento);

private:

  PresentationFactoryUtil();

};

}

#endif /* BERRYPRESENTATIONFACTORYUTIL_H_ */
