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

#include "mitkSurfaceVtkMapper2D.h"

//mitk includes
#include <mitkDataNode.h>
#include <mitkProperties.h>
#include "mitkVtkPropRenderer.h"
#include <mitkSurface.h>
#include <mitkLookupTableProperty.h>
#include <mitkVtkScalarModeProperty.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>

//vtk includes
#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkLookupTable.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkAssembly.h>
#include <vtkPointData.h>
#include <vtkGlyph3D.h>
#include <vtkReverseSense.h>
#include <vtkArrowSource.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkSphereTree.h>
#include <vtkSMPThreadLocal.h>
#include <vtkSMPThreadLocalObject.h>
#include <vtkNonMergingPointLocator.h>
#include <vtkGenericCell.h>
#include <vtkDoubleArray.h>
#include <vtkSMPTools.h>
#include <vtkMultiPieceDataSet.h>
#include <vtkCellData.h>
#include <vtkAppendPolyData.h>

struct vtkLocalDataType
{
  vtkPolyData* Output;
  vtkNonMergingPointLocator* Locator;
  vtkCellData* NewVertsData;
  vtkCellData* NewLinesData;
  vtkCellData* NewPolysData;

  vtkLocalDataType()
    : Output(nullptr)
    , Locator(nullptr)
  {
  }
};

struct PlaneCutFunctor {
  vtkPolyData* polyData;

  vtkDataSet* Input;
  vtkPoints* InPoints;
  int PointsType;
  vtkDataObject* Output;
  vtkPlane* Plane;
  vtkSphereTree* SphereTree;
  const unsigned char* Selected;
  unsigned char* InOutArray;

  vtkSMPThreadLocal<vtkDoubleArray*> CellScalars;
  vtkSMPThreadLocalObject<vtkGenericCell> Cell;
  vtkSMPThreadLocalObject<vtkPoints> NewPts;
  vtkSMPThreadLocalObject<vtkCellArray> NewVerts;
  vtkSMPThreadLocalObject<vtkCellArray> NewLines;
  vtkSMPThreadLocalObject<vtkCellArray> NewPolys;

  vtkSMPThreadLocal<vtkLocalDataType> LocalData;

  double* Origin;
  double* Normal;
  vtkIdType NumSelected;
  bool Interpolate;
  bool GeneratePolygons;

  void SetInPoints(vtkPoints* inPts)
  {
    this->InPoints = inPts;
    inPts->Register(nullptr);
    this->PointsType = this->InPoints->GetDataType();
  }

  void operator() (vtkIdType cellId, vtkIdType endCellId)
  {
    vtkLocalDataType& localData = this->LocalData.Local();
    vtkPointLocator* loc = localData.Locator;

    vtkGenericCell* cell = this->Cell.Local();
    vtkDoubleArray* cellScalars = this->CellScalars.Local();
    vtkPointData* inPD = this->Input->GetPointData();
    vtkCellData* inCD = this->Input->GetCellData();

    vtkPolyData* output = localData.Output;
    vtkPointData* outPD = nullptr;

    vtkCellArray* newVerts = this->NewVerts.Local();
    vtkCellArray* newLines = this->NewLines.Local();
    vtkCellArray* newPolys = this->NewPolys.Local();

    vtkCellData* newVertsData = nullptr;
    vtkCellData* newLinesData = nullptr;
    vtkCellData* newPolysData = localData.NewPolysData;

    vtkCellData* tmpOutCD = nullptr;
    outPD = output->GetPointData();

    double* s;
    int i, numPts;
    vtkPoints* cellPoints;
    const unsigned char* selected = this->Selected + cellId;

    for (; cellId < endCellId; ++cellId) {
      if ((*selected++) != 0) {

        this->Input->GetCell(cellId, cell);
        numPts = cell->GetNumberOfPoints();
        cellScalars->SetNumberOfTuples(numPts);
        s = cellScalars->GetPointer(0);
        cellPoints = cell->GetPoints();

        for (i = 0; i < numPts; i++) {
          *s++ = this->Plane->FunctionValue(cellPoints->GetPoint(i));
        }

        tmpOutCD = newPolysData;
        cell->Contour(0., cellScalars, loc, newVerts, newLines, newPolys, inPD, outPD, inCD, cellId, tmpOutCD);
      }
    }
  }

