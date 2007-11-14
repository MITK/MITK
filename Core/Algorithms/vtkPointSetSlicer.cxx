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
#include <vector>
#include <algorithm>

#include "vtkPointSetSlicer.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPlane.h"
#include "vtkCutter.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnstructuredGrid.h"

#if (VTK_MAJOR_VERSION >= 5)
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#endif

vtkStandardNewMacro(vtkPointSetSlicer);

// Construct with user-specified implicit function; initial value of 0.0; and
// generating cut scalars turned off.
vtkPointSetSlicer::vtkPointSetSlicer(vtkPlane *cf)
{
  this->SlicePlane = cf;
  this->GenerateCutScalars = 0;
  this->Locator = 0;

  this->Cutter = vtkCutter::New();
  this->Cutter->GenerateValues( 1, 0, 1 );
}

vtkPointSetSlicer::~vtkPointSetSlicer()
{
  this->SetSlicePlane(0);
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }

  this->Cutter->Delete();
}

void vtkPointSetSlicer::SetSlicePlane(vtkPlane* plane)
{
  if ( this->SlicePlane == plane ) 
  {
    return;
  }
  if ( this->SlicePlane )
  {
    this->SlicePlane->UnRegister(this);
    this->SlicePlane = 0;
  }
  if ( plane )
  {
    plane->Register(this);
    this->Cutter->SetCutFunction(plane);
  }
  this->SlicePlane = plane;
  this->Modified();
}

// Overload standard modified time function. If cut functions is modified,
// or contour values modified, then this object is modified as well.
unsigned long vtkPointSetSlicer::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long time;

  if ( this->SlicePlane != 0 )
  {
    time = this->SlicePlane->GetMTime();
    mTime = ( time > mTime ? time : mTime );
  }

  if ( this->Locator != 0 )
  {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
  }

  return mTime;
}

#if (VTK_MAJOR_VERSION >= 5)
int vtkPointSetSlicer::RequestData(
  vtkInformation * /*request*/,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro(<< "Executing cutter");

  if (!this->SlicePlane)
    {
    vtkErrorMacro("No slice plane specified");
    return 0;
    }

  if ( input->GetNumberOfPoints() < 1 )
    {
    return 1;
    }

  if (input->GetDataObjectType() == VTK_STRUCTURED_POINTS ||
      input->GetDataObjectType() == VTK_IMAGE_DATA)
    {    
    if ( input->GetCell(0) && input->GetCell(0)->GetCellDimension() >= 3 )
      {
      //this->StructuredPointsCutter(input, output, request, inputVector, outputVector);
      return 1;
      }
    }
  if (input->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
    if (input->GetCell(0))
      {
      int dim = input->GetCell(0)->GetCellDimension();
      // only do 3D structured grids (to be extended in the future)
      if (dim >= 3)
        {
        //this->StructuredGridCutter(input, output);
        return 1;
        }
      }
    }
  if (input->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
    
      //this->RectilinearGridCutter(input, output);
      return 1;
      
    }

  if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    { 
    vtkDebugMacro(<< "Executing Unstructured Grid Cutter");   
    this->UnstructuredGridCutter(input, output);
    }
  else
    {
    vtkDebugMacro(<< "Executing DataSet Cutter");
    //this->DataSetCutter(input, output);
    }

  return 1;
}
int vtkPointSetSlicer::RequestUpdateExtent(
  vtkInformation *,
  vtkInformationVector **inputVector,
  vtkInformationVector *)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
  return 1;
}
int vtkPointSetSlicer::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}
#else
void vtkPointSetSlicer::Exectue()
{
  vtkDebugMacro(<< "Executing cutter");

  vtkDataSet *input = this->GetInput();
  vtkPolyData *output = this->GetOutput();

  if (!input)
  {
    vtkErrorMacro("No input specified");
    return;
  }

  if (!this->SlicePlane)
  {
    vtkErrorMacro("No slice plane specified");
    return;
  }

  if ( input->GetNumberOfPoints() < 1 )
  {
    vtkErrorMacro("Input data set is empty");
    return;
  }

  if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
  { 
    vtkDebugMacro(<< "Executing Unstructured Grid Cutter");   
    this->UnstructuredGridCutter(input, output);
  }
  else
  {
    vtkDebugMacro(<< "Executing DataSet Cutter");
    //this->DataSetCutter(input, output);
  }
}
#endif

