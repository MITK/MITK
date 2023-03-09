/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSimpleFunctorPolicy_h
#define mitkSimpleFunctorPolicy_h

#include "itkIndex.h"
#include "mitkSimpleFunctorBase.h"
#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT SimpleFunctorPolicy
  {
  public:
    typedef std::vector<mitk::ScalarType> InputPixelArrayType;
    typedef std::vector<mitk::ScalarType> OutputPixelArrayType;

    typedef SimpleFunctorBase               FunctorType;
    typedef SimpleFunctorBase::ConstPointer FunctorConstPointer;

    typedef itk::Index<3> IndexType;

    SimpleFunctorPolicy();

    ~SimpleFunctorPolicy();

    unsigned int GetNumberOfOutputs() const;

    void SetFunctor(const mitk::SimpleFunctorBase *functor);

    bool operator!=(const SimpleFunctorPolicy & other) const;

    bool operator==(const SimpleFunctorPolicy & other) const;

    inline OutputPixelArrayType operator()( const InputPixelArrayType & value, const IndexType& /*currentIndex*/ ) const
    {
      if (!m_Functor)
      {
        itkGenericExceptionMacro(<< "Error. Cannot process operator(). Functor is Null.");
      }

      OutputPixelArrayType result = m_Functor->Compute(value);

      return result;
    }

  private:

    FunctorConstPointer m_Functor;
  };

}


#endif