  PlaneCutFunctor(vtkDataSet* input, vtkDataObject* output,
      vtkPlane* plane, vtkSphereTree* tree,
      double* origin, double* normal)
  {
    this->Input = input;
    this->InPoints = nullptr;
    this->Output = output;
    this->Plane = plane;
    this->SphereTree = tree;
    this->InOutArray = nullptr;
    this->Origin = origin;
    this->Normal = normal;

    this->polyData = vtkPolyData::SafeDownCast(input);
    if (this->polyData->NeedToBuildCells()) {
      this->polyData->BuildCells();
    }
    this->SetInPoints(this->polyData->GetPoints());
  }

  virtual ~PlaneCutFunctor()
  {
    vtkSMPThreadLocal<vtkLocalDataType>::iterator dataIter = this->LocalData.begin();
    while (dataIter != this->LocalData.end()) {
      (*dataIter).NewVertsData->Delete();
      (*dataIter).NewLinesData->Delete();
      (*dataIter).NewPolysData->Delete();
      ++dataIter;
    }

    vtkSMPThreadLocal<vtkDoubleArray*>::iterator cellScalarsIter = this->CellScalars.begin();
    while (cellScalarsIter != this->CellScalars.end()) {
      (*cellScalarsIter)->Delete();
      ++cellScalarsIter;
    }

    dataIter = this->LocalData.begin();
    while (dataIter != this->LocalData.end()) {
      (*dataIter).Output->Delete();
      (*dataIter).Locator->Delete();
      ++dataIter;
    }

    if (this->InPoints) {
      this->InPoints->Delete();
    }

    delete[] this->InOutArray;
  }

  void BuildAccelerationStructure()
  {
    this->Selected = this->SphereTree->SelectPlane(this->Origin, this->Normal, this->NumSelected);
  }

  static void Execute(vtkDataSet* input, vtkDataObject* output,
      vtkPlane* plane, vtkSphereTree* tree,
      double* origin, double* normal)
  {
    vtkIdType numCells = input->GetNumberOfCells();
    PlaneCutFunctor functor(input, output, plane, tree, origin, normal);
    functor.BuildAccelerationStructure();
    vtkSMPTools::For(0, numCells, functor);
  }
  
  // Called at the start of each thread
  void Initialize()
  {
    vtkLocalDataType& localData = this->LocalData.Local();

    localData.Output = vtkPolyData::New();
    vtkPolyData* output = localData.Output;

    localData.Locator = vtkNonMergingPointLocator::New();
    vtkPointLocator* locator = localData.Locator;

    vtkIdType numCells = this->Input->GetNumberOfCells();

    vtkPoints*& newPts = this->NewPts.Local();
    newPts->SetDataType(VTK_FLOAT);
    output->SetPoints(newPts);

    vtkIdType estimatedSize = static_cast<vtkIdType>(sqrt(static_cast<double>(numCells)));
    estimatedSize = estimatedSize / 1024 * 1024; // Multiple of 1024
    estimatedSize = (estimatedSize < 1024 ? 1024 : estimatedSize);

    newPts->Allocate(estimatedSize, estimatedSize);

    // Bounds are not important for non-merging locator
    double bounds[6];
    bounds[0] = bounds[2] = bounds[4] = (VTK_FLOAT_MIN);
    bounds[1] = bounds[3] = bounds[5] = (VTK_FLOAT_MAX);
    locator->InitPointInsertion(newPts, bounds, this->Input->GetNumberOfPoints());

    vtkCellArray*& newVerts = this->NewVerts.Local();
    newVerts->Allocate(estimatedSize, estimatedSize);
    output->SetVerts(newVerts);

    vtkCellArray*& newLines = this->NewLines.Local();
    newLines->Allocate(estimatedSize, estimatedSize);
    output->SetLines(newLines);

    vtkCellArray*& newPolys = this->NewPolys.Local();
    newPolys->Allocate(estimatedSize, estimatedSize);
    output->SetPolys(newPolys);

    vtkDoubleArray*& cellScalars = this->CellScalars.Local();
    cellScalars = vtkDoubleArray::New();
    cellScalars->SetNumberOfComponents(1);
    cellScalars->Allocate(VTK_CELL_SIZE);

    vtkPointData* outPd = output->GetPointData();
    vtkCellData* outCd = output->GetCellData();
    vtkPointData* inPd = this->Input->GetPointData();
    vtkCellData* inCd = this->Input->GetCellData();

    outPd->InterpolateAllocate(inPd, estimatedSize, estimatedSize);
    outCd->CopyAllocate(inCd, estimatedSize, estimatedSize);

    localData.NewVertsData = vtkCellData::New();
    localData.NewLinesData = vtkCellData::New();
    localData.NewPolysData = vtkCellData::New();
    localData.NewVertsData->CopyAllocate(outCd);
    localData.NewLinesData->CopyAllocate(outCd);
    localData.NewPolysData->CopyAllocate(outCd);
  }

