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

#ifndef BERRYPARTPLACEHOLDER_H_
#define BERRYPARTPLACEHOLDER_H_

#include "berryLayoutPart.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 * A PlaceHolder is a non-visible stand-in for a layout part.
 */
class PartPlaceholder : public LayoutPart {

public:

  berryObjectMacro(PartPlaceholder);

    /**
     * Placeholder ids may contain wildcards.  This is the wildcard string.
     *
     * @since 3.0
     */
    static const std::string WILD_CARD; // = "*"; //$NON-NLS-1$

    PartPlaceholder(const std::string& id);

    /**
     * Creates the SWT control
     */
    void CreateControl(void* parent);

    /**
     * Get the part control.  This method may return null.
     */
    void* GetControl();

    /**
     * Returns whether this placeholder has a wildcard.
     *
     * @since 3.0
     */
    bool HasWildCard();

    bool IsPlaceHolder() const;
};

}

#endif /*BERRYPARTPLACEHOLDER_H_*/
