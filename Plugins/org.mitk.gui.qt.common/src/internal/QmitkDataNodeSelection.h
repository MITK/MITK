/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataNodeSelection_h
#define QmitkDataNodeSelection_h

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

  berry::Object::Pointer GetFirstElement() const override;
  iterator Begin() const override;
  iterator End() const override;

  int Size() const override;

  ContainerType::Pointer ToVector() const override;

  /**
   * @see berry::ISelection::IsEmpty()
   */
  bool IsEmpty() const override;

  bool operator==(const berry::Object* obj) const override;

};

#endif