  // Called after all threads are finished
  void Reduce()
  {
    // Recover multipiece output
    vtkMultiPieceDataSet* mp = vtkMultiPieceDataSet::SafeDownCast(this->Output);

    // Remove useless FieldData Array from multipiece
    // Created by automatic pass data in pipeline
    vtkFieldData* mpFieldData = mp->GetFieldData();
    for (int i = mpFieldData->GetNumberOfArrays() - 1; i >= 0; i--) {
      mpFieldData->RemoveArray(i);
    }

    // Create the final multi-piece
    int count = 0;
    vtkSMPThreadLocal<vtkLocalDataType>::iterator outIter = this->LocalData.begin();
    while (outIter != this->LocalData.end()) {
      vtkPolyData* output = (*outIter).Output;
      mp->SetPiece(count++, output);
      output->GetFieldData()->PassData(this->Input->GetFieldData());
      ++outIter;
    }

    outIter = this->LocalData.begin();
    while (outIter != this->LocalData.end()) {
      vtkPolyData* output = (*outIter).Output;
      vtkCellArray* newVerts = output->GetVerts();
      vtkCellArray* newLines = output->GetLines();
      vtkCellArray* newPolys = output->GetPolys();
      vtkCellData* outCD = output->GetCellData();
      vtkCellData* newVertsData = (*outIter).NewVertsData;
      vtkCellData* newLinesData = (*outIter).NewLinesData;
      vtkCellData* newPolysData = (*outIter).NewPolysData;

      // Reconstruct cell data
      outCD->CopyData(newVertsData, 0, newVerts->GetNumberOfCells(), 0);
      vtkIdType offset = newVerts->GetNumberOfCells();
      outCD->CopyData(newLinesData, offset, newLines->GetNumberOfCells(), 0);
      offset += newLines->GetNumberOfCells();
      outCD->CopyData(newPolysData, offset, newPolys->GetNumberOfCells(), 0);
      ++outIter;
    }
  }
};

// constructor LocalStorage
mitk::SurfaceVtkMapper2D::LocalStorage::LocalStorage()
{
  m_Mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
  m_Mapper->ScalarVisibilityOff();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_PropAssembly = vtkSmartPointer <vtkAssembly>::New();
  m_PropAssembly->AddPart( m_Actor );
  m_CuttingPlane = vtkSmartPointer<vtkPlane>::New();
  m_SphereTree = nullptr;
  m_EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();
}

// destructor LocalStorage
mitk::SurfaceVtkMapper2D::LocalStorage::~LocalStorage()
{
}

const mitk::Surface* mitk::SurfaceVtkMapper2D::GetInput() const
{
  return static_cast<const mitk::Surface * > ( GetDataNode()->GetData() );
}

// constructor PointSetVtkMapper2D
mitk::SurfaceVtkMapper2D::SurfaceVtkMapper2D()
{
}

mitk::SurfaceVtkMapper2D::~SurfaceVtkMapper2D()
{
}

// reset mapper so that nothing is displayed e.g. toggle visiblity of the propassembly
void mitk::SurfaceVtkMapper2D::ResetMapper( BaseRenderer* renderer )
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_PropAssembly->VisibilityOff();
}

vtkProp* mitk::SurfaceVtkMapper2D::GetVtkProp(mitk::BaseRenderer * renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_PropAssembly;
}

