//#ifndef MITKVTKQRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
//#define MITKVTKQRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
//
//#include "mitkVtkRenderWindowInteractor.h"
//#include <qobject.h>
//#include "vtkObjectFactory.h"
//
////##Documentation
////## @brief vtkRenderWindowInteractor implementation for Qt/mitk
////##
////## For using the VtkQRenderWindowInteractor it is necessary to add the VtkQRenderWindowInteractor.cpp to your project.
////## The VtkQRenderWindowInteractorFactory will install the VtkQRenderWindowInteractor in the vtkObjectFactory automatically.
////## (Bridge-pattern)
////## @ingroup NavigationControl
//class VtkQRenderWindowInteractor : public mitk::VtkRenderWindowInteractor, public QObject
//{
//public:
//  //##Documentation
//  //## Construct object so that light follows camera motion.
//  static VtkQRenderWindowInteractor *New();
//
//  vtkTypeMacro(VtkQRenderWindowInteractor,vtkRenderWindowInteractor);
//  void PrintSelf(ostream& os, vtkIndent indent);
//
//  itkTypeMacro(VtkQRenderWindowInteractor,vtkRenderWindowInteractor);
//
//  //##Documentation
//  //## Initialize the event handler
//  virtual void Initialize();
//
//  //##Documentation
//  //## Enable/Disable interactions.  By default interactors are enabled when
//  //## initialized.  Initialize() must be called prior to enabling/disabling
//  //## interaction. These methods are used when a window/widget is being
//  //## shared by multiple renderers and interactors.  This allows a "modal"
//  //## display where one interactor is active when its data is to be displayed
//  //## and all other interactors associated with the widget are disabled
//  //## when their data is not displayed.
//  virtual void Enable();
//  virtual void Disable();
//
//  //##Documentation
//  //## Win32 specific application terminate, calls ClassExitMethod then
//  //## calls PostQuitMessage(0) to terminate the application. An application can Specify
//  //## ExitMethod for alternative behavior (i.e. suppression of keyboard exit)
//  void TerminateApp(void);
//
//  // Description:
//  // Render the scene. Just pass the render call on to the 
//  // associated vtkRenderWindow.
//  virtual void Render();
//
//  //##Documentation
//  //## Win32 timer methods
//  int CreateTimer(int timertype);
//  int DestroyTimer(void);
//
//  //##Documentation
//  //## Various methods that a Qt window can redirect to this class to be 
//  //## handled.
//  virtual void MousePressEvent(mitk::MouseEvent*);
//  virtual void MouseReleaseEvent(mitk::MouseEvent*);
//  virtual void MouseMoveEvent(mitk::MouseEvent*);
//  virtual void KeyPressEvent(mitk::KeyEvent*);
//  //ETX
//
//  //##Documentation
//  //## Methods to set the default exit method for the class. This method is
//  //## only used if no instance level ExitMethod has been defined.  It is
//  //## provided as a means to control how an interactor is exited given
//  //## the various language bindings (tcl, Win32, etc.).
//  static void SetClassExitMethod(void (*f)(void *), void *arg);
//  static void SetClassExitMethodArgDelete(void (*f)(void *));
//
//  //##Documentation
//  //## These methods correspond to the the Exit, User and Pick
//  //## callbacks. They allow for the Style to invoke them.
//  virtual void ExitCallback();
//
//  virtual void SetSize(int w, int h);
//  virtual void SetSize(int size[2]);
//
//protected:
//  VtkQRenderWindowInteractor();
//  ~VtkQRenderWindowInteractor();
//
//  int    TimerId;
//
//  int     MouseInWindow;
//
//  virtual void Resize(int w, int h);
//
//  //BTX
//  //##Documentation
//  //## Class variables so an exit method can be defined for this class
//  //## (used to set different exit methods for various language bindings,
//  //## i.e. tcl, java, Win32)
//  static void (*ClassExitMethod)(void *);
//  static void (*ClassExitMethodArgDelete)(void *);
//  static void *ClassExitMethodArg;
//  //ETX
//
//  virtual void timerEvent ( QTimerEvent * );
//
//private:
//  VtkQRenderWindowInteractor(const VtkQRenderWindowInteractor&);  //## Not implemented.
//  void operator=(const VtkQRenderWindowInteractor&);  //## Not implemented.
//};
//
////## Factory for VtkQRenderWindowInteractor
//class VtkQRenderWindowInteractorFactory : public vtkObjectFactory 
//{
//  static VtkQRenderWindowInteractorFactory* m_Factory;
//public:
//  static VtkQRenderWindowInteractorFactory *New();
//  VtkQRenderWindowInteractorFactory( );
//  ~VtkQRenderWindowInteractorFactory( );
//  virtual const char* GetVTKSourceVersion();
//  virtual const char* GetDescription();
//};
//
//#endif /* MITKVTKQRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722 */
//
