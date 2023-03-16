/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDescriptivePharmacokineticBrixModelParameterizer_h
#define mitkDescriptivePharmacokineticBrixModelParameterizer_h

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkDescriptivePharmacokineticBrixModel.h"

namespace mitk
{
  /** Parameterizer for the DescriptivePharmacokineticBrixModel that use an image
   for initializing the model. This parameterizer is amongst others used for pixel based fiting
   strategies.
   @sa DescriptivePharmacokineticBrixModelParameterizer*/
  class MITKPHARMACOKINETICS_EXPORT DescriptivePharmacokineticBrixModelParameterizer : public
    ConcreteModelParameterizerBase<mitk::DescriptivePharmacokineticBrixModel>
  {
  public:
    typedef DescriptivePharmacokineticBrixModelParameterizer Self;
    typedef ConcreteModelParameterizerBase<mitk::DescriptivePharmacokineticBrixModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ConcreteModelParameterizerBase, ModelParameterizerBase);
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

    itkSetConstObjectMacro(BaseImage, BaseImageType);
    itkGetConstObjectMacro(BaseImage, BaseImageType);

    /* Returns the global static parameters for the model.
     * @remark this default implementation assumes no global static parameters exist.
     * Thus an empty map is returned.*/
    StaticParameterMapType GetGlobalStaticParameters() const override;

    /* Returns the local static parameters for the model at the given index.
     * @remark this default implementation assumes no local static parameters exist.
     * Thus an empty map is returned.*/
    StaticParameterMapType GetLocalStaticParameters(const IndexType& currentPosition) const override;

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:

    DescriptivePharmacokineticBrixModelParameterizer();

    ~DescriptivePharmacokineticBrixModelParameterizer() override;

    /**injection time Tau in minutes [min]*/
    double m_Tau;

    /**Pointer to the image that containes the values of the first timestep
    (base value of the series that should be modelled)*/
    BaseImageType::ConstPointer m_BaseImage;

  private:

    //No copy constructor allowed
    DescriptivePharmacokineticBrixModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif
