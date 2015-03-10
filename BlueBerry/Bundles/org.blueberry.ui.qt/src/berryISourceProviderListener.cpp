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

#include "berryISourceProviderListener.h"

namespace berry {

void ISourceProviderListener::Events::AddListener(ISourceProviderListener* l)
{
  if (l == 0) return;

  multipleSourcesChanged += Delegate2(l, &ISourceProviderListener::SourceChanged);
  singleSourceChanged += Delegate3(l, &ISourceProviderListener::SourceChanged);
}

void ISourceProviderListener::Events::RemoveListener(ISourceProviderListener* l)
{
  if (l == 0) return;

  multipleSourcesChanged -= Delegate2(l, &ISourceProviderListener::SourceChanged);
  singleSourceChanged -= Delegate3(l, &ISourceProviderListener::SourceChanged);
}

ISourceProviderListener::~ISourceProviderListener()
{
}

}
