#ifndef BERRYIWORKBENCHPARTCONSTANTS_H_
#define BERRYIWORKBENCHPARTCONSTANTS_H_

#include <string>

#include "berryUiDll.h"

namespace berry
{

/**
 * This interface describes the constants used for <link>IWorkbenchPart</link> properties.
 * <p>
 * <b>Note:</b>This interface should not be implemented or extended.
 * </p>
 *
 * @since 3.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IWorkbenchPartConstants {

  /**
   * Used in a PropertyChangeEvent as the property string to signal
   * that integer flags are used.
   */
  static const std::string INTEGER_PROPERTY;

    /**
     * The property id for <code>getTitle</code>, <code>getTitleImage</code>
     * and <code>getTitleToolTip</code>.
     */
    static const int PROP_TITLE;

    /**
     * The property id for <code>ISaveablePart.isDirty()</code>.
     */
    static const int PROP_DIRTY;

    /**
     * The property id for <code>IEditorPart.getEditorInput()</code>.
     */
    static const int PROP_INPUT;

    /**
     * The property id for <code>IWorkbenchPart2.getPartName</code>
     */
    static const int PROP_PART_NAME;

    /**
     * The property id for <code>IWorkbenchPart2.getContentDescription()</code>
     */
    static const int PROP_CONTENT_DESCRIPTION;

    /**
     * The property id for any method on the optional <code>ISizeProvider</code> interface
     * @since 3.4
     */
    static const int PROP_PREFERRED_SIZE;

    /**
      * Indicates that the underlying part was created
      */
    static const int PROP_OPENED; // = 0x211;

     /**
      * Internal property ID: Indicates that the underlying part was destroyed
      */
    static const int PROP_CLOSED; // = 0x212;

     /**
      * Internal property ID: Indicates that the result of IEditorReference.isPinned()
      */
    static const int PROP_PINNED; // = 0x213;

     /**
      * Internal property ID: Indicates that the result of getVisible() has changed
      */
    static const int PROP_VISIBLE; // = 0x214;

     /**
      * Internal property ID: Indicates that the result of isZoomed() has changed
      */
    //static const int PROP_ZOOMED = 0x215;

     /**
      * Internal property ID: Indicates that the part has an active child and the
      * active child has changed. (fired by PartStack)
      */
    static const int PROP_ACTIVE_CHILD_CHANGED; // = 0x216;

     /**
      * Internal property ID: Indicates that changed in the min / max
      * state has changed
      */
    //static final int PROP_MAXIMIZED = 0x217;

};

}

#endif /*BERRYIWORKBENCHPARTCONSTANTS_H_*/
