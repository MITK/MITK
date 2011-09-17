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

#include "berryISourceProviderListener.h"

namespace berry {

void ISourceProviderListener::Events::AddListener(ISourceProviderListener::Pointer l)
{
  if (l == 0) return;

  multipleSourcesChanged += Delegate2(l.GetPointer(), &ISourceProviderListener::SourceChanged);
  singleSourceChanged += Delegate3(l.GetPointer(), &ISourceProviderListener::SourceChanged);
}

void ISourceProviderListener::Events::RemoveListener(ISourceProviderListener::Pointer l)
{
  if (l == 0) return;

  multipleSourcesChanged -= Delegate2(l.GetPointer(), &ISourceProviderListener::SourceChanged);
  singleSourceChanged -= Delegate3(l.GetPointer(), &ISourceProviderListener::SourceChanged);
}

}
