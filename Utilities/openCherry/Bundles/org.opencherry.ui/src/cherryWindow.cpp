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

#include "cherryWindow.h"

#include "cherryConstants.h"
#include "cherrySameShellProvider.h"
#include "tweaklets/cherryGuiWidgetsTweaklet.h"

namespace cherry
{

const int Window::OK = 0;
const int Window::CANCEL = 1;

std::vector<void*> Window::defaultImages = std::vector<void*>();

Window::IExceptionHandler::Pointer Window::exceptionHandler =
    new DefaultExceptionHandler();
IShellProvider::Pointer Window::defaultModalParent = new DefaultModalParent();

Window::WindowShellListener::WindowShellListener(Window* wnd)
 : window(wnd)
 {

 }

void Window::WindowShellListener::ShellClosed(ShellEvent::Pointer event)
{
  event->doit = false; // don't close now
  if (window->CanHandleShellCloseEvent())
  {
    window->HandleShellCloseEvent();
  }
}

void Window::DefaultExceptionHandler::HandleException(const std::exception& t)
{
  // Try to keep running.
  std::cerr << t.what();
}

Shell::Pointer Window::DefaultModalParent::GetShell()
{
  Shell::Pointer parent = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetActiveShell();

  // Make sure we don't pick a parent that has a modal child (this can lock the app)
  if (parent == 0)
  {
    // If this is a top-level window, then there must not be any open modal windows.
    parent = Window::GetModalChild(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShells());
  }
  else
  {
    // If we picked a parent with a modal child, use the modal child instead
    Shell::Pointer modalChild = Window::GetModalChild(parent->GetShells());
    if (modalChild != 0)
    {
      parent = modalChild;
    }
  }

  return parent;
}

Shell::Pointer Window::GetModalChild(const std::vector<Shell::Pointer>& toSearch)
{
  int modal = Constants::APPLICATION_MODAL | Constants::SYSTEM_MODAL | Constants::PRIMARY_MODAL;

  for (int i = toSearch.size() - 1; i >= 0; i--)
  {
    Shell::Pointer shell = toSearch[i];

    // Check if this shell has a modal child
    std::vector<Shell::Pointer> children = shell->GetShells();
    Shell::Pointer modalChild = GetModalChild(children);
    if (modalChild != 0)
    {
      return modalChild;
    }

    // If not, check if this shell is modal itself
    if (shell->IsVisible() && (shell->GetStyle() & modal) != 0)
    {
      return shell;
    }
  }

  return 0;
}

//void Window::RunEventLoop()
//{
//
//  //Use the display provided by the shell if possible
//  Display display;
//  if (shell == null)
//  {
//    display = Display.getCurrent();
//  }
//  else
//  {
//    display = loopShell.getDisplay();
//  }
//
//  while (loopShell != null && !loopShell.isDisposed())
//  {
//    try
//    {
//      if (!display.readAndDispatch())
//      {
//        display.sleep();
//      }
//    } catch (Throwable e)
//    {
//      exceptionHandler.handleException(e);
//    }
//  }
//  display.update();
//}

Window::Window(Shell::Pointer parentShell)
{
  this->parentShell = new SameShellProvider(parentShell);
  this->Init();
}

Window::Window(IShellProvider::Pointer shellProvider)
{
  poco_assert(shellProvider != 0);
  this->parentShell = shellProvider;
  this->Init();
}

void Window::Init()
{
  this->shellStyle = Constants::SHELL_TRIM;
  this->returnCode = OK;
  this->block = false;
}

bool Window::CanHandleShellCloseEvent()
{
  return true;
}

void Window::ConfigureShell(Shell::Pointer newShell)
{

  // The single image version of this code had a comment related to bug
  // 46624,
  // and some code that did nothing if the stored image was already
  // disposed.
  // The equivalent in the multi-image version seems to be to remove the
  // disposed images from the array passed to the shell.
  if (defaultImages.size() > 0)
  {
//    ArrayList nonDisposedImages = new ArrayList(defaultImages.length);
//    for (int i = 0; i < defaultImages.length; ++i)
//    {
//      if (defaultImages[i] != null && !defaultImages[i].isDisposed())
//      {
//        nonDisposedImages.add(defaultImages[i]);
//      }
//    }
//
//    if (nonDisposedImages.size() <= 0)
//    {
//      System.err.println("Window.configureShell: images disposed"); //$NON-NLS-1$
//    }
//    else
//    {
//      //Image[] array = new Image[nonDisposedImages.size()];
//      nonDisposedImages.toArray(array);
      newShell->SetImages(defaultImages);
//    }
  }

//  Layout layout = getLayout();
//  if (layout != null)
//  {
//    newShell.setLayout(layout);
//  }
}

//voidWindow::ConstrainShellSize()
//{
//  // limit the shell size to the display size
//  Rectangle bounds = shell.getBounds();
//  Rectangle constrained = getConstrainedShellBounds(bounds);
//  if (!bounds.equals(constrained))
//  {
//    shell.setBounds(constrained);
//  }
//}

void* Window::CreateContents(Shell::Pointer parent)
{
  // by default, just create a composite
  //return new Composite(parent, SWT.NONE);
  return parent;
}

Shell::Pointer Window::CreateShell()
{

  Shell::Pointer newParent = this->GetParentShell();
//  if (newParent != 0 && newParent.isDisposed())
//  {
//    parentShell = new SameShellProvider(null);
//    newParent = getParentShell();//Find a better parent
//  }

  //Create the shell
  Shell::Pointer newShell = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateShell(newParent, this->GetShellStyle());

  //    resizeListener = new Listener() {
  //      public void handleEvent(Event e) {
  //        resizeHasOccurred = true;
  //      }
  //    };

  //newShell.addListener(SWT.Resize, resizeListener);
  newShell->SetData(this);

  //Add a listener
  newShell->AddShellListener(this->GetShellListener());

  //Set the layout
  this->ConfigureShell(newShell);

//  //Register for font changes
//  if (fontChangeListener == null)
//  {
//    fontChangeListener = new FontChangeListener();
//  }
//  JFaceResources.getFontRegistry().addListener(fontChangeListener);

  return newShell;
}

void* Window::GetContents()
{
  return contents;
}

Point Window::GetInitialLocation(const Point& initialSize)
{
  void* parent = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(shell->GetControl());

//  Monitor monitor = shell.getDisplay().getPrimaryMonitor();
//  if (parent != null)
//  {
//    monitor = parent.getMonitor();
//  }
//
//  Rectangle monitorBounds = monitor.getClientArea();
  Point centerPoint(0,0);
  Rectangle parentBounds(0,0,0,0);
  if (parent != 0)
  {
    parentBounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(parent);
    centerPoint.x = parentBounds.x + parentBounds.width/2;
    centerPoint.y = parentBounds.y - parentBounds.height/2;
  }
  else
  {
    //centerPoint = Geometry.centerPoint(monitorBounds);
    parentBounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)
      ->GetScreenSize(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetPrimaryScreenNumber());
    centerPoint.x = parentBounds.width/2;
    centerPoint.y = parentBounds.height/2;
  }

