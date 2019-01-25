#ifndef mitkIRESTObserver_h
#define mitkIRESTObserver_h

namespace mitk
{
  class IRESTObserver
  {
  public:
    virtual bool notify() = 0;


  private:
  };
}

#endif // !mitkIRESTObserver
