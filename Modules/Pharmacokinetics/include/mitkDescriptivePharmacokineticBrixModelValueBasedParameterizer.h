/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDescriptivePharmacokineticBrixModelValueBasedParameterizer_h
#define mitkDescriptivePharmacokineticBrixModelValueBasedParameterizer_h

#include <mitkConcreteModelParameterizerBase.h>

namespace mitk
{
  /**
   * \brief Parameterizer for the DescriptivePharmacokineticBrixModel using a single baseline value.
   *
   * Instead of a baseline image, this parameterizer uses a single scalar base value for the
   * pre-contrast signal S0, making it suitable for ROI-based fitting strategies where a
   * complete image is not needed.
   *
   * \sa DescriptivePharmacokineticBrixModel, DescriptivePharmacokineticBrixModelParameterizer
   */
  class MITKPHARMACOKINETICS_EXPORT DescriptivePharmacokineticBrixModelValueBasedParameterizer : public
    ConcreteModelParameterizerBase<mitk::DescriptivePharmacokineticBrixModel>
  {
  public:
    typedef DescriptivePharmacokineticBrixModelValueBasedParameterizer Self;
    typedef ConcreteModelParameterizerBase<mitk::DescriptivePharmacokineticBrixModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(DescriptivePharmacokineticBrixModelValueBasedParameterizer, ConcreteModelParameterizerBase);
    itkNewMacro(Self);

    typedef Superclass::ModelBaseType ModelBaseType;
    typedef Superclass::ModelBasePointer ModelBasePointer;

    typedef Superclass::ModelType ModelType;
    typedef Superclass::ModelPointer ModelPointer;

    typedef Superclass::StaticParameterValueType StaticParameterValueType;
    typedef Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef Superclass::StaticParameterMapType StaticParameterMapType;

    typedef itk::Image<double, 3> BaseImageType;

    typedef Superclass::IndexType IndexType;

    itkSetMacro(Tau, double);
    itkGetConstReferenceMacro(Tau, double);

    itkSetMacro(BaseValue, double);
    itkGetConstReferenceMacro(BaseValue, double);

    /* Returns the global static parameters for the model.
     * \remark this default implementation assumes no global static parameters exist.
     * Thus an empty map is returned.*/
    StaticParameterMapType GetGlobalStaticParameters() const override;

    /* Returns the local static parameters for the model at the given index.
     * \remark this default implementation assumes no local static parameters exist.
     * Thus an empty map is returned.*/
    StaticParameterMapType GetLocalStaticParameters(const IndexType& currentPosition) const override;

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    DescriptivePharmacokineticBrixModelValueBasedParameterizer();

    ~DescriptivePharmacokineticBrixModelValueBasedParameterizer() override;

    /**injection time Tau in minutes [min]*/
    double m_Tau;

    /** Contains the base value that should be used by the model.*/
    double m_BaseValue;

  private:

    //No copy constructor allowed
    DescriptivePharmacokineticBrixModelValueBasedParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif
