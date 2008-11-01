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


#ifndef CHERRYSHELL_H_
#define CHERRYSHELL_H_

#include <cherryMacros.h>

#include "cherryUiDll.h"
#include "cherryIShellListener.h"
#include "guitk/cherryGuiTkIControlListener.h"
#include "cherryRectangle.h"
#include "cherryPoint.h"

#include <vector>

namespace cherry {

/**
 * Instances of this class represent the "windows"
 * which the desktop or "window manager" is managing.
 * Instances that do not have a parent (that is, they
 * are built using the constructor, which takes a
 * <code>Display</code> as the argument) are described
 * as <em>top level</em> shells. Instances that do have
 * a parent are described as <em>secondary</em> or
 * <em>dialog</em> shells.
 * <p>
 * Instances are always displayed in one of the maximized,
 * minimized or normal states:
 * <ul>
 * <li>
 * When an instance is marked as <em>maximized</em>, the
 * window manager will typically resize it to fill the
 * entire visible area of the display, and the instance
 * is usually put in a state where it can not be resized
 * (even if it has style <code>RESIZE</code>) until it is
 * no longer maximized.
 * </li><li>
 * When an instance is in the <em>normal</em> state (neither
 * maximized or minimized), its appearance is controlled by
 * the style constants which were specified when it was created
 * and the restrictions of the window manager (see below).
 * </li><li>
 * When an instance has been marked as <em>minimized</em>,
 * its contents (client area) will usually not be visible,
 * and depending on the window manager, it may be
 * "iconified" (that is, replaced on the desktop by a small
 * simplified representation of itself), relocated to a
 * distinguished area of the screen, or hidden. Combinations
 * of these changes are also possible.
 * </li>
 * </ul>
 * </p><p>
 * The <em>modality</em> of an instance may be specified using
 * style bits. The modality style bits are used to determine
 * whether input is blocked for other shells on the display.
 * The <code>PRIMARY_MODAL</code> style allows an instance to block
 * input to its parent. The <code>APPLICATION_MODAL</code> style
 * allows an instance to block input to every other shell in the
 * display. The <code>SYSTEM_MODAL</code> style allows an instance
 * to block input to all shells, including shells belonging to
 * different applications.
 * </p><p>
 * Note: The styles supported by this class are treated
 * as <em>HINT</em>s, since the window manager for the
 * desktop on which the instance is visible has ultimate
 * control over the appearance and behavior of decorations
 * and modality. For example, some window managers only
 * support resizable windows and will always assume the
 * RESIZE style, even if it is not set. In addition, if a
 * modality style is not supported, it is "upgraded" to a
 * more restrictive modality style that is supported. For
 * example, if <code>PRIMARY_MODAL</code> is not supported,
 * it would be upgraded to <code>APPLICATION_MODAL</code>.
 * A modality style may also be "downgraded" to a less
 * restrictive style. For example, most operating systems
 * no longer support <code>SYSTEM_MODAL</code> because
 * it can freeze up the desktop, so this is typically
 * downgraded to <code>APPLICATION_MODAL</code>.
 * <dl>
 * <dt><b>Styles:</b></dt>
 * <dd>BORDER, CLOSE, MIN, MAX, NO_TRIM, RESIZE, TITLE, ON_TOP, TOOL</dd>
 * <dd>APPLICATION_MODAL, MODELESS, PRIMARY_MODAL, SYSTEM_MODAL</dd>
 * <dt><b>Events:</b></dt>
 * <dd>Activate, Close, Deactivate, Deiconify, Iconify</dd>
 * </dl>
 * Class <code>SWT</code> provides two "convenience constants"
 * for the most commonly required style combinations:
 * <dl>
 * <dt><code>SHELL_TRIM</code></dt>
 * <dd>
 * the result of combining the constants which are required
 * to produce a typical application top level shell: (that
 * is, <code>CLOSE | TITLE | MIN | MAX | RESIZE</code>)
 * </dd>
 * <dt><code>DIALOG_TRIM</code></dt>
 * <dd>
 * the result of combining the constants which are required
 * to produce a typical application dialog shell: (that
 * is, <code>TITLE | CLOSE | BORDER</code>)
 * </dd>
 * </dl>
 * </p>
 * <p>
 * Note: Only one of the styles APPLICATION_MODAL, MODELESS,
 * PRIMARY_MODAL and SYSTEM_MODAL may be specified.
 * </p><p>
 * IMPORTANT: This class is not intended to be subclassed.
 * </p>
 *
 * @see Decorations
 * @see SWT
 * @see <a href="http://www.opencherry.org/swt/snippets/#shell">Shell snippets</a>
 * @see <a href="http://www.opencherry.org/swt/examples.php">SWT Example: ControlExample</a>
 * @see <a href="http://www.opencherry.org/swt/">Sample code and further information</a>
 */
class CHERRY_UI Shell : public virtual Object
{
public:

  cherryClassMacro(Shell);

  Object::Pointer GetData() const;
  void SetData(Object::Pointer data);

  void SetBounds(int x, int y, int width, int height);

  virtual void SetBounds(const Rectangle& bounds) = 0;
  virtual Rectangle GetBounds() const = 0;

  virtual void SetLocation(int x, int y) = 0;

