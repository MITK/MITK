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


#ifndef BERRYQMODELINDEXOBJECT_H_
#define BERRYQMODELINDEXOBJECT_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <QModelIndex>

namespace berry
{

class QModelIndexObject : public Object
{

public:

  berryObjectMacro(berry::QModelIndexObject)

  QModelIndexObject(const QModelIndex& index);

  const QModelIndex& GetQModelIndex() const;

  bool operator==(const Object* obj) const;

private:

  QModelIndex m_QModelIndex;

};

}

#endif /* BERRYQMODELINDEXOBJECT_H_ */
