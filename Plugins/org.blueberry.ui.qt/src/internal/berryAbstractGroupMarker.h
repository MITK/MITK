/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYABSTRACTGROUPMARKER_H
#define BERRYABSTRACTGROUPMARKER_H

#include "berryContributionItem.h"

namespace berry {

/**
 * Abstract superclass for group marker classes.
 * <p>
 * This class is not intended to be subclassed outside the framework.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.
 */
class AbstractGroupMarker : public ContributionItem
{

protected:

    /**
     * Constructor for use by subclasses.
     */
    AbstractGroupMarker();

    /**
     * Create a new group marker with the given name.
     * The group name must not be <code>null</code> or the empty string.
     * The group name is also used as the item id.
     *
     * @param groupName the name of the group
     */
    AbstractGroupMarker(const QString& groupName);

public:

    /**
     * Returns the group name.
     *
     * @return the group name
     */
    QString GetGroupName() const;

    /**
     * The <code>AbstractGroupMarker</code> implementation of this <code>IContributionItem</code>
     * method returns <code>true</code> iff the id is not <code>null</code>. Subclasses may override.
     */
    bool IsGroupMarker() const override;
};

}

#endif // BERRYABSTRACTGROUPMARKER_H
