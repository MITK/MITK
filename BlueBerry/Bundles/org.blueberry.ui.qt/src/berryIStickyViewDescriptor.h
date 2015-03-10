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


#ifndef BERRYISTICKYVIEWDESCRIPTOR_H_
#define BERRYISTICKYVIEWDESCRIPTOR_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * Supplemental view interface that describes various sticky characteristics
 * that a view may possess.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see org.eclipse.ui.views.IViewRegistry
 * @see org.eclipse.ui.views.IViewDescriptor
 * @since 3.1
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IStickyViewDescriptor : public Object {

  berryObjectMacro(IStickyViewDescriptor);

  ~IStickyViewDescriptor();

  /**
   * Return the id of the view to be made sticky.
   *
   * @return the id of the view to be made sticky
   */
  virtual QString GetId() const = 0;

  /**
   * Return the location of this sticky view.  Must be one of
   * <code>IPageLayout.LEFT</code>, <code>IPageLayout.RIGHT</code>,
   * <code>IPageLayout.TOP</code>, or <code>IPageLayout.BOTTOM</code>.
   *
   * @return the location constant
   */
  virtual int GetLocation() const = 0;

  /**
   * Return whether this view should be closeable.
   *
   * @return whether this view should be closeeable
   */
  virtual bool IsCloseable() const = 0;

  /**
   * Return whether this view should be moveable.
   *
   * @return whether this view should be moveable
   */
  virtual bool IsMoveable() const = 0;
};

}

#endif /* BERRYISTICKYVIEWDESCRIPTOR_H_ */
