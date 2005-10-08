/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKPointSetMAPPER2D_H_HEADER_INCLUDED
#define MITKPointSetMAPPER2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"

namespace mitk {

class BaseRenderer;
class PointSet;

//##ModelId=3F0189F0014F
//##Documentation
//## @brief OpenGL-based mapper to display a mitk::PointSet in a 2D window
//##
//## @ingroup Mapper
class PointSetMapper2D : public GLMapper2D
{
public:
    //##ModelId=3F0189F00363
    mitkClassMacro(PointSetMapper2D, Mapper2D);

    //##ModelId=3F0189F00354
    itkNewMacro(Self);

    //##ModelId=3F0189F00366
    //##Documentation
    //## @brief Get the PointDataList to map
    const mitk::PointSet * GetInput(void);

    //##ModelId=3F0189F00373
    virtual void Paint(mitk::BaseRenderer * renderer);

    virtual void ApplyProperties(mitk::BaseRenderer* renderer);

protected:
    //##ModelId=3F0189F00378
    PointSetMapper2D();

    //##ModelId=3F0189F00382
    virtual ~PointSetMapper2D();

    bool m_Polygon;
};

} // namespace mitk



#endif /* MITKPointSetMapper2D_H_HEADER_INCLUDED */
