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

#include <itkMesh.h>
#include <itkLineCell.h>
#include <itkTriangleCell.h>
#include <itkPolygonCell.h>
#include <itkQuadrilateralCell.h>
#include <itkCellInterface.h>
#include <itkDefaultDynamicMeshTraits.h> 
#include <itkSphereMeshSource.h> 
#include <itkTransformMeshFilter.h>
#include <itkTranslationTransform.h>
#include <itkMinimumMaximumImageCalculator.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkFloatArray.h>

template <typename MeshType>
class NullScalarAccessor
{
public:
  static inline float GetScalar(typename MeshType::PointDataContainer* point, typename MeshType::PointIdentifier idx)
  {
    return 0.0f;
  };
};

/*!
\brief The class provides mehtods for ITK - VTK mesh conversion
*
*  \todo document the inner class 
*  \todo maybe inner class should be moved out
*/

template <typename MeshType, class ScalarAccessor=NullScalarAccessor<MeshType> >
class MeshUtil
{

  /*!
  \brief implementation for VTK cell visitors
  \todo documentation
  */
  class VistVTKCellsClass
  {
    vtkCellArray* m_Cells;
    int* m_LastCell;
    int* m_TypeArray;
  public:
    // typedef the itk cells we are interested in
    typedef typename itk::CellInterface< typename MeshType::CellPixelType, 
                                         typename MeshType::CellTraits >  CellInterfaceType;
    typedef itk::LineCell<CellInterfaceType>      floatLineCell;
    typedef itk::PolygonCell<CellInterfaceType>      floatPolygonCell;
    typedef itk::TriangleCell<CellInterfaceType>      floatTriangleCell;
    typedef itk::QuadrilateralCell<CellInterfaceType> floatQuadrilateralCell;

    /*! Set the vtkCellArray that will be constructed
    */
    void SetCellArray(vtkCellArray* a) 
    {
      m_Cells = a;
    }

    /*! 
    Set the cell counter pointer
    */
    void SetCellCounter(int* i)
    {
      m_LastCell = i;
    }

    /*!
    Set the type array for storing the vtk cell types
    */
    void SetTypeArray(int* i)
    {
      m_TypeArray = i;
    }

    /*!
    Visit a line and create the VTK_LINE cell   
    */
    void Visit(unsigned long , floatLineCell* t)
    {
      m_Cells->InsertNextCell(2,  (vtkIdType*)t->PointIdsBegin());
      m_TypeArray[*m_LastCell] = VTK_LINE;
      (*m_LastCell)++;
    }

    /*!
    Visit a line and create the VTK_POLYGON cell   
    */
    void Visit(unsigned long , floatPolygonCell* t)
    {
 
      unsigned long num = t->GetNumberOfVertices();
      if (num > 3) {
        m_Cells->InsertNextCell(num, (vtkIdType*)t->PointIdsBegin());
        m_TypeArray[*m_LastCell] = VTK_POLYGON;
        (*m_LastCell)++;
      }
    }

    /*!
    Visit a triangle and create the VTK_TRIANGLE cell   
    */
    void Visit(unsigned long , floatTriangleCell* t)
    {
      m_Cells->InsertNextCell(3,  (vtkIdType*)t->PointIdsBegin());
      m_TypeArray[*m_LastCell] = VTK_TRIANGLE;
      (*m_LastCell)++;
    }

    /*! 
    Visit a triangle and create the VTK_QUAD cell 
    */
    void Visit(unsigned long , floatQuadrilateralCell* t)
    {
      m_Cells->InsertNextCell(4,  (vtkIdType*)t->PointIdsBegin());
      m_TypeArray[*m_LastCell] = VTK_QUAD;
      (*m_LastCell)++;
    }
  };


  typedef typename MeshType::CellType                CellType;
  typedef typename itk::LineCell< CellType >         LineType;
  typedef typename itk::PolygonCell< CellType >         PolygonType;
  typedef typename itk::TriangleCell< CellType >     TriangleType;


  /*! 
  default line cell visitior definition 
  */
  typedef typename itk::CellInterfaceVisitorImplementation<
                                      typename MeshType::CellPixelType,
                                      typename MeshType::CellTraits,
                                      LineType, 
                                      VistVTKCellsClass>			LineVisitor;

  /*! 
  default line cell visitior definition 
  */
  typedef typename itk::CellInterfaceVisitorImplementation<
                                      typename MeshType::CellPixelType,
                                      typename MeshType::CellTraits,
                                      PolygonType, 
                                      VistVTKCellsClass>			PolygonVisitor;

  /*! 
  default triangle cell visitior definition 
  */
  typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
    typename MeshType::CellTraits,
    itk::TriangleCell<itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits >
    >, VistVTKCellsClass>			TriangleVisitor;


