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


#ifndef _MITK_MITK_REGISTRATION_WRAPPER_MAPPER_3D_H
#define _MITK_MITK_REGISTRATION_WRAPPER_MAPPER_3D_H


#include "mitkRegistrationWrapperMapperBase.h"

#include "MitkMatchPointRegistrationExports.h"

class vtkPropAssembly;


namespace mitk {

/** Class for the 3D visualization of a registration object.*/
class MITKMATCHPOINTREGISTRATION_EXPORT MITKRegistrationWrapperMapper3D : public MITKRegistrationWrapperMapperBase
{
public:

    mitkClassMacro(MITKRegistrationWrapperMapper3D, MITKRegistrationWrapperMapperBase);
    itkNewMacro(Self);

    virtual bool GetGeometryDescription(mitk::BaseRenderer *renderer, mitk::BaseGeometry::ConstPointer& gridDesc, unsigned int& gridFrequ) const;
    virtual bool RendererGeometryIsOutdated(mitk::BaseRenderer *renderer, const itk::TimeStamp& time) const;

protected:

    MITKRegistrationWrapperMapper3D();
    virtual ~MITKRegistrationWrapperMapper3D();


private:

};

} // end namespace mitk




#endif /* MITKRegistrationWrapperMapper3D_H_HEADER_INCLUDED */

