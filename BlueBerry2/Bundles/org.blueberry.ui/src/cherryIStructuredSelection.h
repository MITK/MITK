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


#ifndef CHERRYISTRUCTUREDSELECTION_H_
#define CHERRYISTRUCTUREDSELECTION_H_

#include "cherryISelection.h"

#include <osgi/framework/ObjectVector.h>

namespace cherry
{

/**
 * A selection containing elements.
 */
struct IStructuredSelection : public ISelection {

  typedef ObjectVector<Object::Pointer> ContainerType;
  typedef ContainerType::const_iterator iterator;

  osgiInterfaceMacro(cherry::IStructuredSelection);

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

};

}

#endif /* CHERRYISTRUCTUREDSELECTION_H_ */
