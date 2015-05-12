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

  virtual berry::Object::Pointer GetFirstElement() const override;
  virtual iterator Begin() const override;
  virtual iterator End() const override;

  virtual int Size() const override;

  virtual ContainerType::Pointer ToVector() const override;

  /**
   * @see berry::ISelection::IsEmpty()
   */
  bool IsEmpty() const override;

  bool operator==(const berry::Object* obj) const override;

};
