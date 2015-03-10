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

#ifndef BERRYVIEWLAYOUT_H_
#define BERRYVIEWLAYOUT_H_

#include "berryPageLayout.h"
#include "berryViewLayoutRec.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
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
   * @see org.blueberry.ui.IViewLayout#getShowTitle()
   */
  bool GetShowTitle();

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#isCloseable()
   */
  bool IsCloseable();

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#isMoveable()
   */
  bool IsMoveable();

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#isStandalone()
   */
  bool IsStandalone();

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#setCloseable(boolean)
   */
  void SetCloseable(bool closeable);

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#setMoveable(boolean)
   */
  void SetMoveable(bool moveable);

};

}

#endif /*BERRYVIEWLAYOUT_H_*/
