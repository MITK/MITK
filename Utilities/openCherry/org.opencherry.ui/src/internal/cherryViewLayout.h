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

#ifndef CHERRYVIEWLAYOUT_H_
#define CHERRYVIEWLAYOUT_H_

#include "cherryPageLayout.h"
#include "cherryViewLayoutRec.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * Implementation of IViewLayout.
 * This is an API facade on the internal ViewLayoutRec.
 *  
 * @since 3.0
 */
class ViewLayout : public IViewLayout {
  
private:
  
  ViewLayoutRec rec;

public:
  
  ViewLayout(PageLayout* pageLayout, ViewLayoutRec r)
  : rec(r) {
        poco_check_ptr(pageLayout);
        //Assert.isNotNull(rec);
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.IViewLayout#getShowTitle()
     */
    bool GetShowTitle() {
        return rec.showTitle;
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.IViewLayout#isCloseable()
     */
    bool IsCloseable() {
        return rec.isCloseable;
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.IViewLayout#isMoveable()
     */
    bool IsMoveable() {
        return rec.isMoveable;
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.IViewLayout#isStandalone()
     */
    bool IsStandalone() {
        return rec.isStandalone;
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.IViewLayout#setCloseable(boolean)
     */
    void SetCloseable(boolean closeable) {
        rec.isCloseable = closeable;
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.IViewLayout#setMoveable(boolean)
     */
    void SetMoveable(boolean moveable) {
        rec.isMoveable = moveable;
    }

};

}

#endif /*CHERRYVIEWLAYOUT_H_*/
