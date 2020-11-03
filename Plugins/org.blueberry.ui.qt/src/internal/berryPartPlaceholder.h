/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    static const QString WILD_CARD; // = "*";

    PartPlaceholder(const QString& id);

    /**
     * Creates the SWT control
     */
    void CreateControl(QWidget* parent) override;

    /**
     * Get the part control.  This method may return null.
     */
    QWidget* GetControl() override;

    /**
     * Returns whether this placeholder has a wildcard.
     *
     * @since 3.0
     */
    bool HasWildCard();

    bool IsPlaceHolder() const override;
};

}

#endif /*BERRYPARTPLACEHOLDER_H_*/
