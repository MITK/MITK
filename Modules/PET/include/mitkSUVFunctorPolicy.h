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

#include <functional>
#include <limits>

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
   * \par Configuration contract
   *
   * Default-constructed instances hold NaN for the injected activity, body weight,
   * and half-life, and an empty decay-time functor. Such an instance is
   * intentionally invalid: use the parameterised constructor or the corresponding
   * setters to bring it into a usable state, and call IsConfigured() to verify
   * the contract before invoking operator().
   *
   * If operator() is called on an unconfigured instance, the NaN values propagate
   * through the math and produce a fully NaN output image. That makes misuse
   * visible at the output but does not raise an exception. Callers that want a
   * loud failure should validate at the boundary via IsConfigured().
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

    /**
     * \brief Default constructor.
     *
     * Leaves the policy in an unconfigured state (NaN scalars, empty decay-time
     * functor). Setters must be called before invoking operator(); see
     * IsConfigured(). The default constructor exists to satisfy
     * itk::IndexedUnaryFunctorImageFilter, which holds a default-constructed
     * functor value member that is overwritten by SetFunctor().
     */
    SUVbwFunctorPolicy();

    /**
     * \brief Construct a fully configured policy in one step.
     *
     * The decay-time functor must still be supplied via SetDecayTimeFunctor()
     * before the policy is invoked.
     *
     * \param[in] injectedActivity Injected activity in [Bq].
     * \param[in] bodyweight       Body weight in [kg].
     * \param[in] halfLife         Radionuclide half-life in [s].
     */
    SUVbwFunctorPolicy(double injectedActivity, double bodyweight, double halfLife);

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
     * \brief Check whether the policy is fully configured and safe to invoke.
     *
     * Returns \c true iff all three numeric parameters (injected activity, body
     * weight, half-life) are finite and a decay-time functor has been set.
     * Callers should validate at the boundary before invoking operator().
     *
     * \return \c true if every required parameter has been supplied.
     */
    bool IsConfigured() const;

    /**
     * \brief Inequality comparison operator.
     *
     * Compares only the three scalar parameters. The decay-time functor is
     * deliberately not compared (std::function does not provide a meaningful
     * equality).
     *
     * \param[in] other The other policy to compare against.
     * \return \c true if the policies differ in injected activity, body weight, or half-life.
     */
    bool operator!=(const SUVbwFunctorPolicy& other) const;

    /**
     * \brief Equality comparison operator.
     *
     * Compares only the three scalar parameters; see operator!=.
     *
     * \param[in] other The other policy to compare against.
     * \return \c true if injected activity, body weight, and half-life are identical.
     */
    bool operator==(const SUVbwFunctorPolicy& other) const;

    /**
     * \brief Compute the SUVbw for a single pixel value at a given image index.
     *
     * Looks up the decay time via the configured decay time functor and multiplies
     * the input value by the SUVbw scale factor. No per-voxel validation is
     * performed; see IsConfigured() for the boundary check.
     *
     * \param[in] value The raw PET pixel value.
     * \param[in] currentIndex The 3D image index of the current pixel (used to query decay time).
     * \return The computed SUVbw value. NaN if the policy was not configured.
     *
     * \pre IsConfigured() returns true. If not, the result will be NaN.
     */
    SUVPixelType operator()(const SUVPixelType& value,
      const IndexType& currentIndex) const;

  private:

    /** Activity injected in [Bq]. NaN until SetInjectedActivity is called. */
    double m_InjectedActivity = std::numeric_limits<double>::quiet_NaN();
    /** Weight of the subject in [kg]. NaN until SetBodyWeight is called. */
    double m_bodyweight = std::numeric_limits<double>::quiet_NaN();
    /** Halflife of the used nuclide in [s]. NaN until SetHalfLife is called. */
    double m_halfLife = std::numeric_limits<double>::quiet_NaN();

    DecayTimeFunctionType m_Functor;
  };

}


#endif
