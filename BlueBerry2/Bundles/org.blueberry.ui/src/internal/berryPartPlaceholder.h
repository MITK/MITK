/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYPARTPLACEHOLDER_H_
#define BERRYPARTPLACEHOLDER_H_

#include "berryStackablePart.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 * A PlaceHolder is a non-visible stand-in for a layout part.
 */
class PartPlaceholder : public StackablePart {

public:

  osgiObjectMacro(PartPlaceholder);

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
