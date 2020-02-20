/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  if (role != ROLE_EDITOR && memento != 0 && serializer != nullptr)
  {
    presentation->RestoreState(serializer, memento);
  }

  return presentation;
}

PresentationFactoryUtil::PresentationFactoryUtil()
{

}

}
