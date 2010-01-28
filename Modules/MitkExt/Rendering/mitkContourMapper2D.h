/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITK_CONTOUR_MAPPER_2D_H_
#define MITK_CONTOUR_MAPPER_2D_H_

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkGLMapper2D.h"
#include "mitkDataTree.h"

namespace mitk {

class BaseRenderer;
class Contour;

/** 
 * @brief OpenGL-based mapper to display a mitk::Contour object in a 2D render window 
 * 
 * 
 * @ingroup Mapper
 */
class MitkExt_EXPORT ContourMapper2D : public GLMapper2D
{
public:
    
    mitkClassMacro(ContourMapper2D, Mapper2D);

    itkNewMacro(Self);

    /**
     * reimplemented from Baseclass
     */
    virtual void Paint(BaseRenderer * renderer);

    /**
     * return a refernce of the rendered data object 
     */
    const Contour* GetInput(void);

protected:
    ContourMapper2D();

    virtual ~ContourMapper2D();
};

} // namespace mitk



#endif /* MITKContourMapper2D_H_HEADER_INCLUDED */
