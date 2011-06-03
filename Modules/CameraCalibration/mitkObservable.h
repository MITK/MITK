#ifndef mitkObservable_h
#define mitkObservable_h

#include <mitkAlgorithm.h>

namespace mitk
{
  ///
  /// filter "Observable" interface 
  ///
  class Observable
  {
  public:
    ///
    /// this class garantuees to call Update() on
    /// all observer if it changes
    /// if you need more then one observer bundle
    /// the algorithms in a BatchAlgorithm
    ///
    virtual void SetObserver( Algorithm* _Algorithm ) = 0;
  };

} // namespace mitk

#endif // mitkObservable_h
