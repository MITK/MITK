/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYIVIEWCATEGORY_H_
#define BERRYIVIEWCATEGORY_H_

#include "berryIViewDescriptor.h"

#include <berryMacros.h>


namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 */
struct BERRY_UI_QT IViewCategory : public virtual Object
{
  berryObjectMacro(berry::IViewCategory)

  virtual QString GetId() const = 0;
  virtual QString GetLabel() const = 0;
  virtual QStringList GetPath() const = 0;
  virtual QList<IViewDescriptor::Pointer> GetViews() const = 0;

  virtual ~IViewCategory();
};

}

#endif /*BERRYIVIEWCATEGORY_H_*/