void vtkPointSetSlicer::GetCellTypeDimensions(unsigned char* cellTypeDimensions)
{
  // Assume most cells will be 3d.
  memset(cellTypeDimensions, 3, VTK_NUMBER_OF_CELL_TYPES);
  cellTypeDimensions[VTK_EMPTY_CELL] = 0;
  cellTypeDimensions[VTK_VERTEX] = 0;
  cellTypeDimensions[VTK_POLY_VERTEX] = 0;
  cellTypeDimensions[VTK_LINE] = 1;
  cellTypeDimensions[VTK_POLY_LINE] = 1;
  cellTypeDimensions[VTK_QUADRATIC_EDGE] = 1;
  cellTypeDimensions[VTK_PARAMETRIC_CURVE] = 1;
  cellTypeDimensions[VTK_HIGHER_ORDER_EDGE] = 1;
  cellTypeDimensions[VTK_TRIANGLE] = 2;
  cellTypeDimensions[VTK_TRIANGLE_STRIP] = 2;
  cellTypeDimensions[VTK_POLYGON] = 2;
  cellTypeDimensions[VTK_PIXEL] = 2;
  cellTypeDimensions[VTK_QUAD] = 2;
  cellTypeDimensions[VTK_QUADRATIC_TRIANGLE] = 2;
  cellTypeDimensions[VTK_QUADRATIC_QUAD] = 2;
  cellTypeDimensions[VTK_PARAMETRIC_SURFACE] = 2;
  cellTypeDimensions[VTK_PARAMETRIC_TRI_SURFACE] = 2;
  cellTypeDimensions[VTK_PARAMETRIC_QUAD_SURFACE] = 2;
  cellTypeDimensions[VTK_HIGHER_ORDER_TRIANGLE] = 2;
  cellTypeDimensions[VTK_HIGHER_ORDER_QUAD] = 2;
#if (VTK_MAJOR_VERSION >= 5)
  cellTypeDimensions[VTK_HIGHER_ORDER_POLYGON] = 2;
#endif
}


