/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCurveParameterFunctor_h
#define mitkCurveParameterFunctor_h

#include <mitkCurveDescriptionParameterBase.h>
#include <mitkSimpleFunctorBase.h>
#include <MitkPharmacokineticsExports.h>

namespace mitk
{

  /** \class CurveParameterFunctor
   * \brief Functor that aggregates multiple CurveDescriptionParameterBase instances and computes all registered descriptors.
   *
   * This functor can be used with itkMultiOutputNaryImageFilter for pixel-wise curve
   * descriptor computation. Register any number of CurveDescriptionParameterBase instances;
   * the functor will compute all their values for each input curve.
   *
   * \warning The functor's Compute method and registered descriptor instances must be thread-safe.
   * \sa CurveDescriptionParameterBase, PixelBasedDescriptionParameterImageGenerator, SimpleFunctorBase
   */
  class MITKPHARMACOKINETICS_EXPORT CurveParameterFunctor : public SimpleFunctorBase
  {
  public:
    typedef CurveParameterFunctor Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

      itkFactorylessNewMacro(Self);
      itkTypeMacro(CurveParameterFunctor, SimpleFunctorBase);

      typedef CurveDescriptionParameterBase::CurveDescriptionParameterNameType ParameterNameType;
      typedef CurveDescriptionParameterBase::DescriptionParameterNamesType ParameterNamesType;
      using GridArrayType = SimpleFunctorBase::GridArrayType;

    /** \brief Computes all registered description parameters for the given input curve values.
     *  \param[in] value The input pixel vector representing the curve values over time.
     *  \return Vector of all computed descriptor values from all registered parameters.
     *  \throw itk::ExceptionObject if no grid or no descriptors are set. */
    SimpleFunctorBase::OutputPixelVectorType Compute(const InputPixelVectorType & value) const override;

    /** \brief Returns the total number of output values across all registered descriptors.
     *  \return The number of output values. */
    unsigned int GetNumberOfOutputs() const override;

    /** \brief Returns the currently set time grid.
     *  \return The time grid array. */
    GridArrayType GetGrid() const override;
    /** \brief Sets the time grid for curve descriptor computation.
     *  \param[in] _arg The time grid array (in seconds). */
    itkSetMacro(Grid, GridArrayType);

    /** \brief Returns the names of all registered description parameters.
     *
     * If a descriptor produces multiple values, its name is prefixed to each sub-parameter name.
     * \return Vector of all parameter names. */
    ParameterNamesType GetDescriptionParameterNames() const;

    /** \brief Removes all registered description parameter functions.
     *  \warning Not thread-safe. Do not call while Compute is running. */
    void ResetDescriptionParameters();
    /** \brief Registers a new description parameter function under the given name.
     *  \param[in] parameterName The name to register the parameter under.
     *  \param[in] parameterFunction Pointer to the descriptor function to register.
     *  \warning Not thread-safe. Do not call while Compute is running. */
    void RegisterDescriptionParameter(const ParameterNameType& parameterName, CurveDescriptionParameterBase* parameterFunction);
    /** \brief Returns the descriptor function registered under the given name.
     *  \param[in] parameterName The name to look up.
     *  \return Pointer to the descriptor function, or nullptr if not found.
     *  \warning Not thread-safe. Do not call while Compute is running. */
    const CurveDescriptionParameterBase* GetDescriptionParameterFunction(const ParameterNameType& parameterName) const;

  protected:
    CurveParameterFunctor();
    ~CurveParameterFunctor() override;

  private:
    typedef std::map<ParameterNameType, CurveDescriptionParameterBase::Pointer> DescriptionParameterMapType;
    DescriptionParameterMapType m_DescriptorMap;
    GridArrayType m_Grid;
  };

}

#endif
