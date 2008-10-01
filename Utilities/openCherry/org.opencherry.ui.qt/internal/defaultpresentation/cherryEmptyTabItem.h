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


#ifndef CHERRYEMPTYTABITEM_H_
#define CHERRYEMPTYTABITEM_H_

#include "../util/cherryAbstractTabItem.h"

namespace cherry {

struct EmptyTabItem : public AbstractTabItem {

    /* (non-Javadoc)
     * @see AbstractTabItem#getBounds()
     */
    QRect GetBounds();

    /* (non-Javadoc)
     * @see AbstractTabItem#setInfo(org.opencherry.ui.internal.presentations.util.PartInfo)
     */
    void SetInfo(const PartInfo& info);

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.AbstractTabItem#getData()
     */
    Object::Pointer GetData();

    /* (non-Javadoc)
     * @see org.opencherry.ui.internal.presentations.util.AbstractTabItem#setData(java.lang.Object)
     */
    void SetData(Object::Pointer data);
};

}

#endif /* CHERRYEMPTYTABITEM_H_ */
