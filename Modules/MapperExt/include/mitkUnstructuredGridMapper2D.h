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


#ifndef MitkPointSetSliceLMapper2D_H
#define MitkPointSetSliceLMapper2D_H

#include "mitkCommon.h"
#include "MitkMapperExtExports.h"
#include "mitkGLMapper.h"
#include "mitkTransferFunction.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkVtkScalarModeProperty.h"

class vtkPointSetSlicer;
class vtkPlane;
class vtkLookupTable;
class vtkPointSet;
class vtkScalarsToColors;
class vtkPiecewiseFunction;
class vtkAbstractMapper3D;

namespace mitk
{

class BaseRenderer;

/**
 * @brief OpenGL-based mapper to display a 2d cut through a poly data
 * OpenGL-based mapper to display a 2D cut through a poly data. The result is
 * normally a line. This class can be added to any data object, which is
 * rendered in 3D via a vtkPolyData.
 */
class MITKMAPPEREXT_EXPORT UnstructuredGridMapper2D : public GLMapper
{
public:

    mitkClassMacro( UnstructuredGridMapper2D, GLMapper );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * Renders a cut through a pointset by cutting trough the n-cells,
     * producing (n-1)-cells.
     * @param renderer the render to render in.
     */
    virtual void Paint( mitk::BaseRenderer* renderer ) override;

    LocalStorageHandler<BaseLocalStorage> m_LSH;

protected:

    UnstructuredGridMapper2D();

    virtual ~UnstructuredGridMapper2D();

    virtual void GenerateDataForRenderer(BaseRenderer*) override;

    /**
     * Determines, if the associated BaseData is mapped three-dimensionally (mapper-slot id 2)
     * with a class convertable to vtkAbstractMapper3D().
     * @returns NULL if it is not convertable or the appropriate Mapper otherwise
     */
    virtual vtkAbstractMapper3D* GetVtkAbstractMapper3D(BaseRenderer* renderer);

    /**
     * Determines the pointset object to be cut.
     * returns the pointset if possible, otherwise NULL.
     */
    virtual vtkPointSet* GetVtkPointSet(BaseRenderer* renderer, int time = 0 );

    /**
     * Determines the LookupTable used by the associated vtkMapper.
     * returns the LUT if possible, otherwise NULL.
     */
    virtual vtkScalarsToColors* GetVtkLUT(BaseRenderer* renderer );

    /**
     * Checks if this mapper can be used to generate cuts through the associated
     * base data.
     * @return true if yes or false if not.
     */
    virtual bool IsConvertibleToVtkPointSet(BaseRenderer* renderer);

    vtkPlane* m_Plane;
    vtkPointSetSlicer* m_Slicer;
    vtkPointSet* m_VtkPointSet;
    vtkScalarsToColors* m_ScalarsToColors;
    vtkPiecewiseFunction* m_ScalarsToOpacity;

    mitk::ColorProperty::Pointer m_Color;
    mitk::IntProperty::Pointer m_LineWidth;
    mitk::BoolProperty::Pointer m_Outline;
    mitk::BoolProperty::Pointer m_ScalarVisibility;
    mitk::VtkScalarModeProperty::Pointer m_ScalarMode;
};

} // namespace mitk
#endif /* MitkPointSetSliceLMapper2D_H */
