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

#include "berryPresentationFactoryUtil.h"

namespace berry
{

const int PresentationFactoryUtil::ROLE_EDITOR = 0x01;

const int PresentationFactoryUtil::ROLE_VIEW = 0x02;

const int PresentationFactoryUtil::ROLE_STANDALONE = 0x03;

const int PresentationFactoryUtil::ROLE_STANDALONE_NOTITLE = 0x04;

StackPresentation::Pointer PresentationFactoryUtil::CreatePresentation(
    IPresentationFactory* factory, int role, QWidget* parent,
    IStackPresentationSite::Pointer site, IPresentationSerializer* serializer,
    IMemento::Pointer memento)
{

  StackPresentation::Pointer presentation;

  switch (role)
  {
  case ROLE_EDITOR:
    presentation = factory->CreateEditorPresentation(parent, site);
    break;
  case ROLE_STANDALONE:
    presentation
        = factory->CreateStandaloneViewPresentation(parent, site, true);
    break;
  case ROLE_STANDALONE_NOTITLE:
    presentation = factory->CreateStandaloneViewPresentation(parent, site,
        false);
    break;
  default:
    presentation = factory->CreateViewPresentation(parent, site);
  }

  //don't initialize editors at creation time - it will not contain any parts
  if (role != ROLE_EDITOR && memento != 0 && serializer != 0)
  {
    presentation->RestoreState(serializer, memento);
  }

  return presentation;
}

PresentationFactoryUtil::PresentationFactoryUtil()
{

}

}
