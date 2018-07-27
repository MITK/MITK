#ifndef MITKTHREESTEPLINEARMODELFACTORY_H
#define MITKTHREESTEPLINEARMODELFACTORY_H

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkThreeStepLinearModel.h"
#include "mitkThreeStepLinearModelParameterizer.h"

#include "MitkPharmacokineticsExports.h"
namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT ThreeStepLinearModelFactory : public ConcreteModelFactoryBase<ThreeStepLinearModel>
  {
  public:
    mitkClassMacroItkParent(ThreeStepLinearModelFactory, ConcreteModelFactoryBase<ThreeStepLinearModel>);
    itkFactorylessNewMacro(Self);

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:
    virtual ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const;

    ThreeStepLinearModelFactory();

    virtual ~ThreeStepLinearModelFactory();

  private:

    //No copy constructor allowed
    ThreeStepLinearModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}

#endif // MITKTHREESTEPLINEARMODELFACTORY_H
