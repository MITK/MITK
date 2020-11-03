/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef AIFBASEDMODELBASE_H
#define AIFBASEDMODELBASE_H


#include "MitkPharmacokineticsExports.h"
#include "mitkModelBase.h"
#include "itkArray2D.h"

namespace mitk
{

  /** \class AIFBasedModelBase
   * \brief Base Class for all physiological perfusion models using an Aterial Input Function
   * All AIF based models come with an array of AIF values and the corresponding TimeGrid ( AIF(t))
   * This class provides functions for setting the AIF Values and optionally a specific AIF TimeGrid.
   * It also provides a method for interpolation of the AIF source array to a specified Timegrid that differs from
   * AIFTimeGrid. The AIF must be set with an itk::Array. If no AIFTimeGrid is specified with the Setter, it is assumed
   * that the AIFTimeGrid is the same as the ModelTimegrid (e.g. AIF is derived from data set to be fitted). In this
   * case, AIFvalues must have the same length as ModelTimeGrid, otherwise an exception is generated*/
  class MITKPHARMACOKINETICS_EXPORT AIFBasedModelBase : public mitk::ModelBase
  {
  public:

    typedef AIFBasedModelBase Self;
    typedef ModelBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;


    /** Run-time type information (and related methods). */
    itkTypeMacro(PhysiologciModelBase, AIFBasedModelBase);


    static const std::string NAME_STATIC_PARAMETER_AIF;
    static const std::string NAME_STATIC_PARAMETER_AIFTimeGrid;

    static const std::string UNIT_STATIC_PARAMETER_AIF;
    static const std::string UNIT_STATIC_PARAMETER_AIFTimeGrid;

    /** Typedef for Aterial InputFunction AIF(t)*/
    typedef itk::Array<double> AterialInputFunctionType;

    itkGetConstReferenceMacro(AterialInputFunctionValues, AterialInputFunctionType);
    itkGetConstReferenceMacro(AterialInputFunctionTimeGrid, TimeGridType);

    itkSetMacro(AterialInputFunctionValues, AterialInputFunctionType);
    itkSetMacro(AterialInputFunctionTimeGrid, TimeGridType);

    std::string GetXAxisName() const override;

    std::string GetXAxisUnit() const override;

    std::string GetYAxisName() const override;

    std::string GetYAxisUnit() const override;

    /** Returns the TimeGrid used for the AIF. Either the externally set AIF time grid
     * or the time grid of the model if nothing is set.*/
    const TimeGridType& GetCurrentAterialInputFunctionTimeGrid() const;

    /** Returns the Aterial Input function matching currentTimeGrid
     *  The original values are interpolated to the passed TimeGrid
     * if currentTimeGrid.Size() = 0 , the Original AIF will be returned*/
    const AterialInputFunctionType GetAterialInputFunction(TimeGridType currentTimeGrid) const;

    ParameterNamesType GetStaticParameterNames() const override;
    ParametersSizeType GetNumberOfStaticParameters() const override;
    ParamterUnitMapType GetStaticParameterUnits() const override;


  protected:
    AIFBasedModelBase();
    ~AIFBasedModelBase() override;

    /** Reimplementation that checks if AIF and timegrid settings are valid.
     * @param [out] error Set internally to indicate the error reason if method returns false. Is used by GetSignal() for the
     * exception comment.
     * @return Returns true if the model is valid and can compute a signal. Otherwise it returns false.*/
    bool ValidateModel(std::string& error) const override;

    void PrintSelf(std::ostream& os, ::itk::Indent indent) const override;

    void SetStaticParameter(const ParameterNameType& name,
                                    const StaticParameterValuesType& values) override;

    StaticParameterValuesType GetStaticParameterValue(const ParameterNameType& name) const
    override;

    TimeGridType m_AterialInputFunctionTimeGrid;
    AterialInputFunctionType m_AterialInputFunctionValues;


  private:


    //No copy constructor allowed
    AIFBasedModelBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented


  };
}
#endif // AIFBASEDMODELBASE_H
