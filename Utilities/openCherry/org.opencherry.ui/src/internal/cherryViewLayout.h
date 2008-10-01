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

namespace cherry
{

/**
 * \ingroup org_opencherry_ui_internal
 *
 * Implementation of IViewLayout.
 * This is an API facade on the internal ViewLayoutRec.
 *
 * @since 3.0
 */
class ViewLayout: public IViewLayout
{

private:

  ViewLayoutRec::Pointer rec;

public:

  ViewLayout(PageLayout::Pointer pageLayout, ViewLayoutRec::Pointer rec);

  /* (non-Javadoc)
   * @see org.opencherry.ui.IViewLayout#getShowTitle()
   */
  bool GetShowTitle();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IViewLayout#isCloseable()
   */
  bool IsCloseable();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IViewLayout#isMoveable()
   */
  bool IsMoveable();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IViewLayout#isStandalone()
   */
  bool IsStandalone();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IViewLayout#setCloseable(boolean)
   */
  void SetCloseable(bool closeable);

  /* (non-Javadoc)
   * @see org.opencherry.ui.IViewLayout#setMoveable(boolean)
   */
  void SetMoveable(bool moveable);

};

}

#endif /*CHERRYVIEWLAYOUT_H_*/
