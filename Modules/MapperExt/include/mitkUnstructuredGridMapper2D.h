/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGridMapper2D_h
#define mitkUnstructuredGridMapper2D_h

#include <MitkMapperExtExports.h>
#include <mitkColorProperty.h>
#include <mitkCommon.h>
#include <mitkGLMapper.h>
#include <mitkProperties.h>
#include <mitkTransferFunction.h>
#include <mitkVtkScalarModeProperty.h>

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

  /** \brief OpenGL-based mapper for rendering 2D cross-sections of unstructured grids.
   *
   * Displays a 2D cut through an unstructured grid or poly data by slicing
   * through the n-cells, producing (n-1)-cells (typically lines). This mapper
   * can be used with any data object that has a 3D VTK mapper producing a
   * vtkPolyData or vtkPointSet.
   *
   * \sa UnstructuredGridVtkMapper3D, GLMapper
   * \ingroup Mapper
   */
  class MITKMAPPEREXT_EXPORT UnstructuredGridMapper2D : public GLMapper
  {
  public:
    mitkClassMacro(UnstructuredGridMapper2D, GLMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Render a 2D cross-section of the unstructured grid.
     *
     * Cuts through the n-cells with the current slice plane, producing
     * (n-1)-cells which are rendered as lines.
     *
     * \param[in] renderer The renderer to paint into.
     */
    void Paint(mitk::BaseRenderer *renderer) override;

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    UnstructuredGridMapper2D();

    ~UnstructuredGridMapper2D() override;

    void GenerateDataForRenderer(BaseRenderer *) override;

    /**
     * Determines, if the associated BaseData is mapped three-dimensionally (mapper-slot id 2)
     * with a class convertable to vtkAbstractMapper3D().
     * @returns nullptr if it is not convertable or the appropriate Mapper otherwise
     */
    virtual vtkAbstractMapper3D *GetVtkAbstractMapper3D(BaseRenderer *renderer);

    /**
     * Determines the pointset object to be cut.
     * returns the pointset if possible, otherwise nullptr.
     */
    virtual vtkPointSet *GetVtkPointSet(BaseRenderer *renderer, int time = 0);

    /**
     * Determines the LookupTable used by the associated vtkMapper.
     * returns the LUT if possible, otherwise nullptr.
     */
    virtual vtkScalarsToColors *GetVtkLUT(BaseRenderer *renderer);

    /**
     * Checks if this mapper can be used to generate cuts through the associated
     * base data.
     * @return true if yes or false if not.
     */
    virtual bool IsConvertibleToVtkPointSet(BaseRenderer *renderer);

    vtkPlane *m_Plane;
    vtkPointSetSlicer *m_Slicer;
    vtkPointSet *m_VtkPointSet;
    vtkScalarsToColors *m_ScalarsToColors;
    vtkPiecewiseFunction *m_ScalarsToOpacity;

    mitk::ColorProperty::Pointer m_Color;
    mitk::IntProperty::Pointer m_LineWidth;
    mitk::BoolProperty::Pointer m_Outline;
    mitk::BoolProperty::Pointer m_ScalarVisibility;
    mitk::VtkScalarModeProperty::Pointer m_ScalarMode;
  };

} // namespace mitk
#endif
