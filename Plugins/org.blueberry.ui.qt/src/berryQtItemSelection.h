/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTITEMSELECTION_H_
#define BERRYQTITEMSELECTION_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryIStructuredSelection.h>

#include <QItemSelection>

namespace berry {

class BERRY_UI_QT QtItemSelection : public virtual IStructuredSelection
{

public:

  berryObjectMacro(QtItemSelection);

  QtItemSelection();
  QtItemSelection(const QItemSelection& sel);

  QItemSelection GetQItemSelection() const;

  bool IsEmpty() const override;

  Object::Pointer GetFirstElement() const override;
  iterator Begin() const override;
  iterator End() const override;
  int Size() const override;
  ContainerType::Pointer ToVector() const override;

  bool operator==(const Object* obj) const override;

private:

  ContainerType::Pointer m_Selection;
  QItemSelection m_QItemSelection;

};

}

#endif /* BERRYQTITEMSELECTION_H_ */