void mitk::SurfaceVtkMapper2D::Update(mitk::BaseRenderer* renderer)
{
  const mitk::DataNode* node = GetDataNode();
  if( node == NULL )
    return;
  bool visible = true;
  node->GetVisibility(visible, renderer, "visible");
  if ( !visible )
    return;

  mitk::Surface* surface  = static_cast<mitk::Surface *>( node->GetData() );
  if ( surface == NULL )
    return;

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  // Check if time step is valid
  const mitk::TimeGeometry *dataTimeGeometry = surface->GetTimeGeometry();
  if ( ( dataTimeGeometry == NULL )
       || ( dataTimeGeometry->CountTimeSteps() == 0 )
       || ( !dataTimeGeometry->IsValidTimeStep( this->GetTimestep() ) ) )
  {
    return;
  }

  surface->UpdateOutputInformation();
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);

  //check if something important has changed and we need to rerender
  if ( (localStorage->m_LastUpdateTime < node->GetMTime()) //was the node modified?
       || (localStorage->m_LastUpdateTime < surface->GetPipelineMTime()) //Was the data modified?
       || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) //was the geometry modified?
       || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldPlaneGeometry()->GetMTime())
       || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
       || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
  {
    this->GenerateDataForRenderer( renderer );
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::SurfaceVtkMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);

  if (renderer->is3dDrawingLocked()) {
    localStorage->m_Mapper->SetInputData(localStorage->m_EmptyPolyData);
    return;
  }

  const DataNode* node = GetDataNode();
  Surface* surface  = static_cast<Surface *>( node->GetData() );
  const TimeGeometry *dataTimeGeometry = surface->GetTimeGeometry();

  ScalarType time =renderer->GetTime();
  int timestep=0;

  if( time > itk::NumericTraits<ScalarType>::NonpositiveMin() )
    timestep = dataTimeGeometry->TimePointToTimeStep( time );

  vtkSmartPointer<vtkPolyData> inputPolyData = surface->GetVtkPolyData( timestep );
  if ((inputPolyData == NULL) || (inputPolyData->GetNumberOfPoints() < 1))
    return;

  //apply color and opacity read from the PropertyList
  this->ApplyAllProperties(renderer);

  const PlaneGeometry* planeGeometry = renderer->GetCurrentWorldPlaneGeometry();
  if( ( planeGeometry == NULL ) || ( !planeGeometry->IsValid() ) || ( !planeGeometry->HasReferenceGeometry() ))
  {
    return;
  }

  if (localStorage->m_Actor->GetMapper() == NULL)
    localStorage->m_Actor->SetMapper(localStorage->m_Mapper);

  double origin[3];
  origin[0] = planeGeometry->GetOrigin()[0];
  origin[1] = planeGeometry->GetOrigin()[1];
  origin[2] = planeGeometry->GetOrigin()[2];

  double normal[3];
  normal[0] = planeGeometry->GetNormal()[0];
  normal[1] = planeGeometry->GetNormal()[1];
  normal[2] = planeGeometry->GetNormal()[2];

  localStorage->m_CuttingPlane->SetOrigin(origin);
  localStorage->m_CuttingPlane->SetNormal(normal);
  //Transform the data according to its geometry.
  //See UpdateVtkTransform documentation for details.
  vtkSmartPointer<vtkLinearTransform> vtktransform = GetDataNode()->GetVtkTransform(this->GetTimestep());
  vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  filter->SetTransform(vtktransform);
  filter->SetInputData(inputPolyData);

  // Build Sphere Tree
  if (localStorage->m_SphereTree == nullptr) {
    localStorage->m_SphereTree = vtkSmartPointer<vtkSphereTree>::New();
    if (inputPolyData->NeedToBuildCells()) {
      inputPolyData->BuildCells();
    }
    // Should be OK to init with polydata only once. Mitk logic recreates mappers when data changes.
    localStorage->m_SphereTree->SetDataSet(inputPolyData);
  }
  localStorage->m_SphereTree->Build();

  // Iterate over cells and map them onto polyData
  vtkSmartPointer<vtkMultiPieceDataSet> outputPolyData = vtkSmartPointer<vtkMultiPieceDataSet>::New();
  PlaneCutFunctor::Execute(inputPolyData, outputPolyData, localStorage->m_CuttingPlane, localStorage->m_SphereTree, origin, normal);

  vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
  for (unsigned int i = 0; i < outputPolyData->GetNumberOfPieces(); i++) {
    appendFilter->AddInputDataObject(outputPolyData->GetPieceAsDataObject(i));
  }

  appendFilter->Update();

  localStorage->m_Mapper->SetInputData(appendFilter->GetOutput());
}

