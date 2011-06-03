#ifndef mitkVnlVectorCaster_h
#define mitkVnlVectorCaster_h

#include <vnl/vnl_vector_fixed.h>
#include <mitkObservable.h>
#include <mitkAlgorithm.h>

namespace mitk
{
  ///
  /// class for casting vnl vectors to another basic type
  /// by means of static cast
  ///
  template <class T, class R, unsigned int n>
  class VnlVectorCaster:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    VnlVectorCaster(
      const vnl_vector_fixed<T, n>* _InputVector,
      vnl_vector_fixed<R, n>* _OutputVector):
        m_InputVector(_InputVector), 
        m_OutputVector(_OutputVector)
    {
    }
    ///
    /// executes the Algorithm 
    ///
    void Update()
    {
      for( size_t i=0; i<m_InputVector->size(); ++i )
        (*m_OutputVector)[i] = static_cast<R>( (*m_InputVector)[i] );

    }
  private:
    ///
    /// VnlVectorCaster input member variable
    ///
    const vnl_vector_fixed<T, n>* m_InputVector;
    ///
    /// VnlVectorCaster output member variable
    ///
    vnl_vector_fixed<R, n>* m_OutputVector;
  };
} // namespace mitk

#endif // mitkVnlVectorCaster_h
