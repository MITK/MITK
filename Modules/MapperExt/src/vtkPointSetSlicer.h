/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _VTKPOINTSETSLICER_H_
#define _VTKPOINTSETSLICER_H_

#include <iostream>

#include "vtkVersion.h"

class vtkCutter;
class vtkPlane;
class vtkPointLocator;
class vtkCell;
class vtkDataArray;
class vtkCellArray;
class vtkPointData;
class vtkCellData;

#include "mitkCommon.h"

#include "vtkPolyDataAlgorithm.h"

class vtkPointSetSlicer : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkPointSetSlicer, vtkPolyDataAlgorithm);

  void PrintSelf(std::ostream &os, vtkIndent indent) override;

  // Description:
  // Construct with user-specified implicit function; initial value of 0.0; and
  // generating cut scalars turned off.
  static vtkPointSetSlicer *New();

  // Description:
  // Override GetMTime because we delegate to vtkContourValues and refer to
  // vtkImplicitFunction.
  vtkMTimeType GetMTime() override;

  // Description
  // Specify the implicit function to perform the cutting.
  virtual void SetSlicePlane(vtkPlane *);
  vtkGetObjectMacro(SlicePlane, vtkPlane);

  // Description:
  // If this flag is enabled, then the output scalar values will be
  // interpolated from the implicit function values, and not the input scalar
  // data.
  vtkSetMacro(GenerateCutScalars, int);
  vtkGetMacro(GenerateCutScalars, int);
  vtkBooleanMacro(GenerateCutScalars, int);

  // Description:
  // Specify a spatial locator for merging points. By default,
  // an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator, vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified. The
  // locator is used to merge coincident points.
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

#endif /* _VTKPOINTSETSLICER_H_ */
