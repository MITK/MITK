#ifndef MITKSTATUSBAR_H
#define MITKSTATUSBAR_H
#include "itkObject.h"

namespace mitk {
//##Documentation
//## @brief Interface for a class, that provides to send a Message to the applications StatusBar
//## @ingroup Interaction
//## has to be derived and implemented for each GUI platform
//## is nearly equal to itk::OutputWindow, 
//## no Window, but one line of text and a delay for clear. There are different Texts, that 
//## can be displayed differently. 
//## ToDo: create different views with different colors for the different Outputs!
class StatusBar : public itk::Object
{
public:
  itkTypeMacro(StatusBar, itk::Object);
  
  typedef StatusBar	Self;
  typedef itk::Object Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self>  ConstPointer;
  
  //##Documentation
  //## @brief static method to get the singleton StatusBar-instance 
  //## so the methods DisplayText, etc. can be called
  //## No reference counting, cause of decentral static use!
  static StatusBar* GetInstance();

  //##Documentation
  //## @brief Supply a GUI- dependant StatusBar. Has to be set by the application
  //## to connect the application dependant subclass of mitkStatusBar
  //## if you create an instance, then call ->Delete() on the supplied
  //## instance after setting it.
  static void SetInstance(StatusBar* instance);

  //##Documentation
  //## @brief Send a string to the applications StatusBar
  virtual void DisplayText(const char* t);
  //##Documentation
  //## @brief Send a string with a time delay to the applications StatusBar
  virtual void DisplayText(const char* t, int ms);

  virtual void DisplayErrorText(const char *t);
  virtual void DisplayWarningText(const char *t);
  virtual void DisplayWarningText(const char *t, int ms);
  virtual void DisplayGenericOutputText(const char *t);
  virtual void DisplayDebugText(const char *t);

  //##Documentation
  //## @brief removes any temporary message being shown.
  virtual void Clear();

  //##Documentation
  //## @brief Set the SizeGrip of the window 
  //## (the triangle in the lower right Windowcorner for changing the size) 
  //## to enabled or disabled 
  virtual void SetSizeGripEnabled(bool enable);

protected:
  StatusBar();
  virtual ~StatusBar();
  static Pointer m_Instance;
};

}// end namespace mitk
#endif /* define MITKSTATUSBAR_H */
