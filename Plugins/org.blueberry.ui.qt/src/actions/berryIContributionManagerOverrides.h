/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYICONTRIBUTIONMANAGEROVERRIDES_H
#define BERRYICONTRIBUTIONMANAGEROVERRIDES_H

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct IContributionItem;

/**
 * This interface is used by instances of <code>IContributionItem</code>
 * to determine if the values for certain properties have been overriden
 * by their manager.
 * <p>
 * This interface is internal to the framework; it should not be implemented outside
 * the framework.
 * </p>
 *
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IContributionManagerOverrides : virtual Object
{

  berryObjectMacro(berry::IContributionManagerOverrides);

  /**
   * Id for the enabled property. Value is <code>"enabled"</code>.
   */
  static const QString P_ENABLED;

  /**
   * Find out the enablement of the item
   * @param item the contribution item for which the enable override value is
   * determined
   * @param defaultValue the default value
   * @return <ul>
   *           <li><code>1</code> if the given contribution item should be enabled</li>
   *           <li><code>0</code> if the item should not be enabled</li>
   *           <li><code>-1</code> if the item may determine its own enablement</li>
   *         </ul>
   */
  virtual int GetEnabled(const IContributionItem* item) const = 0;

  /**
   * Visiblity override.
   *
   * @param item the contribution item in question
   * @return <ul>
   *           <li><code>1</code> if the given contribution item should be visible</li>
   *           <li><code>0</code> if the item should not be visible</li>
   *           <li><code>-1</code> if the item may determine its own visibility</li>
   *         </ul>
   */
  virtual int GetVisible(const IContributionItem* item) const = 0;
};

}

#endif // BERRYICONTRIBUTIONMANAGEROVERRIDES_H
