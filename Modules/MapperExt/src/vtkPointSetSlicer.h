/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef vtkPointSetSlicer_h
#define vtkPointSetSlicer_h

#include <iostream>

#include <vtkVersion.h>

class vtkCutter;
class vtkPlane;
class vtkPointLocator;
class vtkCell;
class vtkDataArray;
class vtkCellArray;
class vtkPointData;
class vtkCellData;

#include <mitkCommon.h>

#include <vtkPolyDataAlgorithm.h>

/**
 * \brief VTK filter that slices a point set (unstructured grid) with a plane.
 *
 * Produces polydata representing the intersection of the input unstructured
 * grid with a vtkPlane. Used internally by mitk::EnhancedPointSetVtkMapper3D
 * to visualize point set cross sections on 2D slices.
 */
class vtkPointSetSlicer : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkPointSetSlicer, vtkPolyDataAlgorithm);

  /** \brief Print object state to the given stream. */
  void PrintSelf(std::ostream &os, vtkIndent indent) override;

  /**
   * \brief Construct a new vtkPointSetSlicer instance.
   *
   * Initial value of 0.0 and generating cut scalars turned off.
   */
  static vtkPointSetSlicer *New();

  /** \brief Return the modification time, considering the slice plane. */
  vtkMTimeType GetMTime() override;

  /**
   * \brief Set the plane used for slicing.
   * \param plane The vtkPlane to slice with.
   */
  virtual void SetSlicePlane(vtkPlane *plane);
  vtkGetObjectMacro(SlicePlane, vtkPlane);

  /**
   * \brief Enable/disable generation of cut scalars.
   *
   * If enabled, output scalar values are interpolated from the implicit
   * function values rather than the input scalar data.
   */
  vtkSetMacro(GenerateCutScalars, int);
  vtkGetMacro(GenerateCutScalars, int);
  vtkBooleanMacro(GenerateCutScalars, int);

  /**
   * \brief Set a spatial locator for merging coincident points.
   *
   * By default, an instance of vtkMergePoints is used.
   *
   * \param locator The point locator to use.
   */
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator, vtkPointLocator);

  /**
   * \brief Create a default locator (vtkMergePoints).
   *
   * Used when none has been explicitly specified.
   */
  void CreateDefaultLocator();

protected:
  vtkPointSetSlicer(vtkPlane *cf = nullptr);
  ~vtkPointSetSlicer() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int FillInputPortInformation(int port, vtkInformation *info) override;

  void UnstructuredGridCutter(vtkDataSet *input, vtkPolyData *output);

  void ContourUnstructuredGridCell(vtkCell *cell,
                                   vtkDataArray *cellScalars,
                                   vtkPointLocator *locator,
                                   vtkCellArray *verts,
                                   vtkCellArray *lines,
                                   vtkCellArray *polys,
                                   vtkPointData *inPd,
                                   vtkPointData *outPd,
                                   vtkCellData *inCd,
                                   vtkIdType cellId,
                                   vtkCellData *outCd);

  vtkPlane *SlicePlane;
  vtkCutter *Cutter;

  vtkPointLocator *Locator;
  int GenerateCutScalars;

private:
  vtkPointSetSlicer(const vtkPointSetSlicer &); // Not implemented.
  void operator=(const vtkPointSetSlicer &);    // Not implemented.

  static int edges[12][2];

  typedef int EDGE_LIST;
  typedef struct
  {
    EDGE_LIST edges[8];
  } POLY_CASES;

  static POLY_CASES polyCases[256];
};

#endif /* vtkPointSetSlicer_h */
