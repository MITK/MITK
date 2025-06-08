/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef SUVFUNCTOR_POLICY_H
#define SUVFUNCTOR_POLICY_H

#include <vector>
#include <functional>

#include "itkIndex.h"
#include "mitkNumericConstants.h"

#include "MitkPETExports.h"

namespace mitk
{

  /** Functor policy to calculate the body weightes SUV with help of the itk::IndexedUnaryFunctorImageFilter. */
  class MITKPET_EXPORT SUVbwFunctorPolicy
  {
  public:
    typedef ScalarType SUVPixelType;

    typedef itk::Index<3> IndexType;
    /**Type for functors that are used to query the decay time for a certain index.
     @return The functor */
    using DecayTimeFunctionType = std::function < double(const IndexType&) >;

    SUVbwFunctorPolicy();

    ~SUVbwFunctorPolicy();

    unsigned int GetNumberOfOutputs() const;

    /**Sets a functor that returns the decay time in seconds for a passed Index*/
    void SetDecayTimeFunctor(const DecayTimeFunctionType& functor);

    void SetInjectedActivity(double a);

    void SetBodyWeight(double w);

    void SetHalfLife(double tau);

    bool operator!=(const SUVbwFunctorPolicy& other) const;

    bool operator==(const SUVbwFunctorPolicy& other) const;

    SUVPixelType operator()(const SUVPixelType& value,
      const IndexType& currentIndex) const;

  private:

    /**Activity injected in [Bq]*/
    double m_InjectedActivity;
    /**Weight of the subject in [kg]*/
    double m_bodyweight;
    /**Halflife of the used nuclide in [sec] */
    double m_halfLife;

    DecayTimeFunctionType m_Functor;
  };

}


#endif // LEVENBERGMARQUARDTMODELFITFUNCTOR_H
