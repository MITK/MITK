/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryISourceProviderListener.h"

namespace berry {

void ISourceProviderListener::Events::AddListener(ISourceProviderListener* l)
{
  if (l == nullptr) return;

  multipleSourcesChanged += Delegate2(l, &ISourceProviderListener::SourceChanged);
  singleSourceChanged += Delegate3(l, &ISourceProviderListener::SourceChanged);
}

void ISourceProviderListener::Events::RemoveListener(ISourceProviderListener* l)
{
  if (l == nullptr) return;

  multipleSourcesChanged -= Delegate2(l, &ISourceProviderListener::SourceChanged);
  singleSourceChanged -= Delegate3(l, &ISourceProviderListener::SourceChanged);
}

ISourceProviderListener::~ISourceProviderListener()
{
}

}
