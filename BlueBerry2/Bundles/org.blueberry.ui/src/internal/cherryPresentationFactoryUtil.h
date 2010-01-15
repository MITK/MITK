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

#ifndef CHERRYPRESENTATIONFACTORYUTIL_H_
#define CHERRYPRESENTATIONFACTORYUTIL_H_

#include "../presentations/cherryIPresentationFactory.h"
#include "../presentations/cherryIStackPresentationSite.h"
#include "../presentations/cherryIPresentationSerializer.h"

namespace cherry
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

#endif /* CHERRYPRESENTATIONFACTORYUTIL_H_ */