  virtual Point ComputeSize(int wHint, int hHint, bool changed) = 0;

  virtual void SetText(const std::string& text) = 0;

  virtual bool IsVisible() = 0;
  virtual void SetVisible(bool visible) = 0;

  virtual void* GetControl() = 0;

  virtual void SetImages(const std::vector<void*>& images) = 0;

  /**
   * Returns <code>true</code> if the receiver is currently
   * maximized, and false otherwise.
   * <p>
   *
   * @return the maximized state
   *
   * @see #SetMaximized
   */
  virtual bool GetMaximized() = 0;

  /**
   * Returns <code>true</code> if the receiver is currently
   * minimized, and false otherwise.
   * <p>
   *
   * @return the minimized state
   *
   * @see #SetMinimized
   */
  virtual bool GetMinimized() = 0;

  /**
   * Sets the minimized stated of the receiver.
   * If the argument is <code>true</code> causes the receiver
   * to switch to the minimized state, and if the argument is
   * <code>false</code> and the receiver was previously minimized,
   * causes the receiver to switch back to either the maximized
   * or normal states.
   * <p>
   * Note: The result of intermixing calls to <code>setMaximized(true)</code>
   * and <code>setMinimized(true)</code> will vary by platform. Typically,
   * the behavior will match the platform user's expectations, but not
   * always. This should be avoided if possible.
   * </p>
   *
   * @param minimized the new maximized state
   *
   * @see #SetMaximized
   */
  virtual void SetMinimized(bool minimized) = 0;

  /**
   * Sets the maximized state of the receiver.
   * If the argument is <code>true</code> causes the receiver
   * to switch to the maximized state, and if the argument is
   * <code>false</code> and the receiver was previously maximized,
   * causes the receiver to switch back to either the minimized
   * or normal states.
   * <p>
   * Note: The result of intermixing calls to <code>setMaximized(true)</code>
   * and <code>setMinimized(true)</code> will vary by platform. Typically,
   * the behavior will match the platform user's expectations, but not
   * always. This should be avoided if possible.
   * </p>
   *
   * @param maximized the new maximized state
   *
   * @see #SetMinimized
   */
  virtual void SetMaximized(bool maximized) = 0;

  /**
   * Adds the listener to the collection of listeners who will
   * be notified when operations are performed on the receiver,
   * by sending the listener one of the messages defined in the
   * <code>IShellListener</code> interface.
   *
   * @param listener the listener which should be notified
   *
   * @see IShellListener
   * @see #RemoveShellListener
   */
  virtual void AddShellListener(IShellListener::Pointer listener) = 0;

  /**
   * Removes the listener from the collection of listeners who will
   * be notified when operations are performed on the receiver.
   *
   * @param listener the listener which should no longer be notified
   *
   * @see IShellListener
   * @see #AddShellListener
   */
  virtual void RemoveShellListener(IShellListener::Pointer listener) = 0;

  /**
   * Adds the listener to the collection of listeners who will
   * be notified when the shell is moved or resized, by sending
   * it one of the messages defined in the <code>IControlListener</code>
   * interface.
   *
   * @param listener the listener which should be notified
   *
   * @see IControlListener
   * @see #RemoveControlListener
   */
  virtual void AddControlListener(GuiTk::IControlListener::Pointer listener) = 0;

  /**
   * Removes the listener from the collection of listeners who will
   * be notified when the shell is moved or resized.
   *
   * @param listener the listener which should no longer be notified
   *
   * @see IControlListener
   * @see #AddControlListener
   */
  virtual void RemoveControlListener (GuiTk::IControlListener::Pointer listener) = 0;

  /**
   * Moves the receiver to the top of the drawing order for
   * the display on which it was created (so that all other
   * shells on that display, which are not the receiver's
   * children will be drawn behind it), marks it visible,
   * sets the focus and asks the window manager to make the
   * shell active.
   */
  virtual void Open(bool block = false) = 0;

  /**
   * Requests that the window manager close the receiver in
   * the same way it would be closed when the user clicks on
   * the "close box" or performs some other platform specific
   * key or mouse combination that indicates the window
   * should be removed.
   */
  virtual void Close() = 0;

  /**
    * Returns an array containing all shells which are
    * descendants of the receiver.
    * <p>
    * @return the dialog shells
    *
    * @exception SWTException <ul>
    *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
    *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
    * </ul>
    */
   virtual std::vector<Shell::Pointer> GetShells() = 0;

   /**
     * Returns the receiver's style information.
     * <p>
     * Note that the value which is returned by this method <em>may
     * not match</em> the value which was provided to the constructor
     * when the receiver was created. This can occur when the underlying
     * operating system does not support a particular combination of
     * requested styles. For example, if the platform widget used to
     * implement a particular SWT widget always has scroll bars, the
     * result of calling this method would always have the
     * <code>SWT.H_SCROLL</code> and <code>SWT.V_SCROLL</code> bits set.
     * </p>
     *
     * @return the style bits
     *
     * @exception SWTException <ul>
     *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
     *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
     * </ul>
     */
    virtual int GetStyle () = 0;

private:

  Object::Pointer data;

};

}

#endif /* CHERRYSHELL_H_ */