void vtkPointSetSlicer::UnstructuredGridCutter(vtkDataSet *input, vtkPolyData *output)
{
  vtkIdType cellId, i;
  vtkDoubleArray *cellScalars;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkDoubleArray *cutScalars;
  double s;
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkIdType cellArrayIt = 0;
  int numCellPts;
  vtkPointData *inPD, *outPD;
  vtkCellData *inCD=input->GetCellData(), *outCD=output->GetCellData();
  vtkIdList *cellIds;
  int abortExecute = 0;

  double range[2];

  // Create objects to hold output of contour operation
  //
  estimatedSize = (vtkIdType) pow ((double) numCells, .75);
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
  {
    estimatedSize = 1024;
  }

  newPoints = vtkPoints::New();
  newPoints->Allocate(estimatedSize,estimatedSize/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);
  cutScalars = vtkDoubleArray::New();
  cutScalars->SetNumberOfTuples(numPts);

  // Interpolate data along edge. If generating cut scalars, do necessary setup
  if ( this->GenerateCutScalars )
  {
    inPD = vtkPointData::New();
    inPD->ShallowCopy(input->GetPointData());//copies original attributes
    inPD->SetScalars(cutScalars);
  }
  else 
  {
    inPD = input->GetPointData();
  }
  outPD = output->GetPointData();
  outPD->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);
  outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL )
  {
    this->CreateDefaultLocator();
  }
  this->Locator->InitPointInsertion (newPoints, input->GetBounds());

  // Loop over all points evaluating scalar function at each point
  //
  for ( i=0; i < numPts; i++ )
  {
    s = this->SlicePlane->FunctionValue(input->GetPoint(i));
    cutScalars->SetComponent(i,0,s);
  }

  // Compute some information for progress methods
  //
  vtkIdType numCuts = numCells;
  vtkIdType progressInterval = numCuts/20 + 1;
  int cut=0;

  vtkUnstructuredGrid *grid = (vtkUnstructuredGrid *)input;
  vtkIdType *cellArrayPtr = grid->GetCells()->GetPointer();
  double *scalarArrayPtr = cutScalars->GetPointer(0);
  double tempScalar;
  cellScalars = cutScalars->NewInstance();
  cellScalars->SetNumberOfComponents(cutScalars->GetNumberOfComponents());
  cellScalars->Allocate(VTK_CELL_SIZE*cutScalars->GetNumberOfComponents());


  // Three passes over the cells to process lower dimensional cells first.
  // For poly data output cells need to be added in the order:
  // verts, lines and then polys, or cell data gets mixed up.
  // A better solution is to have an unstructured grid output.
  // I create a table that maps cell type to cell dimensionality,
  // because I need a fast way to get cell dimensionality.
  // This assumes GetCell is slow and GetCellType is fast.
  // I do not like hard coding a list of cell types here, 
  // but I do not want to add GetCellDimension(vtkIdType cellId)
  // to the vtkDataSet API.  Since I anticipate that the output
  // will change to vtkUnstructuredGrid.  This temporary solution 
  // is acceptable.
  //
  int cellType;
  unsigned char cellTypeDimensions[VTK_NUMBER_OF_CELL_TYPES];
  vtkCutter::GetCellTypeDimensions(cellTypeDimensions);
  int dimensionality;
  // We skip 0d cells (points), because they cannot be cut (generate no data).
  for (dimensionality = 1; dimensionality <= 3; ++dimensionality)
  {
    // Loop over all cells; get scalar values for all cell points
    // and process each cell.
    //
    cellArrayIt = 0;
    for (cellId=0; cellId < numCells && !abortExecute; cellId++)
    {
      numCellPts = cellArrayPtr[cellArrayIt];
      // I assume that "GetCellType" is fast.
      cellType = input->GetCellType(cellId);
      if (cellType >= VTK_NUMBER_OF_CELL_TYPES)
      { // Protect against new cell types added.
        vtkErrorMacro("Unknown cell type " << cellType);
        cellArrayIt += 1+numCellPts;
        continue;
      }
      if (cellTypeDimensions[cellType] != dimensionality)
      {
        cellArrayIt += 1+numCellPts;
        continue;
      }
      cellArrayIt++;

      //find min and max values in scalar data
      range[0] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
      range[1] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
      cellArrayIt++;

      for (i = 1; i < numCellPts; i++)
      {
        tempScalar = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
        cellArrayIt++;
        if (tempScalar <= range[0])
        {
          range[0] = tempScalar;
        } //if tempScalar <= min range value
        if (tempScalar >= range[1])
        {
          range[1] = tempScalar;
        } //if tempScalar >= max range value
      } // for all points in this cell

      int needCell = 0;
      if (0 >= range[0] && 0 <= range[1]) 
      {
        needCell = 1;
      }

      if (needCell) 
      {
        vtkCell *cell = input->GetCell(cellId);
        cellIds = cell->GetPointIds();
        cutScalars->GetTuples(cellIds,cellScalars);
        // Loop over all contour values.
        if (dimensionality == 3 && !(++cut % progressInterval) )
        {
          vtkDebugMacro(<<"Cutting #" << cut);
          this->UpdateProgress ((double)cut/numCuts);
          abortExecute = this->GetAbortExecute();
        }

        this->ContourUnstructuredGridCell(cell, cellScalars, this->Locator, 
            newVerts, newLines, newPolys, inPD, outPD,
            inCD, cellId, outCD);
      } // if need cell
    } // for all cells
  } // for all dimensions (1,2,3).

  // Update ourselves.  Because we don't know upfront how many verts, lines,
  // polys we've created, take care to reclaim memory. 
  //
  cellScalars->Delete();
  cutScalars->Delete();

  if ( this->GenerateCutScalars )
  {
    inPD->Delete();
  }

  output->SetPoints(newPoints);
  newPoints->Delete();

  if (newVerts->GetNumberOfCells())
  {
    output->SetVerts(newVerts);
  }
  newVerts->Delete();

  if (newLines->GetNumberOfCells())
  {
    output->SetLines(newLines);
  }
  newLines->Delete();

  if (newPolys->GetNumberOfCells())
  {
    output->SetPolys(newPolys);
  }
  newPolys->Delete();

  this->Locator->Initialize();//release any extra memory
  output->Squeeze();
}

