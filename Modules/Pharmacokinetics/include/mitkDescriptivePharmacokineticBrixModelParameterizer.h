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

#ifndef __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_PARAMETERIZER_H
#define __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_PARAMETERIZER_H

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
    virtual StaticParameterMapType GetGlobalStaticParameters() const;

    /* Returns the local static parameters for the model at the given index.
     * @remark this default implementation assumes no local static parameters exist.
     * Thus an empty map is returned.*/
    virtual StaticParameterMapType GetLocalStaticParameters(const IndexType& currentPosition) const;

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:

    DescriptivePharmacokineticBrixModelParameterizer();

    virtual ~DescriptivePharmacokineticBrixModelParameterizer();

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

#endif // __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_PARAMETERIZER_H
