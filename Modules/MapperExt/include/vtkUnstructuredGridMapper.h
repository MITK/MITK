/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkUnstructuredGridMapper_h
#define vtkUnstructuredGridMapper_h

#include <MitkMapperExtExports.h>
#include <mitkBaseRenderer.h>
#include <mitkBoundingObject.h>
#include <mitkCommon.h>

#include <vtkMapper.h>

class vtkPolyDataMapper;
class vtkGeometryFilter;
class vtkUnstructuredGrid;

/** \brief VTK mapper that renders vtkUnstructuredGrid by converting to poly data.
 *
 * Internally uses a vtkGeometryFilter to extract the surface geometry from an
 * unstructured grid and renders it via a vtkPolyDataMapper. Optionally supports
 * clipping to a BoundingObject.
 *
 * \sa UnstructuredGridVtkMapper3D
 */
class MITKMAPPEREXT_EXPORT vtkUnstructuredGridMapper : public vtkMapper
{
public:
  /** \brief Create a new instance. */
  static vtkUnstructuredGridMapper *New();
  vtkTypeMacro(vtkUnstructuredGridMapper, vtkMapper);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /** \brief Render the unstructured grid.
   *
   * \param[in] ren The VTK renderer.
   * \param[in] act The VTK actor.
   */
  void Render(vtkRenderer *ren, vtkActor *act) override;

  /** \brief Get the internal poly data mapper used to render the extracted surface.
   * \return The internal vtkPolyDataMapper.
   */
  vtkGetObjectMacro(PolyDataMapper, vtkPolyDataMapper);

  /** \brief Release graphics resources consumed by this mapper.
   *
   * \param[in] window The render window (unused).
   * \deprecated Use ReleaseGraphicsResources(mitk::BaseRenderer*) instead.
   */
  DEPRECATED(void ReleaseGraphicsResources(vtkWindow *window) override);

  /** \brief Release graphics resources for the given renderer.
   *
   * \param[in] renderer The renderer whose resources should be released.
   */
  void ReleaseGraphicsResources(mitk::BaseRenderer *renderer);

  /** \brief Get the modification time, including the lookup table's mtime.
   * \return The combined modification time.
   */
  vtkMTimeType GetMTime() override;

  /** \brief Set the unstructured grid input.
   * \param[in] input The unstructured grid to render.
   */
  void SetInput(vtkUnstructuredGrid *input);

  /** \brief Get the unstructured grid input.
   * \return The current input grid.
   */
  vtkUnstructuredGrid *GetInput();

  /** \brief Set a bounding object for clipping the rendered grid.
   * \param[in] bo The bounding object, or \c nullptr to disable clipping.
   */
  void SetBoundingObject(mitk::BoundingObject *bo);

protected:
  vtkUnstructuredGridMapper();
  ~vtkUnstructuredGridMapper() override;

  vtkGeometryFilter *GeometryExtractor;
  vtkPolyDataMapper *PolyDataMapper;

  mitk::BoundingObject::Pointer m_BoundingObject;

  void ReportReferences(vtkGarbageCollector *) override;

  // see algorithm for more info
  int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  vtkUnstructuredGridMapper(const vtkUnstructuredGridMapper &); // Not implemented.
  void operator=(const vtkUnstructuredGridMapper &);            // Not implemented.
};

#endif // vtkUnstructuredGridMapper_h