void vtkPointSetSlicer::ContourUnstructuredGridCell(vtkCell* cell,
         vtkDataArray* cellScalars, vtkPointLocator* locator,
         vtkCellArray* verts, vtkCellArray* lines,
         vtkCellArray* polys, vtkPointData* inPd,
         vtkPointData* outPd, vtkCellData* inCd,
         vtkIdType cellId, vtkCellData* outCd)
{
  if (cell->GetCellType() == VTK_HEXAHEDRON)
  {
    static int CASE_MASK[8] = {1,2,4,8,16,32,64,128};
    POLY_CASES *polyCase;
    EDGE_LIST  *edge;
    int i, j, index, *vert;
    volatile int pnum;
    int v1, v2, newCellId;
    double t, x1[3], x2[3], x[3], deltaScalar;
    vtkIdType offset = verts->GetNumberOfCells() + lines->GetNumberOfCells();

    // Build the case table
    for ( i=0, index = 0; i < 8; i++)
    {
      if (cellScalars->GetComponent(i,0) >= 0)
      {
        index |= CASE_MASK[i];
      }
    }

    polyCase = polyCases + index;
    edge = polyCase->edges;

    // get the point number of the polygon
    pnum = 0;
    for (i = 0; i < 8; i++)
      if (edge[i] > -1) pnum++;
      else break;

    vtkIdType* pts = new vtkIdType[pnum];
    for (i=0; i<pnum; i++) // insert polygon
    {
      vert = edges[edge[i]];

      // calculate a preferred interpolation direction
      deltaScalar = (cellScalars->GetComponent(vert[1],0) 
          - cellScalars->GetComponent(vert[0],0));
      if (deltaScalar > 0)
      {
        v1 = vert[0]; v2 = vert[1];
      }
      else
      {
        v1 = vert[1]; v2 = vert[0];
        deltaScalar = -deltaScalar;
      }

      // linear interpolation
      t = ( deltaScalar == 0.0 ? 0.0 : (-cellScalars->GetComponent(v1,0)) / deltaScalar );

      cell->GetPoints()->GetPoint(v1, x1);
      cell->GetPoints()->GetPoint(v2, x2);

      for (j=0; j<3; j++)
      {
        x[j] = x1[j] + t * (x2[j] - x1[j]);
      }
      if ( locator->InsertUniquePoint(x, pts[i]) )
      {
        if ( outPd ) 
        {
          vtkIdType p1 = cell->GetPointIds()->GetId(v1);
          vtkIdType p2 = cell->GetPointIds()->GetId(v2);
          outPd->InterpolateEdge(inPd,pts[i],p1,p2,t);
        }
      }
    }

    // check for degenerate polygon
    std::vector<vtkIdType> pset;
    for (i=0; i<pnum; i++)
    {
      if (std::find(pset.begin(), pset.end(), pts[i]) == pset.end())
        pset.push_back(pts[i]);
    }

    if (pset.size() > 2)
    {
      i = 0;
      for (std::vector<vtkIdType>::iterator iter = pset.begin(); iter != pset.end(); iter++)
      {
        pts[i] = *iter;
        i++;
      }
      newCellId = offset + polys->InsertNextCell(pset.size(),pts);
      outCd->CopyData(inCd,cellId,newCellId);
    }
    delete [] pts;

  }
  else
  {
    cell->Contour(0, cellScalars, locator, verts, lines, polys,
      inPd, outPd, inCd, cellId, outCd);
  }
}

// Specify a spatial locator for merging points. By default, 
// an instance of vtkMergePoints is used.
void vtkPointSetSlicer::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator == locator ) 
    {
    return;
    }
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = 0;
    }
  if ( locator )
    {
    locator->Register(this);
    }
  this->Locator = locator;
  this->Modified();
}

void vtkPointSetSlicer::CreateDefaultLocator()
{
  if ( this->Locator == 0 )
    {
    this->Locator = vtkMergePoints::New();
    this->Locator->Register(this);
    this->Locator->Delete();
    }
}


void vtkPointSetSlicer::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Slice Plane: " << this->SlicePlane << "\n";

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }

  os << indent << "Generate Cut Scalars: " 
     << (this->GenerateCutScalars ? "On\n" : "Off\n");
}

int vtkPointSetSlicer::edges[12][2] = { {0,1},{1,2},{3,2},{0,3},
      {4,5},{5,6},{7,6},{4,7},
      {0,4},{1,5},{2,6},{3,7} };

