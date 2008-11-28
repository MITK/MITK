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

#ifndef CHERRYWINDOW_H_
#define CHERRYWINDOW_H_

#include "cherryUiDll.h"

#include "cherryIShellProvider.h"
#include "cherryIShellListener.h"
#include "cherryPoint.h"

#include "internal/cherryWindowManager.h"

#include <cherryMacros.h>
#include <vector>

namespace cherry
{

/**
 * A JFace window is an object that has no visual representation (no widgets)
 * until it is told to open.
 * <p>
 * Creating a window involves the following steps:
 * <ul>
 * <li>creating an instance of a concrete subclass of <code>Window</code>
 * </li>
 * <li>creating the window's shell and widget tree by calling
 * <code>create</code> (optional)</li>
 * <li>assigning the window to a window manager using
 * <code>WindowManager.add</code> (optional)</li>
 * <li>opening the window by calling <code>open</code></li>
 * </ul>
 * Opening the window will create its shell and widget tree if they have not
 * already been created. When the window is closed, the shell and widget tree
 * are disposed of and are no longer referenced, and the window is automatically
 * removed from its window manager. A window may be reopened.
 * </p>
 * <p>
 * The JFace window framework (this package) consists of this class,
 * <code>Window</code>, the abstract base of all windows, and one concrete
 * window classes (<code>ApplicationWindow</code>) which may also be
 * subclassed. Clients may define additional window subclasses as required.
 * </p>
 * <p>
 * The <code>Window</code> class provides methods that subclasses may
 * override to configure the window, including:
 * <ul>
 * <li><code>close</code>- extend to free other SWT resources</li>
 * <li><code>configureShell</code>- extend or reimplement to set shell
 * properties before window opens</li>
 * <li><code>createContents</code>- extend or reimplement to create controls
 * before window opens</li>
 * <li><code>getInitialSize</code>- reimplement to give the initial size for
 * the shell</li>
 * <li><code>getInitialLocation</code>- reimplement to give the initial
 * location for the shell</li>
 * <li><code>getShellListener</code>- extend or reimplement to receive shell
 * events</li>
 * <li><code>handleFontChange</code>- reimplement to respond to font changes
 * </li>
 * <li><code>handleShellCloseEvent</code>- extend or reimplement to handle
 * shell closings</li>
 * </ul>
 * </p>
 */
class CHERRY_UI Window: public IShellProvider
{

public:

  cherryClassMacro(Window);

  /**
   * Standard return code constant (value 0) indicating that the window was
   * opened.
   *
   * @see #open
   */
  static const int OK; // = 0;

  /**
   * Standard return code constant (value 1) indicating that the window was
   * canceled.
   *
   * @see #open
   */
  static const int CANCEL; // = 1;

  /**
   * An array of images to be used for the window. It is expected that the
   * array will contain the same icon rendered at different resolutions.
   */
  static std::vector<void*> defaultImages;

  /**
   * This interface defines a Exception Handler which can be set as a global
   * handler and will be called if an exception happens in the event loop.
   */
  struct IExceptionHandler: public Object
  {

    cherryClassMacro(IExceptionHandler)
    ;

    /**
     * Handle the exception.
     *
     * @param t
     *            The exception that occured.
     */
    virtual void HandleException(const std::exception& t) = 0;
  };

private:

  struct WindowShellListener : public IShellListener
  {
    WindowShellListener(Window* wnd);

    void ShellClosed(ShellEvent::Pointer event);

  private:
    Window* window;
  };

  /**
   * Defines a default exception handler.
   */
  struct DefaultExceptionHandler: public IExceptionHandler
  {
    /*
     * (non-Javadoc)
     *
     * @see org.opencherry.jface.window.Window.IExceptionHandler#handleException(java.lang.Throwable)
     */
    void HandleException(const std::exception& t);
  };

  /**
   * The exception handler for this application.
   */
  static IExceptionHandler::Pointer exceptionHandler;

  /**
   * Object used to locate the default parent for modal shells
   */
  struct DefaultModalParent: public IShellProvider
  {
    Shell::Pointer GetShell();
  };

  friend struct DefaultModalParent;

  static IShellProvider::Pointer defaultModalParent;

  /**
   * Object that returns the parent shell.
   */
  IShellProvider::Pointer parentShell;

  /**
   * Shell style bits.
   *
   * @see #setShellStyle
   */
  int shellStyle; // = Constants::SHELL_TRIM;

