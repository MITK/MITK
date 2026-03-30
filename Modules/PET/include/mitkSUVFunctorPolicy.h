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

#ifndef mitkSUVFunctorPolicy_h
#define mitkSUVFunctorPolicy_h

#include <vector>
#include <functional>

#include <itkIndex.h>
#include <mitkNumericConstants.h>

#include <MitkPETExports.h>

namespace mitk
{

  /**
   * \brief Functor policy for computing body-weight-normalized SUV (SUVbw) on a per-voxel basis.
   *
   * This class is designed to be used as the functor template argument for
   * itk::IndexedUnaryFunctorImageFilter. It converts raw PET pixel values to SUVbw
   * values, where the decay time can vary per voxel (looked up via a configurable
   * decay time functor based on the voxel's image index).
   *
   * Before use, the injected activity, body weight, half-life, and a decay time
   * functor must be configured.
   *
   * \sa computeSUVbwScaleFactor, computeSUVbw, itk::IndexedUnaryFunctorImageFilter
   */
  class MITKPET_EXPORT SUVbwFunctorPolicy
  {
  public:
    /** \brief Pixel type for SUV output values. */
    typedef ScalarType SUVPixelType;

    /** \brief 3D image index type. */
    typedef itk::Index<3> IndexType;

    /**
     * \brief Function type for querying the decay time at a given image index.
     *
     * The function takes a 3D image index and returns the corresponding
     * decay time in seconds.
     */
    using DecayTimeFunctionType = std::function < double(const IndexType&) >;

    /** \brief Default constructor. */
    SUVbwFunctorPolicy();

    /** \brief Destructor. */
    ~SUVbwFunctorPolicy();

    /**
     * \brief Get the number of output components per pixel.
     *
     * \return Always returns 1.
     */
    unsigned int GetNumberOfOutputs() const;

    /**
     * \brief Set the functor used to query the decay time for a given voxel index.
     *
     * \param[in] functor A callable that accepts an IndexType and returns the decay
     *            time in seconds.
     */
    void SetDecayTimeFunctor(const DecayTimeFunctionType& functor);

    /**
     * \brief Set the injected radioactivity.
     *
     * \param[in] a Injected activity in [Bq].
     */
    void SetInjectedActivity(double a);

    /**
     * \brief Set the patient's body weight.
     *
     * \param[in] w Body weight in [kg].
     */
    void SetBodyWeight(double w);

    /**
     * \brief Set the radionuclide half-life.
     *
     * \param[in] tau Half-life in [s].
     */
    void SetHalfLife(double tau);

    /**
     * \brief Inequality comparison operator.
     *
     * \param[in] other The other policy to compare against.
     * \return \c true if the policies differ in injected activity, body weight, or half-life.
     */
    bool operator!=(const SUVbwFunctorPolicy& other) const;

    /**
     * \brief Equality comparison operator.
     *
     * \param[in] other The other policy to compare against.
     * \return \c true if injected activity, body weight, and half-life are identical.
     */
    bool operator==(const SUVbwFunctorPolicy& other) const;

    /**
     * \brief Compute the SUVbw for a single pixel value at a given image index.
     *
     * Looks up the decay time via the configured decay time functor and multiplies
     * the input value by the SUVbw scale factor.
     *
     * \param[in] value The raw PET pixel value.
     * \param[in] currentIndex The 3D image index of the current pixel (used to query decay time).
     * \return The computed SUVbw value.
     *
     * \pre A decay time functor must have been set via SetDecayTimeFunctor().
     */
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
