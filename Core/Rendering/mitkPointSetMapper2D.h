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
//## @brief OpenGL-based mapper to display a mitk::PointSet in a 2D window.
//##
//## This mapper can actually more than just draw a number of points of a mitk::PointSet.
//## If you set the right properties of the mitk::DataTreeNode, which contains the point set,
//## then this mapper will also draw lines connecting the points, and calculate and display distances
//## and angles between adjacent points. Here is a complete list of boolean properties, which might be of interest:
//##
//## - \b "contour": Draw not only the points but also the connections between them (default false)
//## - \b "show points": Wheter or not to draw the actual points (default true)
//## - \b "show distances": Wheter or not to calculate and print the distance between adjacent points (default false)
//## - \b "show angles": Wheter or not to calculate and print the angle between adjacent points (default false)
//## - \b "show distant lines": When true, the mapper will also draw contour lines that are far away form the current slice (default true)
//## - \b "label": StringProperty with a label for this point set
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
    bool m_ShowPoints;
    bool m_ShowDistances;
    bool m_ShowAngles;
    bool m_ShowDistantLines;
};

} // namespace mitk



#endif /* MITKPointSetMapper2D_H_HEADER_INCLUDED */
