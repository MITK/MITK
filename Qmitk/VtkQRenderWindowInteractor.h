#ifndef MITKVTKQRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722
#define MITKVTKQRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722

//## #include <stdlib.h>
#include "mitkVtkRenderWindowInteractor.h"
#include <qobject.h>
#include "vtkObjectFactory.h"

//##ModelId=3E6D600E0193
//##Documentation
//## @brief vtkRenderWindowInteractor implementation for Qt/mitk
//## For using the VtkQRenderWindowInteractor it is necessary to add the VtkQRenderWindowInteractor.cpp to your project.
//## The VtkQRenderWindowInteractorFactory will install the VtkQRenderWindowInteractor in the vtkObjectFactory automatically.
//## (Bridge-pattern)
//## @ingroup NavigationControl
class VtkQRenderWindowInteractor : public mitk::VtkRenderWindowInteractor, public QObject
{
public:
   //##ModelId=3E6D600F00E1
  //##Documentation
  //## Construct object so that light follows camera motion.
  static VtkQRenderWindowInteractor *New();

    //##ModelId=3E6D600F00EA
  vtkTypeMacro(VtkQRenderWindowInteractor,vtkRenderWindowInteractor);
    //##ModelId=3E6D600F00ED
  void PrintSelf(ostream& os, vtkIndent indent);

  itkTypeMacro(VtkQRenderWindowInteractor,vtkRenderWindowInteractor);

  //##ModelId=3E6D600F00F5
  //##Documentation
  //## Initialize the event handler
  virtual void Initialize();

  //##ModelId=3E6D600F00F7
  //##Documentation
  //## Enable/Disable interactions.  By default interactors are enabled when
  //## initialized.  Initialize() must be called prior to enabling/disabling
  //## interaction. These methods are used when a window/widget is being
  //## shared by multiple renderers and interactors.  This allows a "modal"
  //## display where one interactor is active when its data is to be displayed
  //## and all other interactors associated with the widget are disabled
  //## when their data is not displayed.
  virtual void Enable();
    //##ModelId=3E6D600F00FF
  virtual void Disable();

  //##ModelId=3E6D600F0101
  //##Documentation
  //## By default the interactor installs a MessageProc callback which
  //## intercepts windows messages to the window and controls interactions.
  //## MFC or BCB programs can prevent this and instead directly route any mouse/key
  //## messages into the event bindings by setting InstallMessgeProc to false.
  vtkSetMacro(InstallMessageProc,int);
    //##ModelId=3E6D600F0108
  vtkGetMacro(InstallMessageProc,int);
    //##ModelId=3E6D600F010B
  vtkBooleanMacro(InstallMessageProc,int);

  //##ModelId=3E6D600F0113
  //##Documentation
  //## Win32 specific application terminate, calls ClassExitMethod then
  //## calls PostQuitMessage(0) to terminate the application. An application can Specify
  //## ExitMethod for alternative behavior (i.e. suppression of keyboard exit)
  void TerminateApp(void);

  //##ModelId=3E6D600F0115
  //##Documentation
  //## Win32 timer methods
  int CreateTimer(int timertype);
    //##ModelId=3E6D600F011D
  int DestroyTimer(void);

  //##ModelId=3E6D600F011F
  //##Documentation
  //## Various methods that a Qt window can redirect to this class to be 
  //## handled.
  virtual void MousePressEvent(mitk::MouseEvent*);
    //##ModelId=3E6D600F0128
  virtual void MouseReleaseEvent(mitk::MouseEvent*);
    //##ModelId=3E6D600F0130
  virtual void MouseMoveEvent(mitk::MouseEvent*);
    //##ModelId=3E6D600F0133
  virtual void KeyPressEvent(mitk::KeyEvent*);
  //ETX

  //##ModelId=3E6D600F013A
  //##Documentation
  //## Methods to set the default exit method for the class. This method is
  //## only used if no instance level ExitMethod has been defined.  It is
  //## provided as a means to control how an interactor is exited given
  //## the various language bindings (tcl, Win32, etc.).
  static void SetClassExitMethod(void (*f)(void *), void *arg);
    //##ModelId=3E6D600F0144
  static void SetClassExitMethodArgDelete(void (*f)(void *));

  //##ModelId=3E6D600F0147
  //##Documentation
  //## These methods correspond to the the Exit, User and Pick
  //## callbacks. They allow for the Style to invoke them.
  virtual void ExitCallback();

    //##ModelId=3E6D600F014F
  virtual void SetSize(int w, int h);
    //##ModelId=3E6D600F0158
  virtual void SetSize(int size[2]);

protected:
    //##ModelId=3E6D600F0164
  VtkQRenderWindowInteractor();
    //##ModelId=3E6D600F016C
  ~VtkQRenderWindowInteractor();

  //HWND    WindowId;
    //##ModelId=3E6E4BB20203
  int    TimerId;
  //WNDPROC OldProc;
    //##ModelId=3E6D600F00B8
  int     InstallMessageProc;

    //##ModelId=3E6D600F00C2
  int     MouseInWindow;

    //##ModelId=3E6D600F016D
  virtual void Resize(int w, int h);

  //BTX
  //##ModelId=3E6D600F00CC
  //##Documentation
  //## Class variables so an exit method can be defined for this class
  //## (used to set different exit methods for various language bindings,
  //## i.e. tcl, java, Win32)
  static void (*ClassExitMethod)(void *);
    //##ModelId=3E6D600F00D6
  static void (*ClassExitMethodArgDelete)(void *);
    //##ModelId=3E6D600F00E0
  static void *ClassExitMethodArg;
  //ETX

    //##ModelId=3E6E4BB2027B
  virtual void timerEvent ( QTimerEvent * );

private:
    //##ModelId=3E6D600F0177
  VtkQRenderWindowInteractor(const VtkQRenderWindowInteractor&);  //## Not implemented.
    //##ModelId=3E6D600F0179
  void operator=(const VtkQRenderWindowInteractor&);  //## Not implemented.
};

//## Factory for VtkQRenderWindowInteractor
class VtkQRenderWindowInteractorFactory : public vtkObjectFactory 
{
  static VtkQRenderWindowInteractorFactory* m_Factory;
public:
  static VtkQRenderWindowInteractorFactory *New();
  VtkQRenderWindowInteractorFactory( );
  ~VtkQRenderWindowInteractorFactory( );
  virtual const char* GetVTKSourceVersion();
  virtual const char* GetDescription();
};

#endif /* MITKVTKQRENDERWINDOWINTERACTOR_H_HEADER_INCLUDED_C1C53722 */

