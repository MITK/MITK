/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkVnlVectorCaster_h
#define mitkVnlVectorCaster_h

#include <vnl/vnl_vector.h>
#include <mitkObservable.h>
#include <mitkAlgorithm.h>

namespace mitk
{
  ///
  /// class for casting vnl vectors to another basic type
  /// by means of static cast
  ///
  template <class T, class R>
  class VnlVectorCaster:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    VnlVectorCaster(
      const vnl_vector<T>* _InputVector,
      vnl_vector<R>* _OutputVector):
        m_InputVector(_InputVector),
        m_OutputVector(_OutputVector)
    {
    }

    ///
    /// static update
    ///
    static void Update(
      const vnl_vector<T>& _InputVector,
      vnl_vector<R>& _OutputVector)
    {
      for( size_t i=0; i<_InputVector.size(); ++i )
        _OutputVector[i] = static_cast<R>( _InputVector[i] );
    }

    ///
    /// executes the Algorithm
    ///
    void Update()
    {
      Update( *m_InputVector, *m_OutputVector );
    }
  private:
    ///
    /// VnlVectorCaster input member variable
    ///
    const vnl_vector<T>* m_InputVector;
    ///
    /// VnlVectorCaster output member variable
    ///
    vnl_vector<R>* m_OutputVector;
  };
} // namespace mitk

#endif // mitkVnlVectorCaster_h
