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


#ifndef BERRYISTRUCTUREDSELECTION_H_
#define BERRYISTRUCTUREDSELECTION_H_

#include "berryISelection.h"

#include <berryObjectVector.h>
#include <org_blueberry_ui_Export.h>
namespace berry
{

/**
 * A selection containing elements.
 */
struct BERRY_UI IStructuredSelection : public ISelection {

  typedef ObjectVector<Object::Pointer> ContainerType;
  typedef ContainerType::const_iterator iterator;

  berryInterfaceMacro(IStructuredSelection, berry);

  /**
   * Returns the first element in this selection, or <code>null</code>
   * if the selection is empty.
   *
   * @return an element, or <code>null</code> if none
   */
  virtual Object::Pointer GetFirstElement() const = 0;

  /**
   * Returns an iterator to the beginning of the elements of this selection.
   *
   * @return an iterator over the selected elements
   */
  virtual iterator Begin() const = 0;

  /**
   * Returns an iterator to the end of the elements of this selection.
   *
   * @return an iterator over the selected elements
   */
  virtual iterator End() const = 0;

  /**
   * Returns the number of elements selected in this selection.
   *
   * @return the number of elements selected
   */
  virtual int Size() const = 0;

  /**
   * Returns the elements in this selection as a vector.
   *
   * @return the selected elements as a vector
   */
  virtual ContainerType::Pointer ToVector() const = 0;

  /*
   * We need to define at least the destructor in this compilation unit
   * because of the export macro above. Otherwise Windows throws
   * a linker error in dependent librraies. The export macro is needed
   * for gcc < 4.5 to correctly mark the type_info object of this class
   * as visible (we have default visibility 'hidden') so that dynamic_cast calls work.
   */
  virtual ~IStructuredSelection();

};

}

#endif /* BERRYISTRUCTUREDSELECTION_H_ */