  /*! 
  default quad cell visitior definition 
  */
  typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType, typename MeshType::CellTraits,
    itk::QuadrilateralCell< itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
    VistVTKCellsClass> QuadrilateralVisitor;


public:

  /*!
  create an itkMesh object from a vtkPolyData
  */
  static typename MeshType::Pointer meshFromPolyData(vtkPolyData* poly)
  {
    // Create a new mesh
    typename MeshType::Pointer output = MeshType::New();
    output->SetCellsAllocationMethod( MeshType::CellsAllocatedDynamicallyCellByCell );  

    typedef typename MeshType::CellDataContainer MeshCellDataContainerType;

    output->SetCellData(MeshCellDataContainerType::New());

    // Get the points from vtk
    vtkPoints* vtkpoints = poly->GetPoints();
    int numPoints = poly->GetNumberOfPoints();

    // Create a compatible point container for the mesh
    // the mesh is created with a null points container
    // MeshType::PointsContainer::Pointer points = 
    //   MeshType::PointsContainer::New();
    // // Resize the point container to be able to fit the vtk points
    // points->Reserve(numPoints);
    // // Set the point container on the mesh
    //output->SetPoints(points);
    for(int i =0; i < numPoints; i++)
    {
      float* test = vtkpoints->GetPoint(i);
      std::cout << "next point: " << test[0]<< "," << test[1] << "," << test[2] << std::endl;
      typename MeshType::PixelType* apoint = (typename MeshType::PixelType*) vtkpoints->GetPoint(i);
      typename MeshType::PointType p;
      p[0] = test[0];
      p[1] = test[1];
      p[2] = test[2];
      output->SetPoint( i, p );
    }

    vtkCellArray* vtkcells = poly->GetPolys();
//    vtkCellArray* vtkcells = poly->GetStrips();
    //MeshType::CellsContainerPointer cells = MeshType::CellsContainer::New();
    //output->SetCells(cells);
    // extract the cell id's from the vtkUnstructuredGrid
    int numcells = vtkcells->GetNumberOfCells();
    int* vtkCellTypes = new int[numcells];
    int cellId =0;
    for(; cellId < numcells; cellId++)
    {
      vtkCellTypes[cellId] = poly->GetCellType(cellId);
    }

    // cells->Reserve(numcells);
    vtkIdType npts;
    vtkIdType* pts;
    cellId = 0;

    typedef typename MeshType::MeshTraits   OMeshTraits;
    typedef typename OMeshTraits::PixelType       OPixelType; 
    typedef typename MeshType::CellTraits					CellTraits;
    typedef typename itk::CellInterface<OPixelType, CellTraits>   CellInterfaceType;
    typedef typename itk::TriangleCell<CellInterfaceType>         TriCellType;
    typedef typename TriCellType::CellAutoPointer	           TriCellPointer;

    TriCellPointer newCell;

    for(vtkcells->InitTraversal(); vtkcells->GetNextCell(npts, pts); cellId++)
    {
      //    MeshType::CellAutoPointer c;
      switch(vtkCellTypes[cellId])
      {
      case VTK_TRIANGLE:
        {
          newCell.TakeOwnership( new TriCellType );
          newCell->SetPointIds((unsigned long*)pts);
          output->SetCell(cellId, newCell );
          output->SetCellData(cellId, (typename MeshType::PixelType)3);
          break;    
          //unsigned long pointIds[3] ={tp[0],tp[1],tp[2]};
          //newCell.TakeOwnership(  new TriCellType ); 
          //newCell->SetPointIds(pointIds);
          //output->SetCell(cellId, newCell );
          //output->SetCellData(cellId, (OPixelType)3);
          //cellId++;

          // break;
        }  
      case VTK_QUAD:
      case VTK_EMPTY_CELL:
      case VTK_VERTEX:
      case VTK_POLY_VERTEX:
      case VTK_LINE:
      case VTK_POLY_LINE:
      case VTK_TRIANGLE_STRIP:
      case VTK_POLYGON:
      case VTK_PIXEL:
      case VTK_TETRA:
      case VTK_VOXEL:
      case VTK_HEXAHEDRON:
      case VTK_WEDGE:
      case VTK_PYRAMID:
      case VTK_PARAMETRIC_CURVE:
      case VTK_PARAMETRIC_SURFACE:
      default:
        std::cerr << "Warning unhandled cell type " 
          << vtkCellTypes[cellId] << std::endl;
      }
    }
    //output->Print(std::cout);
    output->BuildCellLinks();
    return output;
  }


