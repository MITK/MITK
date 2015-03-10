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

#ifndef BERRYLAYOUTPART_H_
#define BERRYLAYOUTPART_H_

#include <berryMacros.h>

#include "berryIDropTarget.h"

#include "berryISizeProvider.h"
#include "berryShell.h"

namespace berry {

struct ILayoutContainer;
struct IWorkbenchWindow;

/**
 * \ingroup org_blueberry_ui_internal
 *
 * A presentation part is used to build the presentation for the
 * workbench.  Common subclasses are pane and folder.
 */
class LayoutPart : virtual public Object, public virtual ISizeProvider
{

public:

  berryObjectMacro(LayoutPart)

protected: ILayoutContainer* container;

    protected: QString id;

    public: static const QString PROP_VISIBILITY;// = "PROP_VISIBILITY";

    /**
     * Number of times deferUpdates(true) has been called without a corresponding
     * deferUpdates(false)
     */
  private: int deferCount;

    /**
     * PresentationPart constructor comment.
     */
    public: LayoutPart(const QString& id);

    public: virtual ~LayoutPart();

    /**
     * When a layout part closes, focus will return to a previously active part.
     * This method determines whether this part should be considered for activation
     * when another part closes. If a group of parts are all closing at the same time,
     * they will all return false from this method while closing to ensure that the
     * parent does not activate a part that is in the process of closing. Parts will
     * also return false from this method if they are minimized, closed fast views,
     * obscured by zoom, etc.
     *
     * @return true iff the parts in this container may be given focus when the active
     * part is closed
     */
    public: virtual bool AllowsAutoFocus();


    /**
     * Creates the GUI control
     */
    public: virtual void CreateControl(QWidget* parent) = 0;

    /**
     * Disposes the GUI control
     *
     * This can be used to execute cleanup code or notify listeners
     * when a LayoutPart is no longer used, but is still referenced
     * by a SmartPointer (instead of putting the code in the LayoutPart
     * destructor).
     */
    public: virtual void Dispose();

    /**
     * Gets the presentation bounds.
     */
    public: QRect GetBounds();


    /**
     * Gets the parent for this part.
     * <p>
     * In general, this is non-null if the object has been added to a container and the
     * container's widgetry exists. The exception to this rule is PartPlaceholders
     * created when restoring a ViewStack using restoreState, which point to the
     * ViewStack even if its widgetry doesn't exist yet. Returns null in the remaining
     * cases.
     * </p>
     * <p>
     * TODO: change the semantics of this method to always point to the parent container,
     * regardless of whether its widgetry exists. Locate and refactor code that is currently
     * depending on the special cases.
     * </p>
     */
    public: virtual SmartPointer<ILayoutContainer> GetContainer();

    /**
     * Get the part control.  This method may return null.
     */
    public: virtual QWidget* GetControl() = 0;

    public: virtual bool IsPlaceHolder() const;

    /**
     * Gets the ID for this part.
     */
    public: virtual QString GetID() const;

    public: virtual bool IsCompressible();

    /**
     * Gets the presentation size.
     */
    public: virtual QSize GetSize();

    /**
     * @see org.blueberry.ui.presentations.StackPresentation#getSizeFlags(boolean)
     *
     * @since 3.1
     */
    public: virtual int GetSizeFlags(bool horizontal);

    /**
     * @see org.blueberry.ui.presentations.StackPresentation#computePreferredSize(boolean, int, int, int)
     *
     * @since 3.1
     */
    public: virtual int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredParallel);

    public: virtual IDropTarget::Pointer GetDropTarget(Object::Pointer draggedObject, const QPoint& displayCoordinates);

    public: bool IsDocked();

    public: virtual Shell::Pointer GetShell();

    /**
   * Returns the workbench window window for a part.
   *
   * @return the workbench window, or <code>null</code> if there's no window
   *         associated with this part.
   */
    public: virtual SmartPointer<IWorkbenchWindow> GetWorkbenchWindow();

