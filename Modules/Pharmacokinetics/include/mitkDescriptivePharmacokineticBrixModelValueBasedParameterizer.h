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

#ifndef __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_VALUEBASED_PARAMETERIZER_H
#define __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_VALUEBASED_PARAMETERIZER_H

#include "mitkConcreteModelParameterizerBase.h"

namespace mitk
{
  /** Parameterizer for the DescriptivePharmacokineticBrixModel that don't use an image
   for initializing the model but a single signal value. This parameterizer is amongst
   others used for ROI based fiting strategies where no complete image is needed/used.
   @sa DescriptivePharmacokineticBrixModelParameterizer*/
  class MITKPHARMACOKINETICS_EXPORT DescriptivePharmacokineticBrixModelValueBasedParameterizer : public
    ConcreteModelParameterizerBase<mitk::DescriptivePharmacokineticBrixModel>
  {
  public:
    typedef DescriptivePharmacokineticBrixModelValueBasedParameterizer Self;
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

    itkSetMacro(BaseValue, double);
    itkGetConstReferenceMacro(BaseValue, double);

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

    DescriptivePharmacokineticBrixModelValueBasedParameterizer();

    virtual ~DescriptivePharmacokineticBrixModelValueBasedParameterizer();

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

#endif // __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_VALUEBASED_PARAMETERIZER_H