  return Point(centerPoint.x - (initialSize.x / 2), centerPoint.y - (initialSize.y * 2 / 3));
}

Point Window::GetInitialSize()
{
  return shell->ComputeSize(Constants::DEFAULT, Constants::DEFAULT, true);
}

Shell::Pointer Window::GetParentShell()
{
  Shell::Pointer parent = parentShell->GetShell();

  int modal = Constants::APPLICATION_MODAL | Constants::SYSTEM_MODAL | Constants::PRIMARY_MODAL;

  if ((this->GetShellStyle() & modal) != 0)
  {
    // If this is a modal shell with no parent, pick a shell using defaultModalParent.
    if (parent == 0)
    {
      parent = defaultModalParent->GetShell();
    }
  }

  return parent;
}

IShellListener::Pointer Window::GetShellListener()
{
  if (windowShellListener == 0)
    windowShellListener = new WindowShellListener(this);

  return windowShellListener;
}

int Window::GetShellStyle()
{
  return shellStyle;
}

void Window::HandleShellCloseEvent()
{
  this->SetReturnCode(CANCEL);
  this->Close();
}

void Window::InitializeBounds()
{
//  if (resizeListener != null)
//  {
//    shell.removeListener(SWT.Resize, resizeListener);
//  }
//  if (resizeHasOccurred)
//  { // Check if shell size has been set already.
//    return;
//  }

  Point size = this->GetInitialSize();
  Point location = this->GetInitialLocation(size);
  shell->SetBounds(Rectangle(location.x, location.y, size.x, size.y));
}