  /**
   * Window manager, or <code>null</code> if none.
   *
   * @see #setWindowManager
   */
  WindowManager* windowManager;

  /**
   * Window shell, or <code>null</code> if none.
   */
  Shell::Pointer shell;

  /**
   * Top level SWT control, or <code>null</code> if none
   */
  void* contents;

  /**
   * Window return code; initially <code>OK</code>.
   *
   * @see #setReturnCode
   */
  int returnCode; // = OK;

  /**
   * <code>true</code> if the <code>open</code> method should not return
   * until the window closes, and <code>false</code> if the
   * <code>open</code> method should return immediately; initially
   * <code>false</code> (non-blocking).
   *
   * @see #setBlockOnOpen
   */
  bool block; // = false;

  //  /**
  //   * Internal class for informing this window when fonts change.
  //   */
  //  class FontChangeListener implements IPropertyChangeListener {
  //    public void propertyChange(PropertyChangeEvent event) {
  //      handleFontChange(event);
  //    }
  //  }

  //  /**
  //   * Internal font change listener.
  //   */
  //  private FontChangeListener fontChangeListener;

  /**
   * Internal fields to detect if shell size has been set
   */
  //bool resizeHasOccurred = false;

  //Listener resizeListener;


  /**
   * Returns the most specific modal child from the given list of Shells.
   *
   * @param toSearch shells to search for modal children
   * @return the most specific modal child, or null if none
   *
   * @since 3.1
   */
  static Shell::Pointer GetModalChild(
      const std::vector<Shell::Pointer>& toSearch);

  /**
   * Runs the event loop for the given shell.
   *
   * @param loopShell
   *            the shell
   */
  //void RunEventLoop();

protected:

  /**
   * Creates a window instance, whose shell will be created under the given
   * parent shell. Note that the window will have no visual representation
   * until it is told to open. By default, <code>open</code> does not block.
   *
   * @param parentShell
   *            the parent shell, or <code>null</code> to create a top-level
   *            shell. Try passing "(Shell)null" to this method instead of "null"
   *            if your compiler complains about an ambiguity error.
   * @see #setBlockOnOpen
   * @see #getDefaultOrientation()
   */
  Window(Shell::Pointer parentShell);

  /**
   * Creates a new window which will create its shell as a child of whatever
   * the given shellProvider returns.
   *
   * @param shellProvider object that will return the current parent shell. Not null.
   *
   * @since 3.1
   */
  Window(IShellProvider::Pointer shellProvider);

  /**
   * Initializes this windows variables
   */
  virtual void Init();

  /**
   * Determines if the window should handle the close event or do nothing.
   * <p>
   * The default implementation of this framework method returns
   * <code>true</code>, which will allow the
   * <code>handleShellCloseEvent</code> method to be called. Subclasses may
   * extend or reimplement.
   * </p>
   *
   * @return whether the window should handle the close event.
   */
  virtual bool CanHandleShellCloseEvent();

  /**
   * Configures the given shell in preparation for opening this window in it.
   * <p>
   * The default implementation of this framework method sets the shell's
   * image and gives it a grid layout. Subclasses may extend or reimplement.
   * </p>
   *
   * @param newShell
   *            the shell
   */
  virtual void ConfigureShell(Shell::Pointer newShell);

  /**
   * Constrain the shell size to be no larger than the display bounds.
   *
   * @since 2.0
   */
  //void ConstrainShellSize();

  /**
   * Creates and returns this window's contents. Subclasses may attach any
   * number of children to the parent. As a convenience, the return value of
   * this method will be remembered and returned by subsequent calls to
   * getContents(). Subclasses may modify the parent's layout if they overload
   * getLayout() to return null.
   *
   * <p>
   * It is common practise to create and return a single composite that
   * contains the entire window contents.
   * </p>
   *
   * <p>
   * The default implementation of this framework method creates an instance
   * of <code>Composite</code>. Subclasses may override.
   * </p>
   *
   * @param parent
   *            the parent composite for the controls in this window. The type
   *            of layout used is determined by getLayout()
   *
   * @return the control that will be returned by subsequent calls to
   *         getContents()
   */
  virtual void* CreateContents(Shell::Pointer parent);

  /**
   * Creates and returns this window's shell.
   * <p>
   * The default implementation of this framework method creates a new shell
   * and configures it using <code/>configureShell</code>. Rather than
   * override this method, subclasses should instead override
   * <code/>configureShell</code>.
   * </p>
   *
   * @return the shell
   */
  virtual Shell::Pointer CreateShell();

