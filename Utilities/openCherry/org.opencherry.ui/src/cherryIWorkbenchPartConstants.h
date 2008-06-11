#ifndef CHERRYIWORKBENCHPARTCONSTANTS_H_
#define CHERRYIWORKBENCHPARTCONSTANTS_H_

namespace cherry
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
struct IWorkbenchPartConstants {

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
    
};

}

#endif /*CHERRYIWORKBENCHPARTCONSTANTS_H_*/
