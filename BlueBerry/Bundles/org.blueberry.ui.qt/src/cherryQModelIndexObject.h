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


#ifndef CHERRYQMODELINDEXOBJECT_H_
#define CHERRYQMODELINDEXOBJECT_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include <QModelIndex>

namespace cherry
{

class QModelIndexObject : public Object
{

public:

  cherryObjectMacro(cherry::QModelIndexObject)

  QModelIndexObject(const QModelIndex& index);

  const QModelIndex& GetQModelIndex() const;

  bool operator==(const Object* obj) const;

private:

  QModelIndex m_QModelIndex;

};

}

#endif /* CHERRYQMODELINDEXOBJECT_H_ */