  /**
   * Returns the top level control for this window. The parent of this control
   * is the shell.
   *
   * @return the top level control, or <code>null</code> if this window's
   *         control has not been created yet
   */
  virtual void* GetContents();

  /**
   * Returns the initial location to use for the shell. The default
   * implementation centers the shell horizontally (1/2 of the difference to
   * the left and 1/2 to the right) and vertically (1/3 above and 2/3 below)
   * relative to the parent shell, or display bounds if there is no parent
   * shell.
   *
   * @param initialSize
   *            the initial size of the shell, as returned by
   *            <code>getInitialSize</code>.
   * @return the initial location of the shell
   */
  virtual Point GetInitialLocation(const Point& initialSize);

  /**
   * Returns the initial size to use for the shell. The default implementation
   * returns the preferred size of the shell, using
   * <code>Shell.computeSize(SWT.DEFAULT, SWT.DEFAULT, true)</code>.
   *
   * @return the initial size of the shell
   */
  virtual Point GetInitialSize();

  /**
   * Returns parent shell, under which this window's shell is created.
   *
   * @return the parent shell, or <code>null</code> if there is no parent
   *         shell
   */
  Shell::Pointer GetParentShell();

  /**
   * Returns a shell listener. This shell listener gets registered with this
   * window's shell.
   * <p>
   * The default implementation of this framework method returns a new
   * listener that makes this window the active window for its window manager
   * (if it has one) when the shell is activated, and calls the framework
   * method <code>handleShellCloseEvent</code> when the shell is closed.
   * Subclasses may extend or reimplement.
   * </p>
   *
   * @return a shell listener
   */
  virtual IShellListener::Pointer GetShellListener();

  /**
   * Returns the shell style bits.
   * <p>
   * The default value is <code>SWT.CLOSE|SWT.MIN|SWT.MAX|SWT.RESIZE</code>.
   * Subclassers should call <code>setShellStyle</code> to change this
   * value, rather than overriding this method.
   * </p>
   *
   * @return the shell style bits
   */
  int GetShellStyle();

  /**
   * Notifies that the window's close button was pressed, the close menu was
   * selected, or the ESCAPE key pressed.
   * <p>
   * The default implementation of this framework method sets the window's
   * return code to <code>CANCEL</code> and closes the window using
   * <code>close</code>. Subclasses may extend or reimplement.
   * </p>
   */
  virtual void HandleShellCloseEvent();

  /**
   * Initializes the location and size of this window's SWT shell after it has
   * been created.
   * <p>
   * This framework method is called by the <code>create</code> framework
   * method. The default implementation calls <code>getInitialSize</code>
   * and <code>getInitialLocation</code> and passes the results to
   * <code>Shell.setBounds</code>. This is only done if the bounds of the
   * shell have not already been modified. Subclasses may extend or
   * reimplement.
   * </p>
   */
  virtual void InitializeBounds();

  /**
   * Changes the parent shell. This is only safe to use when the shell is not
   * yet realized (i.e., created). Once the shell is created, it must be
   * disposed (i.e., closed) before this method can be called.
   *
   * @param newParentShell
   *            The new parent shell; this value may be <code>null</code> if
   *            there is to be no parent.
   * @since 3.1
   */
  void SetParentShell(Shell::Pointer newParentShell);

  /**
   * Sets this window's return code. The return code is automatically returned
   * by <code>open</code> if block on open is enabled. For non-blocking
   * opens, the return code needs to be retrieved manually using
   * <code>getReturnCode</code>.
   *
   * @param code
   *            the return code
   */
  void SetReturnCode(int code);

  /**
   * Sets the shell style bits. This method has no effect after the shell is
   * created.
   * <p>
   * The shell style bits are used by the framework method
   * <code>createShell</code> when creating this window's shell.
   * </p>
   *
   * @param newShellStyle
   *            the new shell style bits
   */
  void SetShellStyle(int newShellStyle);

public:

  /**
   * Closes this window, disposes its shell, and removes this window from its
   * window manager (if it has one).
   * <p>
   * This framework method may be extended (<code>super.close</code> must
   * be called).
   * </p>
   * <p>
   *  Note that in order to prevent recursive calls to this method
   *  it does not call <code>Shell#close()</code>. As a result <code>ShellListener</code>s
   *  will not receive a <code>shellClosed</code> event.
   *  </p>
   *
   * @return <code>true</code> if the window is (or was already) closed, and
   *         <code>false</code> if it is still open
   */
  virtual bool Close();

