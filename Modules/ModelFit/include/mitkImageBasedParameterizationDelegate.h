/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIMAGEBASEDPARAMETERIZATIONDELEGATE_H
#define MITKIMAGEBASEDPARAMETERIZATIONDELEGATE_H

#include "mitkValueBasedParameterizationDelegate.h"
#include <itkIndex.h>
#include <itkObject.h>

#include "mitkModelBase.h"
#include "mitkImage.h"
#include "mitkModelTraitsInterface.h"

#include "MitkModelFitExports.h"

namespace mitk {

class MITKMODELFIT_EXPORT ImageBasedParameterizationDelegate : public ValueBasedParameterizationDelegate
{
public:

    typedef ImageBasedParameterizationDelegate Self;
    typedef ValueBasedParameterizationDelegate Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    itkTypeMacro(ImageBasedParameterizationDelegate, InitialParameterizationDelegateBase);

    typedef Superclass::ModelBaseType ModelBaseType;

    typedef Superclass::ParametersType ParametersType;

    typedef Superclass::IndexType IndexType;

    /** Returns the parameterization (e.g. initial parametrization for fitting) that should be used.
   If no ParameterizationDelegate is set (see SetInitialParameterizationDelegate()) it will just return
   the result of GetInitialParameterization().*/
    ParametersType GetInitialParameterization() const override;
    ParametersType GetInitialParameterization(const IndexType& currentPosition) const override;

    /** Adds an image as a source for the initial value of a parameter.
     * @param image Pointer to the image that is the value source.
     * @param paramIndex Indicates which parameter is defined by the source image.
     * It equals the position in the return vector of GetInitialParameterization().
     * @remark setting an image for an index overwrites the value for this index set by
     * SetInitialParameterization.
     * @pre paramIndex must be in bound of the initial parametrization vector.
     * @pre image must be a valid instance*/
    void AddInitialParameterImage(const mitk::Image* image, ParametersType::size_type paramIndex);
protected:

    typedef std::map<ParametersType::size_type, mitk::Image::ConstPointer> ImageMapType;
    ImageMapType m_ParameterImageMap;

    ImageBasedParameterizationDelegate();

    ~ImageBasedParameterizationDelegate() override;

private:

    //No copy constructor allowed
    ImageBasedParameterizationDelegate(const Self& source);
    void operator=(const Self&);  //purposely not implemented
};

}
#endif // MITKTWOCXINITIALPARAMETERIZATIONDELEGATE_H