  /*!
    create an vtkUnstructuredGrid object from an itkMesh
  */
  static vtkUnstructuredGrid* meshToUnstructuredGrid(typename MeshType::Pointer mesh)
  {


    // Get the number of points in the mesh
    int numPoints = mesh->GetNumberOfPoints();
    if(numPoints == 0)
    {
      mesh->Print(std::cerr);
      std::cerr << "no points in Grid " << std::endl;
      exit(-1);
    }
    // Create a vtkUnstructuredGrid
    vtkUnstructuredGrid* vgrid = vtkUnstructuredGrid::New();
    // Create the vtkPoints object and set the number of points
    vtkPoints* vpoints = vtkPoints::New();
    vpoints->SetNumberOfPoints(numPoints);
    // iterate over all the points in the itk mesh filling in
    // the vtkPoints object as we go
    typename MeshType::PointsContainer::Pointer points = mesh->GetPoints();

    for(typename MeshType::PointsContainer::Iterator i = points->Begin();
      i != points->End(); ++i)
    {
      // Get the point index from the point container iterator
      int idx = i->Index();


      // Set the vtk point at the index with the the coord array from itk
      // itk returns a const pointer, but vtk is not const correct, so
      // we have to use a const cast to get rid of the const
      //      vpoints->SetPoint(idx, const_cast<DATATYPE*>(i->Value().GetDataPointer()));
      vpoints->SetPoint(idx, (typename MeshType::PixelType*)(i->Value().GetDataPointer()));
    }
    // Set the points on the vtk grid
    vgrid->SetPoints(vpoints);
    // Now create the cells using the MulitVisitor
    // 1. Create a MultiVisitor
    typename MeshType::CellType::MultiVisitor::Pointer mv =
      typename MeshType::CellType::MultiVisitor::New();
    // 2. Create a triangle and quadrilateral visitor
    typename TriangleVisitor::Pointer tv = TriangleVisitor::New();
    typename QuadrilateralVisitor::Pointer qv =  QuadrilateralVisitor::New();
    // 3. Set up the visitors
    int vtkCellCount = 0; // running counter for current cell being inserted into vtk
    int numCells = mesh->GetNumberOfCells();
    int *types = new int[numCells]; // type array for vtk 
    // create vtk cells and estimate the size
    vtkCellArray* cells = vtkCellArray::New();
    cells->EstimateSize(numCells, 4);
    // Set the TypeArray CellCount and CellArray for both visitors
    tv->SetTypeArray(types);
    tv->SetCellCounter(&vtkCellCount);
    tv->SetCellArray(cells);
    qv->SetTypeArray(types);
    qv->SetCellCounter(&vtkCellCount);
    qv->SetCellArray(cells);
    // add the visitors to the multivisitor
    mv->AddVisitor(tv);
    mv->AddVisitor(qv);
    // Now ask the mesh to accept the multivisitor which
    // will Call Visit for each cell in the mesh that matches the
    // cell types of the visitors added to the MultiVisitor
    mesh->Accept(mv);
    // Now set the cells on the vtk grid with the type array and cell array
    vgrid->SetCells(types, cells);  
    // Clean up vtk objects (no vtkSmartPointer ... )
    cells->Delete();
    vpoints->Delete();

    //std::cout << "meshToUnstructuredGrid end" << std::endl;
    return vgrid;
  }