    /**
     * Move the control over another one.
     */
    public: virtual void MoveAbove(QWidget* refControl);

    /**
     * Reparent a part.
     */
    public: virtual void Reparent(QWidget* newParent);

    /**
     * Returns true if this part was set visible. This returns whatever was last passed into
     * setVisible, but does not necessarily indicate that the part can be seen (ie: one of its
     * ancestors may be invisible)
     */
    public: virtual bool GetVisible();

    /**
     * Returns true if this part can be seen. Returns false if the part or any of its ancestors
     * are invisible.
     */
    public: virtual bool IsVisible();

    /**
     * Shows the receiver if <code>visible</code> is true otherwise hide it.
     */
    public: virtual void SetVisible(bool makeVisible);

    /**
     * Returns <code>true</code> if the given control or any of its descendents has focus.
     */
    private: virtual bool IsFocusAncestor(QWidget* ctrl);

    /**
     * Sets the presentation bounds.
     */
    public: virtual void SetBounds(const QRect& r);

    /**
     * Sets the parent for this part.
     */
    public: virtual void SetContainer(SmartPointer<ILayoutContainer> container);

    /**
     * Sets focus to this part.
     */
    public: virtual void SetFocus();

    /**
     * Sets the part ID.
     */
    public: virtual void SetID(const QString& str);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.IWorkbenchDragDropPart#getPart()
     */
    public: virtual LayoutPart::Pointer GetPart();


    /**
     * deferUpdates(true) disables widget updates until a corresponding call to
     * deferUpdates(false). Exactly what gets deferred is the decision
     * of each LayoutPart, however the part may only defer operations in a manner
     * that does not affect the final result.
     * That is, the state of the receiver after the final call to deferUpdates(false)
     * must be exactly the same as it would have been if nothing had been deferred.
     *
     * @param shouldDefer true iff events should be deferred
     */
    public: void DeferUpdates(bool shouldDefer);

    /**
     * This is called when deferUpdates(true) causes UI events for this
     * part to be deferred. Subclasses can overload to initialize any data
     * structures that they will use to collect deferred events.
     */
    protected: virtual void StartDeferringEvents();

    /**
     * Immediately processes all UI events which were deferred due to a call to
     * deferUpdates(true). This is called when the last call is made to
     * deferUpdates(false). Subclasses should overload this method if they
     * defer some or all UI processing during deferUpdates.
     */
    protected: virtual void HandleDeferredEvents();

    /**
     * Subclasses can call this method to determine whether UI updates should
     * be deferred. Returns true iff there have been any calls to deferUpdates(true)
     * without a corresponding call to deferUpdates(false). Any operation which is
     * deferred based on the result of this method should be performed later within
     * handleDeferredEvents().
     *
     * @return true iff updates should be deferred.
     */
    protected: bool IsDeferred();

    /**
     * Writes a description of the layout to the given string buffer.
     * This is used for drag-drop test suites to determine if two layouts are the
     * same. Like a hash code, the description should compare as equal iff the
     * layouts are the same. However, it should be user-readable in order to
     * help debug failed tests. Although these are english readable strings,
     * they do not need to be translated.
     *
     * @param buf
     */
    public: virtual void DescribeLayout(QString& buf) const;

    /**
     * Returns an id representing this part, suitable for use in a placeholder.
     *
     * @since 3.0
     */
    public: virtual QString GetPlaceHolderId();

    public: virtual void ResizeChild(LayoutPart::Pointer childThatChanged);

    public: void FlushLayout();

    /**
     * Returns true iff the given part can be added to this ILayoutContainer
     * @param toAdd
     * @return
     * @since 3.1
     */
    public: virtual bool AllowsAdd(LayoutPart::Pointer toAdd);

    /**
     * Tests the integrity of this object. Throws an exception if the object's state
     * is not internally consistent. For use in test suites.
     */
    public: virtual void TestInvariants();

    public: virtual QString ToString() const;
};

}

#endif /*BERRYLAYOUTPART_H_*/
