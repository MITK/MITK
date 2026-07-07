/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkAIFBasedModelBase_h
#define mitkAIFBasedModelBase_h


#include <MitkPharmacokineticsExports.h>
#include <mitkModelBase.h>
#include <itkArray2D.h>

namespace mitk
{

  /** \class AIFBasedModelBase
   * \brief Base class for all pharmacokinetic perfusion models that use an Arterial Input Function (AIF).
   *
   * All AIF-based models require an array of AIF concentration values and the corresponding
   * time grid AIF(t). This class provides accessors for setting the AIF values and optionally
   * a dedicated AIF time grid. It also provides interpolation of the AIF to a specified time
   * grid that differs from the AIF's own time grid.
   *
   * The AIF must be set via an itk::Array. If no dedicated AIF time grid is specified, the
   * model's own time grid is assumed (e.g. when the AIF is derived from the same data set
   * being fitted). In that case, AIF values must have the same length as the model time grid;
   * otherwise validation will fail.
   *
   * \sa ModelBase, AIFBasedModelParameterizerBase, AterialInputFunctionGenerator
   */
  class MITKPHARMACOKINETICS_EXPORT AIFBasedModelBase : public mitk::ModelBase
  {
  public:

    typedef AIFBasedModelBase Self;
    typedef ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;


    /** Run-time type information (and related methods). */
    itkTypeMacro(AIFBasedModelBase, ModelBase);

    /** \brief Name identifier for the AIF static parameter ("Arterial Input Function"). */
    static const std::string NAME_STATIC_PARAMETER_AIF;
    /** \brief Name identifier for the AIF time grid static parameter. */
    static const std::string NAME_STATIC_PARAMETER_AIFTimeGrid;

    /** \brief Unit string for the AIF static parameter (mM). */
    static const std::string UNIT_STATIC_PARAMETER_AIF;
    /** \brief Unit string for the AIF time grid static parameter (s). */
    static const std::string UNIT_STATIC_PARAMETER_AIFTimeGrid;

    /** \brief Number of static parameters (AIF values + AIF time grid = 2). */
    static const unsigned int NUMBER_OF_STATIC_PARAMETERS;

    /** \brief Display name for the X axis ("Time"). */
    static const std::string X_AXIS_NAME;

    /** \brief Unit for the X axis ("s"). */
    static const std::string X_AXIS_UNIT;

    /** \brief Display name for the Y axis ("Concentration"). */
    static const std::string Y_AXIS_NAME;

    /** \brief Unit for the Y axis ("mM"). */
    static const std::string Y_AXIS_UNIT;

    /** \brief Array type for the Arterial Input Function AIF(t). */
    typedef itk::Array<double> AterialInputFunctionType;

    /** \brief Returns the currently set AIF concentration values. */
    itkGetConstReferenceMacro(AterialInputFunctionValues, AterialInputFunctionType);
    /** \brief Returns the currently set AIF time grid. */
    itkGetConstReferenceMacro(AterialInputFunctionTimeGrid, TimeGridType);

    /** \brief Sets the AIF concentration values.
     *  \param[in] _arg Array of AIF concentration values in mM. */
    itkSetMacro(AterialInputFunctionValues, AterialInputFunctionType);
    /** \brief Sets the AIF time grid.
     *  \param[in] _arg Array of time points in seconds corresponding to the AIF values. */
    itkSetMacro(AterialInputFunctionTimeGrid, TimeGridType);

    /** \brief Returns the X axis name ("Time").
     *  \return The X axis display name. */
    std::string GetXAxisName() const override;

    /** \brief Returns the X axis unit ("s").
     *  \return The X axis unit string. */
    std::string GetXAxisUnit() const override;

    /** \brief Returns the Y axis name ("Concentration").
     *  \return The Y axis display name. */
    std::string GetYAxisName() const override;

    /** \brief Returns the Y axis unit ("mM").
     *  \return The Y axis unit string. */
    std::string GetYAxisUnit() const override;

    /** \brief Returns the time grid currently used for the AIF.
     *
     * If a dedicated AIF time grid has been set, it is returned. Otherwise the
     * model's own time grid is returned.
     * \return Reference to the active AIF time grid. */
    const TimeGridType& GetCurrentAterialInputFunctionTimeGrid() const;

    /** \brief Returns the AIF values interpolated to the given time grid.
     *
     * If \p currentTimeGrid is empty (size 0), the original uninterpolated AIF values
     * are returned.
     * \param[in] currentTimeGrid The time grid to interpolate the AIF onto.
     * \return Interpolated AIF values matching the requested time grid. */
    const AterialInputFunctionType GetAterialInputFunction(TimeGridType currentTimeGrid) const;

    /** \brief Returns the names of all static parameters (AIF, AIF time grid).
     *  \return Vector of static parameter name strings. */
    ParameterNamesType GetStaticParameterNames() const override;
    /** \brief Returns the number of static parameters (always 2).
     *  \return Number of static parameters. */
    ParametersSizeType GetNumberOfStaticParameters() const override;
    /** \brief Returns the unit map for all static parameters.
     *  \return Map of parameter names to their unit strings. */
    ParamterUnitMapType GetStaticParameterUnits() const override;


  protected:
    AIFBasedModelBase();
    ~AIFBasedModelBase() override;

    AIFBasedModelBase(const AIFBasedModelBase& source);

    /** Reimplementation that checks if AIF and timegrid settings are valid.
     * \param[out] error Set internally to indicate the error reason if method returns false. Is used by GetSignal() for the
     * exception comment.
     * \return Returns true if the model is valid and can compute a signal. Otherwise it returns false.*/
    bool ValidateModel(std::string& error) const override;

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;

    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const
    override;

    TimeGridType m_AterialInputFunctionTimeGrid;
    AterialInputFunctionType m_AterialInputFunctionValues;


  private:

    void operator=(const Self&);  //purposely not implemented


  };
}
#endif
