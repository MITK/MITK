/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryWindow.h"

#include "berryConstants.h"
#include "berrySameShellProvider.h"
#include "berryMenuManager.h"

#include <QMainWindow>

namespace berry
{

const int Window::OK = 0;
const int Window::CANCEL = 1;

QList<QIcon> Window::defaultImages = QList<QIcon>();

Window::IExceptionHandler::Pointer Window::exceptionHandler(new DefaultExceptionHandler());
IShellProvider::Pointer Window::defaultModalParent(new DefaultModalParent());

Window::WindowShellListener::WindowShellListener(Window* wnd)
 : window(wnd)
 {

 }

void Window::WindowShellListener::ShellClosed(const ShellEvent::Pointer& event)
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

Shell::Pointer Window::DefaultModalParent::GetShell() const
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

Shell::Pointer Window::GetModalChild(const QList<Shell::Pointer>& toSearch)
{
  int modal = Constants::APPLICATION_MODAL | Constants::SYSTEM_MODAL | Constants::PRIMARY_MODAL;

  int size = toSearch.size();
  for (int i = size - 1; i < size; i--)
  {
    Shell::Pointer shell = toSearch[i];

    // Check if this shell has a modal child
    QList<Shell::Pointer> children = shell->GetShells();
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

  return Shell::Pointer(nullptr);
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

Window::~Window()
{

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
  CreateTrimWidgets(newShell);
}

//voidWindow::ConstrainShellSize()
//{
//  // limit the shell size to the display size
//  QRect bounds = shell.getBounds();
//  QRect constrained = getConstrainedShellBounds(bounds);
//  if (!bounds.equals(constrained))
//  {
//    shell.setBounds(constrained);
//  }
//}

QWidget* Window::CreateContents(Shell::Pointer parent)
{
  // by default, just create a composite
  //return new Composite(parent, SWT.NONE);
  return parent->GetControl();
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
  newShell->SetData(Object::Pointer(this));

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

QWidget* Window::GetContents()
{
  return contents;
}

QPoint Window::GetInitialLocation(const QPoint& initialSize)
{
  QWidget* parent = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(shell->GetControl());

  QPoint centerPoint(0,0);
  QRect parentBounds(0,0,0,0);
  if (parent != nullptr)
  {
    parentBounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(parent);
    centerPoint.setX(parentBounds.x() + parentBounds.width()/2);
    centerPoint.setY(parentBounds.y() - parentBounds.height()/2);
  }
  else
  {
    parentBounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)
      ->GetScreenSize(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetPrimaryScreenNumber());
    centerPoint.setX(parentBounds.width()/2);
    centerPoint.setY(parentBounds.height()/2);
  }

  return QPoint(centerPoint.x() - (initialSize.x() / 2),
              std::max<int>(parentBounds.y(),
                            std::min<int>(centerPoint.y() - (initialSize.y() * 2 / 3),
                                          parentBounds.y() + parentBounds.height() - initialSize.y())));
}

QPoint Window::GetInitialSize()
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

IShellListener* Window::GetShellListener()
{
  if (windowShellListener.isNull())
    windowShellListener.reset(new WindowShellListener(this));

  return windowShellListener.data();
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

  QPoint size = this->GetInitialSize();
  QPoint location = this->GetInitialLocation(size);
  shell->SetBounds(this->GetConstrainedShellBounds(QRect(location.x(), location.y(), size.x(), size.y())));
}

QRect Window::GetConstrainedShellBounds(const QRect& preferredSize)
{
  QRect result(preferredSize);

  GuiWidgetsTweaklet* guiTweaklet(Tweaklets::Get(GuiWidgetsTweaklet::KEY));
  int screenNum = guiTweaklet->GetClosestScreenNumber(result);
  QRect bounds(guiTweaklet->GetAvailableScreenSize(screenNum));

  if (result.height() > bounds.height()) {
    result.setHeight(bounds.height());
  }

  if (result.width() > bounds.width()) {
    result.setWidth(bounds.width());
  }

  result.moveLeft( std::max<int>(bounds.x(), std::min<int>(result.x(), bounds.x()
    + bounds.width() - result.width())));
  result.moveTop(std::max<int>(bounds.y(), std::min<int>(result.y(), bounds.y()
    + bounds.height() - result.height())));

  return result;
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

void Window::AddMenuBar()
{
  if (GetShell().IsNull() && menuBarManager.IsNull())
  {
    menuBarManager = CreateMenuManager();
  }
}

SmartPointer<MenuManager> Window::CreateMenuManager()
{
  MenuManager::Pointer manager(new MenuManager());
  return manager;
}

void Window::CreateTrimWidgets(SmartPointer<Shell> shell)
{
  if (menuBarManager.IsNotNull())
  {
    QMainWindow* mw = qobject_cast<QMainWindow*>(shell->GetControl());
    if (mw)
    {
      mw->setMenuBar(menuBarManager->CreateMenuBar(shell->GetControl()));
      menuBarManager->UpdateAll(true);
    }
  }

//  if (showTopSeperator())
//  {
//    seperator1 = new Label(shell, SWT.SEPARATOR | SWT.HORIZONTAL);
//  }

  //CreateToolBarControl(shell);
  //CreateStatusLine(shell);
}

bool Window::Close()
{

//  BERRY_INFO << "Window::Close()";

//  // stop listening for font changes
//  if (fontChangeListener != null)
//  {
//    JFaceResources.getFontRegistry().removeListener(fontChangeListener);
//    fontChangeListener = null;
//  }

  // remove this window from a window manager if it has one
  if (windowManager != nullptr)
  {
    windowManager->Remove(Window::Pointer(this));
    windowManager = nullptr;
  }

  if (shell == 0)
  {
    return true;
  }

  shell->RemoveShellListener(this->GetShellListener());
  shell->SetData(Object::Pointer(nullptr));

  // If we "close" the shell recursion will occur.
  // Instead, we need to "dispose" the shell to remove it from the
  // display.
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->DisposeShell(shell);
  shell = nullptr;
  contents = nullptr;

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

QIcon Window::GetDefaultImage()
{
  return (defaultImages.size() < 1) ? QIcon() : defaultImages[0];
}

QList<QIcon> Window::GetDefaultImages()
{
  return defaultImages;
}

int Window::GetReturnCode()
{
  return returnCode;
}

Shell::Pointer Window::GetShell() const
{
  return shell;
}

WindowManager* Window::GetWindowManager()
{
  return windowManager;
}

MenuManager *Window::GetMenuBarManager() const
{
  return menuBarManager.GetPointer();
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

void Window::SetDefaultImage(const QIcon& image)
{
  if (!image.isNull())
    defaultImages.push_back(image);
}

void Window::SetDefaultImages(const QList<QIcon>& images)
{
  defaultImages = images;
}

void Window::SetWindowManager(WindowManager* manager)
{
  windowManager = manager;

  // Code to detect invalid usage

  if (manager != nullptr)
  {
    QList<Window::Pointer> windows = manager->GetWindows();
    for (int i = 0; i < windows.size(); i++)
    {
      if (windows[i] == this)
      {
        return;
      }
    }
    manager->Add(Window::Pointer(this));
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
