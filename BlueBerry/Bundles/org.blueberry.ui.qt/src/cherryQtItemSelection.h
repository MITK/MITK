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


#ifndef CHERRYQTITEMSELECTION_H_
#define CHERRYQTITEMSELECTION_H_

#include "cherryUiQtDll.h"

#include <cherryIStructuredSelection.h>

#include <QItemSelection>

namespace cherry {

class CHERRY_UI_QT QtItemSelection : public virtual IStructuredSelection
{

public:

  cherryObjectMacro(QtItemSelection)

  QtItemSelection();
  QtItemSelection(const QItemSelection& sel);

  QItemSelection GetQItemSelection() const;

  bool IsEmpty() const;

  Object::Pointer GetFirstElement() const;
  iterator Begin() const;
  iterator End() const;
  int Size() const;
  ContainerType::Pointer ToVector() const;

  bool operator==(const Object* obj) const;

private:

  ContainerType::Pointer m_Selection;
  QItemSelection m_QItemSelection;

};

}

#endif /* CHERRYQTITEMSELECTION_H_ */
