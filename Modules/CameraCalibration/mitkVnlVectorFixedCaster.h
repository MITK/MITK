/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkVnlVectorFixedCaster_h
#define mitkVnlVectorFixedCaster_h

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
  class VnlVectorFixedCaster:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    VnlVectorFixedCaster(
      const vnl_vector_fixed<T, n>* _InputVector,
      vnl_vector_fixed<R, n>* _OutputVector):
        m_InputVector(_InputVector),
        m_OutputVector(_OutputVector)
    {
    }
    ///
    /// executes the Algorithm
    ///
    void Update() override
    {
      for( size_t i=0; i<m_InputVector->size(); ++i )
        (*m_OutputVector)[i] = static_cast<R>( (*m_InputVector)[i] );

    }
  private:
    ///
    /// VnlVectorFixedCaster input member variable
    ///
    const vnl_vector_fixed<T, n>* m_InputVector;
    ///
    /// VnlVectorFixedCaster output member variable
    ///
    vnl_vector_fixed<R, n>* m_OutputVector;
  };
} // namespace mitk

#endif
