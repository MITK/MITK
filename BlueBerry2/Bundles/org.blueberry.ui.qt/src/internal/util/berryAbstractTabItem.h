/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYABSTRACTTABITEM_H_
#define BERRYABSTRACTTABITEM_H_

#include <QRect>
#include <osgi/framework/Object.h>

#include "berryPartInfo.h"

namespace berry {

struct AbstractTabItem {

    virtual ~AbstractTabItem() {}

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
