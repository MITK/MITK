/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYABSTRACTTABITEM_H_
#define BERRYABSTRACTTABITEM_H_

#include <QRect>
#include <berryObject.h>

#include "berryPartInfo.h"

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct BERRY_UI_QT AbstractTabItem {

    virtual ~AbstractTabItem();

    virtual QRect GetBounds() = 0;
    virtual void SetInfo(const PartInfo& info) = 0;
    virtual void Dispose() = 0;

    virtual void SetBusy(bool  /*busy*/) {}
    virtual void SetBold(bool  /*bold*/) {}

    virtual Object::Pointer GetData() = 0;
    virtual void SetData(Object::Pointer data) = 0;

    virtual bool IsShowing() { return true; }

};

}


#endif /* BERRYABSTRACTTABITEM_H_ */
