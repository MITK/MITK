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
    bool IsGroupMarker() const;
};

}

#endif // BERRYABSTRACTGROUPMARKER_H
