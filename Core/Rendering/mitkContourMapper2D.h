/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITK_CONTOUR_MAPPER_2D_H_
#define MITK_CONTOUR_MAPPER_2D_H_

#include "mitkCommon.h"
#include "mitkMapper2D.h"
#include "mitkContour.h"
#include "mitkGLMapper2D.h"
#include "mitkDataTree.h"

namespace mitk {

class BaseRenderer;

/** 
 * @brief OpenGL-based mapper to display a mitk::Contour object in a 2D render window 
 * @ingroup Mapper
 * 
 * 
 */
class ContourMapper2D : public GLMapper2D
{
public:
    
    mitkClassMacro(ContourMapper2D, Mapper2D);

    itkNewMacro(Self);

    /**
     * reimplemented from Baseclass
     */
    virtual void Paint(mitk::BaseRenderer * renderer);

    /**
     * return a refernce of the rendered data object 
     */
    const mitk::Contour * GetInput(void);

    /**
     * reimplemented from Baseclass
     */
    virtual void Update();

protected:
    ContourMapper2D();

    virtual ~ContourMapper2D();

    virtual void GenerateOutputInformation();
};

} // namespace mitk



#endif /* MITKContourMapper2D_H_HEADER_INCLUDED */
