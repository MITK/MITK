/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  bool GetShowTitle() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#isCloseable()
   */
  bool IsCloseable() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#isMoveable()
   */
  bool IsMoveable() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#isStandalone()
   */
  bool IsStandalone() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#setCloseable(boolean)
   */
  void SetCloseable(bool closeable) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IViewLayout#setMoveable(boolean)
   */
  void SetMoveable(bool moveable) override;

};

}

#endif /*BERRYVIEWLAYOUT_H_*/