  /*!
    create an vtkUnstructuredGrid object from an itkMesh
  */
  static vtkPolyData* meshToPolyData(typename MeshType::Pointer mesh, bool onlyTriangles = false, bool useScalarAccessor = false)
  {


    // Get the number of points in the mesh
    int numPoints = mesh->GetNumberOfPoints();
    if(numPoints == 0)
    {
      mesh->Print(std::cerr);
      std::cerr << "no points in Grid " << std::endl;
    }
    // Create a vtkUnstructuredGrid
    vtkPolyData* vgrid = vtkPolyData::New();
    // Create the vtkPoints object and set the number of points
    vtkPoints* vpoints = vtkPoints::New();
    vpoints->SetNumberOfPoints(numPoints);


    vtkFloatArray * scalars = vtkFloatArray::New();
    scalars->SetNumberOfComponents(1);
    scalars->SetNumberOfTuples(numPoints);

    // iterate over all the points in the itk mesh filling in
    // the vtkPoints object as we go
    typename MeshType::PointsContainer::Pointer points = mesh->GetPoints();

    for(typename MeshType::PointsContainer::Iterator i = points->Begin();
      i != points->End(); ++i)
    {
      // Get the point index from the point container iterator
      int idx = i->Index();
      
      typename MeshType::PointType p;
      p = i->Value();
      double  pd[3];
      pd[0]=p[0];
      pd[1]=p[1];
      pd[2]=p[2];
      // Set the vtk point at the index with the the coord array from itk
      // itk returns a const pointer, but vtk is not const correct, so
      // we have to use a const cast to get rid of the const
      //      vpoints->SetPoint(idx, const_cast<DATATYPE*>(i->Value().GetDataPointer()));
      vpoints->SetPoint(idx, pd);

      if(useScalarAccessor)
      {   
        scalars->InsertTuple1( idx, ScalarAccessor::GetScalar( mesh->GetPointData(), i->Index() ) );
      }
    }



    // Set the points on the vtk grid
    vgrid->SetPoints(vpoints);
    vgrid->GetPointData()->SetScalars(scalars);
    vgrid->GetPointData()->CopyAllOn();


    // Now create the cells using the MulitVisitor
    // 1. Create a MultiVisitor
    typedef typename MeshType::CellType::MultiVisitor MeshMV;
    typename MeshMV::Pointer mv = MeshMV::New();
    // 2. Create a triangle and quadrilateral visitor
    typename LineVisitor::Pointer lv = LineVisitor::New();
    typename PolygonVisitor::Pointer pv = PolygonVisitor::New();
    typename TriangleVisitor::Pointer tv = TriangleVisitor::New();
    typename QuadrilateralVisitor::Pointer qv =  QuadrilateralVisitor::New();
    // 3. Set up the visitors
    int vtkCellCount = 0; // running counter for current cell being inserted into vtk
    int numCells = mesh->GetNumberOfCells();
    int *types = new int[numCells]; // type array for vtk 
    // create vtk cells and estimate the size
    vtkCellArray* cells = vtkCellArray::New();
    cells->EstimateSize(numCells, 4);
    // Set the TypeArray CellCount and CellArray for both visitors

    lv->SetTypeArray(types);
    lv->SetCellCounter(&vtkCellCount);
    lv->SetCellArray(cells);

    pv->SetTypeArray(types);
    pv->SetCellCounter(&vtkCellCount);
    pv->SetCellArray(cells);

    tv->SetTypeArray(types);
    tv->SetCellCounter(&vtkCellCount);
    tv->SetCellArray(cells);
    qv->SetTypeArray(types);
    qv->SetCellCounter(&vtkCellCount);
    qv->SetCellArray(cells);
    // add the visitors to the multivisitor

    if (onlyTriangles) {
      mv->AddVisitor(tv);
      mesh->Accept(mv);
      vgrid->SetStrips(cells);  
    }
    else 
    {
      mv->AddVisitor(tv);
      mv->AddVisitor(lv);
      mv->AddVisitor(pv);
  //    mv->AddVisitor(qv);
      // Now ask the mesh to accept the multivisitor which
      // will Call Visit for each cell in the mesh that matches the
      // cell types of the visitors added to the MultiVisitor
      mesh->Accept(mv);
      // Now set the cells on the vtk grid with the type array and cell array
      vgrid->SetPolys(cells);  
  //    vgrid->SetTriangles(cells);  
      vgrid->SetLines(cells);
    // Clean up vtk objects (no vtkSmartPointer ... )
    }
    cells->Delete();
    vpoints->Delete();

    //std::cout << "meshToUnstructuredGrid end" << std::endl;
    return vgrid;
  }

  static typename MeshType::Pointer translateMesh(typename MeshType::PointType vec, typename MeshType::Pointer inputMesh)
  {
    typedef typename itk::TranslationTransform< typename MeshType::PixelType, 3>  TransformType;
    typedef typename itk::TransformMeshFilter<MeshType, MeshType, TransformType > MeshFilterType;

    typename TransformType::Pointer translation = TransformType::New();
    translation->SetIdentity();
    
    typename TransformType::ParametersType params = translation->GetParameters();
    unsigned int i;

    for (i=0; i < 3; i++) {
      params[i]= vec[i];
    }
    translation->SetParameters(params);

    typename MeshFilterType::Pointer filter = MeshFilterType::New();
    filter->SetTransform( translation );
    filter->SetInput( inputMesh );
    filter->Update();

    typename MeshType::Pointer resultMesh = filter->GetOutput();
    resultMesh->Register();
    
    return resultMesh;
  }

  template <typename TImageType>
  static std::pair<float,float> minmax(typename TImageType::Pointer image)
  {
    typedef typename itk::MinimumMaximumImageCalculator<TImageType> MinMaxCalcType;
    typename MinMaxCalcType::Pointer minmax = MinMaxCalcType::New();
    minmax->SetImage( image );
    minmax->ComputeMinimum();
    minmax->ComputeMaximum();
    return std::make_pair(minmax->GetMinimum(),minmax->GetMaximum());
  }

};