void mitk::SurfaceVtkMapper2D::ApplyAllProperties(mitk::BaseRenderer* renderer)
{
  const DataNode * node = GetDataNode();

  if(node == NULL)
  {
    return;
  }

  float lineWidth = 1.0f;
  node->GetFloatProperty("line width", lineWidth, renderer);

  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);

  // check for color and opacity properties, use it for rendering if they exists
  float color[3]= { 1.0f, 1.0f, 1.0f };
  node->GetColor(color, renderer, "color");
  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer, "opacity");

  //Pass properties to VTK
  localStorage->m_Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  localStorage->m_Actor->GetProperty()->SetLineWidth(lineWidth);
  //By default, the cutter will also copy/compute normals of the cut
  //to the output polydata. The normals will influence the
  //vtkPolyDataMapper lightning. To view a clean cut the lighting has
  //to be disabled.
  localStorage->m_Actor->GetProperty()->SetLighting(0);

  // same block for scalar data rendering as in 3D mapper
  mitk::TransferFunctionProperty::Pointer transferFuncProp;
  this->GetDataNode()->GetProperty(transferFuncProp, "Surface.TransferFunction", renderer);
  if (transferFuncProp.IsNotNull() )
  {
    localStorage->m_Mapper->SetLookupTable(transferFuncProp->GetValue()->GetColorTransferFunction());
  }

  mitk::LookupTableProperty::Pointer lookupTableProp;
  this->GetDataNode()->GetProperty(lookupTableProp, "LookupTable", renderer);
  if (lookupTableProp.IsNotNull() )
  {
    localStorage->m_Mapper->SetLookupTable(lookupTableProp->GetLookupTable()->GetVtkLookupTable());
  }

  mitk::LevelWindow levelWindow;
  if(this->GetDataNode()->GetLevelWindow(levelWindow, renderer, "levelWindow"))
  {
    localStorage->m_Mapper->SetScalarRange(levelWindow.GetLowerWindowBound(),levelWindow.GetUpperWindowBound());
  }
  else if(this->GetDataNode()->GetLevelWindow(levelWindow, renderer))
  {
    localStorage->m_Mapper->SetScalarRange(levelWindow.GetLowerWindowBound(),levelWindow.GetUpperWindowBound());
  }

  bool scalarVisibility = false;
  this->GetDataNode()->GetBoolProperty("scalar visibility", scalarVisibility);
  localStorage->m_Mapper->SetScalarVisibility( (scalarVisibility ? 1 : 0) );

  if(scalarVisibility)
  {
    mitk::VtkScalarModeProperty* scalarMode;
    if(this->GetDataNode()->GetProperty(scalarMode, "scalar mode", renderer))
      localStorage->m_Mapper->SetScalarMode(scalarMode->GetVtkScalarMode());
    else
      localStorage->m_Mapper->SetScalarModeToDefault();

    bool colorMode = false;
    this->GetDataNode()->GetBoolProperty("color mode", colorMode);
    localStorage->m_Mapper->SetColorMode( (colorMode ? 1 : 0) );

    double scalarsMin = 0;
    this->GetDataNode()->GetDoubleProperty("ScalarsRangeMinimum", scalarsMin, renderer);

    double scalarsMax = 1.0;
    this->GetDataNode()->GetDoubleProperty("ScalarsRangeMaximum", scalarsMax, renderer);

    localStorage->m_Mapper->SetScalarRange(scalarsMin,scalarsMax);
  }
}

void mitk::SurfaceVtkMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  mitk::IPropertyAliases* aliases = mitk::CoreServices::GetPropertyAliases();
  node->AddProperty( "line width", FloatProperty::New(2.0f), renderer, overwrite );
  aliases->AddAlias( "line width", "Surface.2D.Line Width", "Surface");
  node->AddProperty( "scalar mode", VtkScalarModeProperty::New(), renderer, overwrite );
  node->AddProperty( "layer", IntProperty::New(100), renderer, overwrite);
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
