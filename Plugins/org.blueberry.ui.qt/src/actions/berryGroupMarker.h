/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYGROUPMARKER_H
#define BERRYGROUPMARKER_H

#include <internal/berryAbstractGroupMarker.h>

namespace berry {

/**
 * A group marker is a special kind of contribution item denoting
 * the beginning of a group. These groups are used to structure
 * the list of items. Unlike regular contribution items and
 * separators, group markers have no visual representation.
 * The name of the group is synonymous with the contribution item id.
 * <p>
 * This class may be instantiated; it is not intended to be
 * subclassed outside the framework.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.
 */
class BERRY_UI_QT GroupMarker : public AbstractGroupMarker
{

public:

    /**
     * Create a new group marker with the given name.
     * The group name must not be <code>null</code> or the empty string.
     * The group name is also used as the item id.
     *
     * @param groupName the name of the group
     */
    GroupMarker(const QString& groupName);

    /**
     * The <code>GroupMarker</code> implementation of this method
     * returns <code>false</code> since group markers are always invisible.
     */
    bool IsVisible() const override;
};

}
#endif // BERRYGROUPMARKER_H
