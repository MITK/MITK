/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQMODELINDEXOBJECT_H_
#define BERRYQMODELINDEXOBJECT_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QModelIndex>

namespace berry
{

class BERRY_UI_QT QModelIndexObject : public Object
{

public:

  berryObjectMacro(berry::QModelIndexObject);

  QModelIndexObject(const QModelIndex& index);

  const QModelIndex& GetQModelIndex() const;

  bool operator==(const Object* obj) const override;

private:

  QModelIndex m_QModelIndex;

};

}

#endif /* BERRYQMODELINDEXOBJECT_H_ */