  /**
   * Creates this window's widgetry in a new top-level shell.
   * <p>
   * The default implementation of this framework method creates this window's
   * shell (by calling <code>createShell</code>), and its controls (by
   * calling <code>createContents</code>), then initializes this window's
   * shell bounds (by calling <code>initializeBounds</code>).
   * </p>
   */
  virtual void Create();

  /**
   * Returns the default image. This is the image that will be used for
   * windows that have no shell image at the time they are opened. There is no
   * default image unless one is installed via <code>setDefaultImage</code>.
   *
   * @return the default image, or <code>null</code> if none
   * @see #setDefaultImage
   */
  static void* GetDefaultImage();

  /**
   * Returns the array of default images to use for newly opened windows. It
   * is expected that the array will contain the same icon rendered at
   * different resolutions.
   *
   * @see org.opencherry.swt.widgets.Decorations#setImages(org.opencherry.swt.graphics.Image[])
   *
   * @return the array of images to be used when a new window is opened
   * @see #setDefaultImages
   * @since 3.0
   */
  static std::vector<void*> GetDefaultImages();

  /**
   * Returns this window's return code. A window's return codes are
   * window-specific, although two standard return codes are predefined:
   * <code>OK</code> and <code>CANCEL</code>.
   *
   * @return the return code
   */
  int GetReturnCode();

  /**
   * Returns this window's shell.
   *
   * @return this window's shell, or <code>null</code> if this window's
   *         shell has not been created yet
   */
  Shell::Pointer GetShell();

  /**
   * Returns the window manager of this window.
   *
   * @return the WindowManager, or <code>null</code> if none
   */
  WindowManager* GetWindowManager();

  /**
   * Opens this window, creating it first if it has not yet been created.
   * <p>
   * If this window has been configured to block on open (
   * <code>setBlockOnOpen</code>), this method waits until the window is
   * closed by the end user, and then it returns the window's return code;
   * otherwise, this method returns immediately. A window's return codes are
   * window-specific, although two standard return codes are predefined:
   * <code>OK</code> and <code>CANCEL</code>.
   * </p>
   *
   * @return the return code
   *
   * @see #create()
   */
  int Open();

  /**
   * Sets whether the <code>open</code> method should block until the window
   * closes.
   *
   * @param shouldBlock
   *            <code>true</code> if the <code>open</code> method should
   *            not return until the window closes, and <code>false</code>
   *            if the <code>open</code> method should return immediately
   */
  void SetBlockOnOpen(bool shouldBlock);

  /**
   * Sets the default image. This is the image that will be used for windows
   * that have no shell image at the time they are opened. There is no default
   * image unless one is installed via this method.
   *
   * @param image
   *            the default image, or <code>null</code> if none
   */
  static void SetDefaultImage(void* image);

  /**
   * Sets the array of default images to use for newly opened windows. It is
   * expected that the array will contain the same icon rendered at different
   * resolutions.
   *
   * @see org.opencherry.swt.widgets.Decorations#setImages(org.opencherry.swt.graphics.Image[])
   *
   * @param images
   *            the array of images to be used when this window is opened
   * @since 3.0
   */
  static void SetDefaultImages(const std::vector<void*>& images);

  /**
   * Sets the window manager of this window.
   * <p>
   * Note that this method is used by <code>WindowManager</code> to maintain
   * a backpointer. Clients must not call the method directly.
   * </p>
   *
   * @param manager
   *            the window manager, or <code>null</code> if none
   */
  void SetWindowManager(WindowManager* manager);

  /**
   * Sets the exception handler for this application.
   * <p>
   * Note that the handler may only be set once.  Subsequent calls to this method will be
   * ignored.
   * <p>
   *
   * @param handler
   *            the exception handler for the application.
   */
  static void SetExceptionHandler(IExceptionHandler::Pointer handler);

  /**
   * Sets the default parent for modal Windows. This will be used to locate
   * the parent for any modal Window constructed with a null parent.
   *
   * @param provider shell provider that will be used to locate the parent shell
   * whenever a Window is created with a null parent
   * @since 3.1
   */
  static void SetDefaultModalParent(IShellProvider::Pointer provider);

};

}

#endif /* CHERRYWINDOW_H_ */
