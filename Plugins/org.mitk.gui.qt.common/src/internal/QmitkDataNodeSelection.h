/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkDataNodeSelection.h>
#include <mitkDataNode.h>

#include <berryQtItemSelection.h>

#include <vector>

class QmitkDataNodeSelection: public mitk::DataNodeSelection, public berry::QtItemSelection
{
public:

  berryObjectMacro(QmitkDataNodeSelection);

  QmitkDataNodeSelection();
  QmitkDataNodeSelection(const QItemSelection& sel);

  virtual berry::Object::Pointer GetFirstElement() const;
  virtual iterator Begin() const;
  virtual iterator End() const;

  virtual int Size() const;

  virtual ContainerType::Pointer ToVector() const;

  /**
   * @see berry::ISelection::IsEmpty()
   */
  bool IsEmpty() const;

  bool operator==(const berry::Object* obj) const;

};
