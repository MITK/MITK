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


#ifndef MITK_CONTOUR_SET_MAPPER_2D_H_
#define MITK_CONTOUR_SET_MAPPER_2D_H_

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkGLMapper.h"

namespace mitk {

class BaseRenderer;
class ContourSet;

/**
 * @brief OpenGL-based mapper to display a mitk::Contour object in a 2D render window
 *
 *
 * @ingroup Mapper
 */
class MITKSEGMENTATION_EXPORT ContourSetMapper2D : public GLMapper
{
public:

    mitkClassMacro(ContourSetMapper2D, GLMapper);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * reimplemented from Baseclass
     */
    virtual void Paint(mitk::BaseRenderer * renderer) override;

    /**
     * return a refernce of the rendered data object
     */
    const mitk::ContourSet * GetInput(void);

protected:
    ContourSetMapper2D();

    virtual ~ContourSetMapper2D();
};

} // namespace mitk



#endif /* MITK_CONTOUR_SET_MAPPER_2D_H_ */
