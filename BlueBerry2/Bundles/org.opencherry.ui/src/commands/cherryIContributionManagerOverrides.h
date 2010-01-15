/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYICONTRIBUTIONMANAGEROVERRIDES_H_
#define CHERRYICONTRIBUTIONMANAGEROVERRIDES_H_

#include <string>

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include <osgi/framework/Objects.h>
#include <osgi/framework/ObjectString.h>

#include "../cherryUiDll.h"

namespace cherry {

using namespace osgi::framework;

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
 * @since 2.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IContributionManagerOverrides : public virtual Object {

  osgiObjectMacro(IContributionManagerOverrides)

    /**
     * Id for the enabled property. Value is <code>"enabled"</code>.
     *
     * @since 2.0
     */
    const static std::string P_ENABLED; // = "enabled";

    /**
     * Find out the enablement of the item
     * @param item the contribution item for which the enable override value is
     * determined
     * @return <ul>
     *        <li><code>Boolean.TRUE</code> if the given contribution item should be enabled</li>
     *        <li><code>Boolean.FALSE</code> if the item should be disabled</li>
     *        <li><code>null</code> if the item may determine its own enablement</li>
     *      </ul>
     * @since 2.0
     */
    virtual ObjectBool::Pointer GetEnabled(SmartPointer<IContributionItem> item) = 0;

    /**
     * This is not intended to be called outside of the workbench. This method
     * is intended to be deprecated in 3.1.
     *
     * TODO deprecate for 3.1
     * @param item the contribution item for which the accelerator value is determined
     * @return the accelerator
     */
    virtual ObjectInt::Pointer GetAccelerator(SmartPointer<IContributionItem> item) = 0;

    /**
     * This is not intended to be called outside of the workbench. This method
     * is intended to be deprecated in 3.1.
     *
     * TODO deprecate for 3.1
     * @param item the contribution item for which the accelerator text is determined
     * @return the text for the accelerator
     */
    virtual ObjectString::Pointer GetAcceleratorText(SmartPointer<IContributionItem> item) = 0;

    /**
     * This is not intended to be called outside of the workbench. This method
     * is intended to be deprecated in 3.1.
     *
     * TODO deprecate for 3.1
     * @param item the contribution item for which the text is determined
     * @return the text
     */
    virtual ObjectString::Pointer GetText(SmartPointer<IContributionItem> item) = 0;
};

}

#endif /* CHERRYICONTRIBUTIONMANAGEROVERRIDES_H_ */