void Window::SetParentShell(Shell::Pointer newParentShell)
{
  poco_assert(shell == 0); // "There must not be an existing shell."; //$NON-NLS-1$
  parentShell = new SameShellProvider(newParentShell);
}

void Window::SetReturnCode(int code)
{
  returnCode = code;
}

void Window::SetShellStyle(int newShellStyle)
{
  shellStyle = newShellStyle;
}

bool Window::Close()
{

//  // stop listening for font changes
//  if (fontChangeListener != null)
//  {
//    JFaceResources.getFontRegistry().removeListener(fontChangeListener);
//    fontChangeListener = null;
//  }

  // remove this window from a window manager if it has one
  if (windowManager != 0)
  {
    windowManager->Remove(this);
    windowManager = 0;
  }

  if (shell == 0)
  {
    return true;
  }

  // If we "close" the shell recursion will occur.
  // Instead, we need to "dispose" the shell to remove it from the
  // display.
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->DisposeShell(shell);
  shell = 0;
  contents = 0;

  return true;
}

void Window::Create()
{
  shell = this->CreateShell();
  contents = this->CreateContents(shell);

  //initialize the bounds of the shell to that appropriate for the
  // contents
  this->InitializeBounds();
}

void* Window::GetDefaultImage()
{
  return (defaultImages.size() < 1) ? 0
      : defaultImages[0];
}

std::vector<void*> Window::GetDefaultImages()
{
  return defaultImages;
}

int Window::GetReturnCode()
{
  return returnCode;
}

Shell::Pointer Window::GetShell()
{
  return shell;
}

WindowManager* Window::GetWindowManager()
{
  return windowManager;
}

int Window::Open()
{

  if (shell == 0)
  {
    // create the window
    this->Create();
  }

  // limit the shell size to the display size
  //constrainShellSize();

  // open the window
  shell->Open(block);

//  // run the event loop if specified
//  if (block)
//  {
//    this->RunEventLoop();
//  }

  return returnCode;
}

void Window::SetBlockOnOpen(bool shouldBlock)
{
  block = shouldBlock;
}

void Window::SetDefaultImage(void* image)
{
  if (image != 0)
    defaultImages.push_back(image);
}

void Window::SetDefaultImages(const std::vector<void*>& images)
{
  defaultImages = images;
}

void Window::SetWindowManager(WindowManager* manager)
{
  windowManager = manager;

  // Code to detect invalid usage

  if (manager != 0)
  {
    std::vector<Window::Pointer> windows = manager->GetWindows();
    for (unsigned int i = 0; i < windows.size(); i++)
    {
      if (windows[i] == this)
      {
        return;
      }
    }
    manager->Add(this);
  }
}

void Window::SetExceptionHandler(IExceptionHandler::Pointer handler)
{
  if (exceptionHandler == 0)
  {
    exceptionHandler = handler;
  }
}

void Window::SetDefaultModalParent(IShellProvider::Pointer provider)
{
  defaultModalParent = provider;
}

}
