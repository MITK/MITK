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


#ifndef MitkPolyDataGLMapper2D_H
#define MitkPolyDataGLMapper2D_H

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"

class vtkCutter;
class vtkPlane;
class vtkLookupTable;
class vtkPolyData;
class vtkScalarsToColors;
class vtkPolyDataMapper;

namespace mitk
{

class BaseRenderer;

/**
 * @brief OpenGL-based mapper to display a 2d cut through a poly data
 * OpenGL-based mapper to display a 2D cut through a poly data. The result is
 * normally a line. This class can be added to any data object, which is 
 * rendered in 3D via a vtkPolyData.
 */
class MITK_CORE_EXPORT PolyDataGLMapper2D : public GLMapper2D
{
public:

    mitkClassMacro( PolyDataGLMapper2D, GLMapper2D );

    itkNewMacro( Self );

    /**
     * Sets if the cut lines are colored by mapping cell data
     */
    itkSetMacro( ColorByCellData, bool );

    /**
     * Sets if the cut lines are colored by mapping point data
     */
    itkSetMacro( ColorByPointData, bool );

    /**
     * Renders a cut through a poly-data by lines.
     * @param renderer the render to render in.
     */
    virtual void Paint( mitk::BaseRenderer * renderer );

protected:

    PolyDataGLMapper2D();

    virtual ~PolyDataGLMapper2D();

    /**
     * Determines, if the associated BaseData is mapped three-dimensionally (mapper-slot id 2)
     * with a class convertable to vtkPolyDataMapper().
     * @returns NULL if it is not convertable or the appropriate PolyDataMapper otherwise
     */
    virtual vtkPolyDataMapper* GetVtkPolyDataMapper();

    /**
     * Determines the poly data object to be cut.
     * returns the poly data if possible, otherwise NULL.
     */
    virtual vtkPolyData* GetVtkPolyData( );

    /**
     * Determines the LookupTable used by the associated vtkMapper.
     * returns the LUT if possible, otherwise NULL.
     */
    virtual vtkScalarsToColors* GetVtkLUT( );

    /**
     * Checks if this mapper can be used to generate cuts through the associated 
     * base data.
     * @return true if yes or false if not.
     */
    virtual bool IsConvertibleToVtkPolyData();

    vtkPlane* m_Plane;

    vtkCutter* m_Cutter;

    bool m_ColorByCellData;

    bool m_ColorByPointData;

};

} // namespace mitk
#endif /* MitkPolyDataGLMapper2D_H */
