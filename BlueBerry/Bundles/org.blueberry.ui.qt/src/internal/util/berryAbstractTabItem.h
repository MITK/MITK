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
