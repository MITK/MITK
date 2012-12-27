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
#include "SegmentationExports.h"
#include "mitkGLMapper2D.h"

namespace mitk {

class BaseRenderer;
class ContourSet;
class BaseRenderer;

/**
 * @brief OpenGL-based mapper to display a mitk::Contour object in a 2D render window
 *
 *
 * @ingroup Mapper
 */
class Segmentation_EXPORT ContourSetMapper2D : public GLMapper2D
{
public:

    mitkClassMacro(ContourSetMapper2D, Mapper2D);

    itkNewMacro(Self);

    /**
     * reimplemented from Baseclass
     */
    virtual void Paint(mitk::BaseRenderer * renderer);

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
