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
   * \brief Generic functor policy for computing SUV on a per-voxel basis.
   *
   * Designed to be used as the functor template argument for
   * itk::IndexedUnaryFunctorImageFilter. It converts raw PET pixel
   * values to SUV values using the variant-agnostic scale factor
   * (\ref computeSUVScaleFactor) where the per-variant normalization is
   * encoded in \c m_ScaleNumerator.
   *
   * The decay time can vary per voxel (looked up via a configurable
   * decay-time functor based on the voxel's image index).
   *
   * \par Configuration contract
   *
   * Default-constructed instances hold NaN for the injected activity,
   * scale numerator, and half-life, and an empty decay-time functor.
   * Such an instance is intentionally invalid: use the parameterised
   * constructor or the corresponding setters to bring it into a usable
   * state, and call IsConfigured() to verify the contract before
   * invoking operator().
   *
   * If operator() is called on an unconfigured instance, the NaN values
   * propagate through the math and produce a fully NaN output image.
   * That makes misuse visible at the output but does not raise an
   * exception. Callers that want a loud failure should validate at the
   * boundary via IsConfigured().
   *
   * The SUV variant (SUVbw, SUVlbm, SUVbsa, ...) is encoded externally:
   * pick a SUVNormalizationStrategy, compute its scale numerator from
   * the patient measurements, and feed it to SetScaleNumerator(). For
   * the legacy SUVbw use case, SUVbwFunctorPolicy provides a thin
   * BW-specific shortcut that takes body weight in [kg].
   *
   * \sa computeSUVScaleFactor, SUVNormalizationStrategy,
   *     SUVbwFunctorPolicy, itk::IndexedUnaryFunctorImageFilter
   */
  class MITKPET_EXPORT SUVFunctorPolicy
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
     * Leaves the policy in an unconfigured state (NaN scalars, empty
     * decay-time functor). Setters must be called before invoking
     * operator(); see IsConfigured(). The default constructor exists to
     * satisfy itk::IndexedUnaryFunctorImageFilter, which holds a
     * default-constructed functor value member that is overwritten by
     * SetFunctor().
     */
    SUVFunctorPolicy();

    /**
     * \brief Construct a fully configured policy in one step.
     *
     * The decay-time functor must still be supplied via
     * SetDecayTimeFunctor() before the policy is invoked.
     *
     * \param[in] injectedActivity Injected activity in [Bq].
     * \param[in] scaleNumerator   Variant-specific normalization
     *                             quantity (e.g. body weight in [g]
     *                             for SUVbw / SUVlbm, BSA in [cm^2]
     *                             for SUVbsa).
     * \param[in] halfLife         Radionuclide half-life in [s].
     */
    SUVFunctorPolicy(double injectedActivity, double scaleNumerator, double halfLife);

    /** \brief Destructor. */
    virtual ~SUVFunctorPolicy();

    /**
     * \brief Get the number of output components per pixel.
     *
     * \return Always returns 1.
     */
    unsigned int GetNumberOfOutputs() const;

    /**
     * \brief Set the functor used to query the decay time for a given
     *        voxel index.
     *
     * \param[in] functor A callable that accepts an IndexType and
     *            returns the decay time in seconds.
     */
    void SetDecayTimeFunctor(const DecayTimeFunctionType& functor);

    /**
     * \brief Set the injected radioactivity.
     *
     * \param[in] a Injected activity in [Bq].
     */
    void SetInjectedActivity(double a);

    /**
     * \brief Set the variant-specific scale numerator.
     *
     * \param[in] n Scale numerator. Units depend on the SUV variant
     *              (see SUVNormalizationStrategy).
     */
    void SetScaleNumerator(double n);

    /**
     * \brief Set the radionuclide half-life.
     *
     * \param[in] tau Half-life in [s].
     */
    void SetHalfLife(double tau);

    /**
     * \brief Check whether the policy is fully configured and safe to invoke.
     *
     * Returns \c true iff all three numeric parameters (injected
     * activity, scale numerator, half-life) are finite and a decay-time
     * functor has been set. Callers should validate at the boundary
     * before invoking operator().
     *
     * \return \c true if every required parameter has been supplied.
     */
    bool IsConfigured() const;

    /**
     * \brief Inequality comparison operator.
     *
     * Compares only the three scalar parameters. The decay-time functor
     * is deliberately not compared (std::function does not provide a
     * meaningful equality).
     *
     * \param[in] other The other policy to compare against.
     * \return \c true if the policies differ in injected activity,
     *         scale numerator, or half-life.
     */
    bool operator!=(const SUVFunctorPolicy& other) const;

    /**
     * \brief Equality comparison operator.
     *
     * Compares only the three scalar parameters; see operator!=.
     *
     * \param[in] other The other policy to compare against.
     * \return \c true if injected activity, scale numerator, and
     *         half-life are identical.
     */
    bool operator==(const SUVFunctorPolicy& other) const;

    /**
     * \brief Compute the SUV for a single pixel value at a given image index.
     *
     * Looks up the decay time via the configured decay-time functor and
     * multiplies the input value by the SUV scale factor. No per-voxel
     * validation is performed; see IsConfigured() for the boundary
     * check.
     *
     * \param[in] value The raw PET pixel value.
     * \param[in] currentIndex The 3D image index of the current pixel
     *            (used to query decay time).
     * \return The computed SUV value. NaN if the policy was not
     *         configured.
     *
     * \pre IsConfigured() returns true. If not, the result will be NaN.
     */
    SUVPixelType operator()(const SUVPixelType& value,
      const IndexType& currentIndex) const;

  protected:

    /** Activity injected in [Bq]. NaN until SetInjectedActivity is called. */
    double m_InjectedActivity = std::numeric_limits<double>::quiet_NaN();
    /** Variant-specific scale numerator. NaN until SetScaleNumerator is called. */
    double m_ScaleNumerator   = std::numeric_limits<double>::quiet_NaN();
    /** Halflife of the used nuclide in [s]. NaN until SetHalfLife is called. */
    double m_HalfLife         = std::numeric_limits<double>::quiet_NaN();

    DecayTimeFunctionType m_Functor;
  };

  /**
   * \brief Body-weight-normalized SUV functor policy (SUVbw).
   *
   * Backward-compatible thin specialization of SUVFunctorPolicy that
   * accepts body weight in [kg] directly. The body weight is converted
   * to grams internally and stored in the base class as the scale
   * numerator, so the math is identical to SUVFunctorPolicy + a
   * BodyWeightStrategy.
   *
   * New code should prefer SUVFunctorPolicy together with a
   * SUVNormalizationStrategy; this class is kept so the existing PET
   * SUV plugin keeps compiling against the same API it used before
   * SUVlbm/SUVbsa were added.
   *
   * \sa SUVFunctorPolicy, BodyWeightStrategy
   */
  class MITKPET_EXPORT SUVbwFunctorPolicy : public SUVFunctorPolicy
  {
  public:
    /** \brief Default constructor. See SUVFunctorPolicy::SUVFunctorPolicy(). */
    SUVbwFunctorPolicy() = default;

    /**
     * \brief Construct a fully configured policy in one step.
     *
     * \param[in] injectedActivity Injected activity in [Bq].
     * \param[in] bodyweight       Body weight in [kg].
     * \param[in] halfLife         Radionuclide half-life in [s].
     */
    SUVbwFunctorPolicy(double injectedActivity, double bodyweight, double halfLife);

    /**
     * \brief Set the patient's body weight.
     *
     * Internally stores \c w * 1000 (i.e. grams) as the scale numerator
     * on the base class.
     *
     * \param[in] w Body weight in [kg].
     */
    void SetBodyWeight(double w);
  };

}


#endif
