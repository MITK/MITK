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


#ifndef BERRYQTITEMSELECTION_H_
#define BERRYQTITEMSELECTION_H_

#include "berryUiQtDll.h"

#include <berryIStructuredSelection.h>

#include <QItemSelection>

namespace berry {

class BERRY_UI_QT QtItemSelection : public virtual IStructuredSelection
{

public:

  osgiObjectMacro(QtItemSelection)

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

#endif /* BERRYQTITEMSELECTION_H_ */
