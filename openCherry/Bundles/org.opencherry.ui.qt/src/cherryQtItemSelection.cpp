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

#include "cherryQtItemSelection.h"

namespace cherry {

QtItemSelection::QtItemSelection()
{

}

QtItemSelection::QtItemSelection(const QItemSelection& sel)
: m_QItemSelection(sel)
{

}

bool QtItemSelection::IsEmpty() const
{
  return m_QItemSelection.indexes().isEmpty();
}

const QItemSelection& QtItemSelection::GetQItemSelection() const
{
  return m_QItemSelection;
}

}