vtkPointSetSlicer::POLY_CASES
vtkPointSetSlicer::polyCases[256] = {
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{0, 3, 8, -1, -1, -1, -1, -1}},
    {{1, 0, 9, -1, -1, -1, -1, -1}},
    {{1, 3, 8, 9, -1, -1, -1, -1}},
    {{2, 1, 10, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{2, 0, 9, 10, -1, -1, -1, -1}},
    {{2, 10, 9, 8, 3, -1, -1, -1}},
    {{3, 2, 11, -1, -1, -1, -1, -1}},
    {{0, 2, 11, 8, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{1, 9, 8, 11, 2, -1, -1, -1}},
    {{3, 1, 10, 11, -1, -1, -1, -1}},
    {{0, 8, 11, 10, 1, -1, -1, -1}},
    {{3, 11, 10, 9, 0, -1, -1, -1}},
    {{8, 9, 10, 11, -1, -1, -1, -1}},
    {{4, 7, 8, -1, -1, -1, -1, -1}},
    {{3, 7, 4, 0, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{9, 1, 3, 7, 4, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{11, 2, 0, 4, 7, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{1, 2, 11, 7, 4, 9, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{4, 7, 11, 10, 9, -1, -1, -1}},
    {{5, 4, 9, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{0, 4, 5, 1, -1, -1, -1, -1}},
    {{8, 3, 1, 5, 4, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{10, 2, 0, 4, 5, -1, -1, -1}},
    {{2, 3, 8, 4, 5, 10, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{5, 4, 8, 11, 10, -1, -1, -1}},
    {{5, 7, 8, 9, -1, -1, -1, -1}},
    {{9, 5, 7, 3, 0, -1, -1, -1}},
    {{8, 7, 5, 1, 0, -1, -1, -1}},
    {{1, 3, 7, 5, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{2, 10, 5, 7, 3, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{2, 11, 7, 5, 1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{5, 7, 11, 10, -1, -1, -1, -1}},
    {{6, 5, 10, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{1, 5, 6, 2, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{9, 0, 2, 6, 5, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{11, 3, 1, 5, 6, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{3, 0, 9, 5, 6, 11, -1, -1}},
    {{6, 5, 9, 8, 11, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{6, 4, 9, 10, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{10, 6, 4, 0, 1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{9, 4, 6, 2, 1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{2, 0, 4, 6, -1, -1, -1, -1}},
    {{3, 8, 4, 6, 2, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{3, 11, 6, 4, 0, -1, -1, -1}},
    {{6, 4, 8, 11, -1, -1, -1, -1}},
    {{6, 10, 9, 8, 7, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{6, 7, 8, 0, 1, 10, -1, -1}},
    {{6, 10, 1, 3, 7, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{7, 8, 0, 2, 6, -1, -1, -1}},
    {{2, 6, 7, 3, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{6, 7, 11, -1, -1, -1, -1, -1}},
    {{7, 6, 11, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{2, 6, 7, 3, -1, -1, -1, -1}},
    {{8, 0, 2, 6, 7, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{10, 1, 3, 7, 6, -1, -1, -1}},
    {{0, 1, 10, 6, 7, 8, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{7, 6, 10, 9, 8, -1, -1, -1}},
    {{4, 6, 11, 8, -1, -1, -1, -1}},
    {{11, 6, 4, 0, 3, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{8, 4, 6, 2, 3, -1, -1, -1}},
    {{0, 2, 6, 4, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{1, 9, 4, 6, 2, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{1, 10, 6, 4, 0, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{4, 6, 10, 9, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{5, 9, 8, 11, 6, -1, -1, -1}},
    {{5, 6, 11, 3, 0, 9, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{6, 11, 3, 1, 5, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{5, 9, 0, 2, 6, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{1, 5, 6, 2, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{5, 6, 10, -1, -1, -1, -1, -1}},
    {{7, 5, 10, 11, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{11, 7, 5, 1, 2, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{10, 5, 7, 3, 2, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{3, 1, 5, 7, -1, -1, -1, -1}},
    {{0, 8, 7, 5, 1, -1, -1, -1}},
    {{0, 9, 5, 7, 3, -1, -1, -1}},
    {{7, 5, 9, 8, -1, -1, -1, -1}},
    {{4, 8, 11, 10, 5, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{4, 5, 10, 2, 3, 8, -1, -1}},
    {{5, 10, 2, 0, 4, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{4, 8, 3, 1, 5, -1, -1, -1}},
    {{0, 4, 5, 1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{4, 5, 9, -1, -1, -1, -1, -1}},
    {{7, 11, 10, 9, 4, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{7, 4, 9, 1, 2, 11, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{7, 11, 2, 0, 4, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{4, 9, 1, 3, 7, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{3, 7, 4, 0, -1, -1, -1, -1}},
    {{7, 4, 8, -1, -1, -1, -1, -1}},
    {{10, 11, 8, 9, -1, -1, -1, -1}},
    {{0, 3, 11, 10, 9, -1, -1, -1}},
    {{1, 0, 8, 11, 10, -1, -1, -1}},
    {{1, 3, 11, 10, -1, -1, -1, -1}},
    {{2, 1, 9, 8, 11, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{2, 0, 8, 11, -1, -1, -1, -1}},
    {{2, 3, 11, -1, -1, -1, -1, -1}},
    {{3, 2, 10, 9, 8, -1, -1, -1}},
    {{0, 2, 10, 9, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}},
    {{1, 2, 10, -1, -1, -1, -1, -1}},
    {{3, 1, 9, 8, -1, -1, -1, -1}},
    {{0, 1, 9, -1, -1, -1, -1, -1}},
    {{3, 0, 8, -1, -1, -1, -1, -1}},
    {{-1, -1, -1, -1, -1, -1, -1, -1}}
};
