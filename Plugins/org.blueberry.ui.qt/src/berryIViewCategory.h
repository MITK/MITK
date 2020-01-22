/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  berryObjectMacro(berry::IViewCategory);

  virtual QString GetId() const = 0;
  virtual QString GetLabel() const = 0;
  virtual QStringList GetPath() const = 0;
  virtual QList<IViewDescriptor::Pointer> GetViews() const = 0;

  ~IViewCategory() override;
};

}

#endif /*BERRYIVIEWCATEGORY_H_*/
