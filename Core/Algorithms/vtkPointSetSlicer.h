/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 10894 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _VTKPOINTSETSLICER_H_
#define _VTKPOINTSETSLICER_H_

#include <iostream>

class vtkCutter;
class vtkPlane;
class vtkPointLocator;
class vtkCell;
class vtkDataArray;
class vtkCellArray;
class vtkPointData;
class vtkCellData;


#if (VTK_MAJOR_VERSION < 5)
#include "vtkDataSetToPolyDataFilter.h"
#define VTK_NUMBER_OF_CELL_TYPES 68

class /*VTK_GRAPHICS_EXPORT*/ vtkPointSetSlicer : public vtkDataSetToPolyDataFilter
{
public:
  vtkTypeMacro(vtkPointSetSlicer,vtkDataSetToPolyDataFilter);
  
#else
#include "vtkPolyDataAlgorithm.h"

class /*VTK_GRAPHICS_EXPORT*/ vtkPointSetSlicer : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkPointSetSlicer,vtkPolyDataAlgorithm);
  
#endif
  
  void PrintSelf(std::ostream& os, vtkIndent indent);

  // Description:
  // Construct with user-specified implicit function; initial value of 0.0; and
  // generating cut scalars turned off.
  static vtkPointSetSlicer *New();

  // Description:
  // Override GetMTime because we delegate to vtkContourValues and refer to
  // vtkImplicitFunction.
  unsigned long GetMTime();

  // Description
  // Specify the implicit function to perform the cutting.
  virtual void SetSlicePlane(vtkPlane*);
  vtkGetObjectMacro(SlicePlane,vtkPlane);

  // Description:
  // If this flag is enabled, then the output scalar values will be
  // interpolated from the implicit function values, and not the input scalar
  // data.
  vtkSetMacro(GenerateCutScalars,int);
  vtkGetMacro(GenerateCutScalars,int);
  vtkBooleanMacro(GenerateCutScalars,int);

  // Description:
  // Specify a spatial locator for merging points. By default, 
  // an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified. The 
  // locator is used to merge coincident points.
  void CreateDefaultLocator();

  // Description:
  // Normally I would put this in a different class, but since
  // This is a temporary fix until we convert this class and contour filter
  // to generate unstructured grid output instead of poly data, I am leaving it here.
  static void GetCellTypeDimensions(unsigned char* cellTypeDimensions);

protected:
  vtkPointSetSlicer(vtkPlane* cf = 0);
  ~vtkPointSetSlicer();

#if (VTK_MAJOR_VERSION >= 5)
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);
#else
  virtual void Execute();
#endif
  
  void UnstructuredGridCutter(vtkDataSet *input, vtkPolyData *output);
  
  void ContourUnstructuredGridCell(vtkCell* cell,
      vtkDataArray* cellScalars, vtkPointLocator* locator,
      vtkCellArray* verts, vtkCellArray* lines,
      vtkCellArray* polys, vtkPointData* inPd,
      vtkPointData* outPd, vtkCellData* inCd,
      vtkIdType cellId, vtkCellData* outCd);
  
  vtkPlane *SlicePlane;
  vtkCutter *Cutter;

  vtkPointLocator *Locator;
  int GenerateCutScalars;
  
private:
  vtkPointSetSlicer(const vtkPointSetSlicer&);  // Not implemented.
  void operator=(const vtkPointSetSlicer&);  // Not implemented.
  
  static int edges[12][2];
  
  typedef int EDGE_LIST;
  typedef struct {
    EDGE_LIST edges[8];
  } POLY_CASES;
  
  static POLY_CASES polyCases[256];
};


#endif /* _VTKPOINTSETSLICER_H_ */
