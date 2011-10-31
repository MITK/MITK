#ifndef SINGLETONTWO_H
#define SINGLETONTWO_H

#include <itkLightObject.h>

#include <mitkCommon.h>
#include <mitkServiceInterface.h>

class SingletonTwo
{
public:

  static SingletonTwo& GetInstance();

  int b;

private:

  SingletonTwo();
  ~SingletonTwo();

  // Disable copy constructor and assignment operator.
  SingletonTwo(const SingletonTwo&);
  SingletonTwo& operator=(const SingletonTwo&);
};

class SingletonTwoService : public itk::LightObject
{
public:

  static SingletonTwoService* GetInstance();

  int b;

private:

  friend class MyActivator;

  mitkClassMacro(SingletonTwoService, itk::LightObject)
  itkFactorylessNewMacro(SingletonTwoService)

  SingletonTwoService();
  ~SingletonTwoService();

  // Disable copy constructor and assignment operator.
  SingletonTwoService(const SingletonTwoService&);
  SingletonTwoService& operator=(const SingletonTwoService&);
};

MITK_DECLARE_SERVICE_INTERFACE(SingletonTwoService, "org.mitk.snippet.SingletonTwoService")

#endif // SINGLETONTWO_H
