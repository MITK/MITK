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
#include "mitkMapper2D.h"
#include "mitkPointSet.h"
#include "mitkGLMapper2D.h"
#include "mitkDataTree.h"

namespace mitk {

class BaseRenderer;

//##ModelId=3F0189F0014F
//##Documentation
//## @brief OpenGL-based mapper to display a Geometry2D in a 2D window
//## @ingroup Mapper
//## Currently implemented for mapping on PlaneGeometry. 
//## The result is normally a line. An important usage of this class is to show
//## the orientation of the slices displayed in other 2D windows.
//## @todo implement for AbstractTransformGeometry.
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

    //##ModelId=3F0189F00376
    virtual void Update();

protected:
    //##ModelId=3F0189F00378
    PointSetMapper2D();

    //##ModelId=3F0189F00382
    virtual ~PointSetMapper2D();

    //##ModelId=3F0189F00384
    virtual void GenerateOutputInformation();
};

} // namespace mitk



#endif /* MITKPointSetMapper2D_H_HEADER_INCLUDED */
