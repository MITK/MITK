/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef _MITK_MITK_REGISTRATION_WRAPPER_MAPPER_2D_H
#define _MITK_MITK_REGISTRATION_WRAPPER_MAPPER_2D_H


#include "mitkRegistrationWrapperMapperBase.h"

#include "MitkMatchPointRegistrationExports.h"

class vtkPropAssembly;


namespace mitk {

/** Class for the 2D visualization of a registration object.*/
class MITKMATCHPOINTREGISTRATION_EXPORT MITKRegistrationWrapperMapper2D : public MITKRegistrationWrapperMapperBase
{
public:

    mitkClassMacro(MITKRegistrationWrapperMapper2D, MITKRegistrationWrapperMapperBase);
    itkNewMacro(Self);

    bool GetGeometryDescription(mitk::BaseRenderer *renderer, mitk::BaseGeometry::ConstPointer& gridDesc, unsigned int& gridFrequ) const override;
    bool RendererGeometryIsOutdated(mitk::BaseRenderer *renderer, const itk::TimeStamp& time) const override;

protected:

    MITKRegistrationWrapperMapper2D();
    ~MITKRegistrationWrapperMapper2D() override;


private:

};

} // end namespace mitk




#endif /* MITKRegistrationWrapperMapper2D_H_HEADER_INCLUDED */

