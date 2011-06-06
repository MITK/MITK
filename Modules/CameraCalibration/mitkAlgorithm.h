#ifndef mitkAlgorithm_h
#define mitkAlgorithm_h

namespace mitk
{
  ///
  /// filter "Algorithm" interface = a class which does
  /// its work when calling this function
  /// this is a general polling mechansim
  ///
  class Algorithm
  {
  public:
    ///
    /// do update steps here
    ///
    virtual void Update() = 0;
  };

} // namespace mitk

#endif // mitkAlgorithm_h
