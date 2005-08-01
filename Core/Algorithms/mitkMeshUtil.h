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

#if(_MSC_VER==1200)
#error MeshUtils currently not supported for MS Visual C++ 6.0. Sorry.
#endif

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
  \brief A visitor to create VTK cells by means of a class
  defining the InsertImplementation interface
  
  The InsertImplementation interface defines the methods
  \code
    void InsertLine(vtkIdType *pts);
    void InsertTriangle(vtkIdType *pts);
    void InsertPolygon(vtkIdType npts, vtkIdType *pts);
    void InsertQuad(vtkIdType *pts);
  \endcode

  This class calls the appropriate insert-method of the 
  InsertImplementation according to the cell type of 
  the visited cell \em and its actual contents: e.g.,
  for a polygon cell with just two points, a line will
  be created by calling InsertLine.
  \sa ExactSwitchByCellType
  \sa SingleCellArrayInsertImplementation
  \sa DistributeInsertImplementation
  */
  template <class InsertImplementation>
  class SwitchByCellType : public InsertImplementation
  {
    // typedef the itk cells we are interested in
    typedef typename itk::CellInterface< typename MeshType::CellPixelType, 
      typename MeshType::CellTraits >  CellInterfaceType;
    typedef itk::LineCell<CellInterfaceType>          floatLineCell;
    typedef itk::TriangleCell<CellInterfaceType>      floatTriangleCell;
    typedef itk::PolygonCell<CellInterfaceType>       floatPolygonCell;
    typedef itk::QuadrilateralCell<CellInterfaceType> floatQuadrilateralCell;

  public:
    /*!
    Visit a line and create the VTK_LINE cell   
    */
    void Visit(unsigned long , floatLineCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num==2) // useless because itk::LineCell always returns 2
        this->InsertLine(pts);
    }

    /*!
    Visit a line and create the VTK_POLYGON cell   
    */
    void Visit(unsigned long , floatPolygonCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num > 3) 
        this->InsertPolygon(num, pts);
      else
      if (num == 3) 
        this->InsertTriangle(pts);
      else
      if (num==2)
        this->InsertLine(pts);
    }

    /*!
    Visit a triangle and create the VTK_TRIANGLE cell   
    */
    void Visit(unsigned long , floatTriangleCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num == 3) 
        this->InsertTriangle(pts);
      else
      if (num==2)
        this->InsertLine(pts);
    }

    /*! 
    Visit a triangle and create the VTK_QUAD cell 
    */
    void Visit(unsigned long , floatQuadrilateralCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num == 4) 
        this->InsertQuad(pts);
      else
      if (num == 3) 
        this->InsertTriangle(pts);
      else
      if (num==2)
        this->InsertLine(pts);
    }
  };

  /*!
  \brief A visitor similar to SwitchByCellType, but with
  exact matching of cell types
  
  Works as described in SwitchByCellType, but does exact
  matching of cell types, e.g., for a polygon cell with just 
  two points, \em no insert-method will be called, because
  a polygon must have at least three points.
  \sa SwitchByCellType
  \sa SingleCellArrayInsertImplementation
  \sa DistributeInsertImplementation
  */
  template <class InsertImplementation>
  class ExactSwitchByCellType : public InsertImplementation
  {
    // typedef the itk cells we are interested in
    typedef typename itk::CellInterface< typename MeshType::CellPixelType, 
      typename MeshType::CellTraits >  CellInterfaceType;
    typedef itk::LineCell<CellInterfaceType>          floatLineCell;
    typedef itk::TriangleCell<CellInterfaceType>      floatTriangleCell;
    typedef itk::PolygonCell<CellInterfaceType>       floatPolygonCell;
    typedef itk::QuadrilateralCell<CellInterfaceType> floatQuadrilateralCell;

  public:
    /*!
    Visit a line and create the VTK_LINE cell   
    */
    void Visit(unsigned long , floatLineCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num==2)
        this->InsertLine(pts);
    }

    /*!
    Visit a line and create the VTK_POLYGON cell   
    */
    void Visit(unsigned long , floatPolygonCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num > 3) 
        this->InsertPolygon(num, pts);
    }

    /*!
    Visit a triangle and create the VTK_TRIANGLE cell   
    */
    void Visit(unsigned long , floatTriangleCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num == 3) 
        this->InsertTriangle(pts);
    }

    /*! 
    Visit a triangle and create the VTK_QUAD cell 
    */
    void Visit(unsigned long , floatQuadrilateralCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num == 4) 
        this->InsertQuad(pts);
    }
  };

  /*!
  \brief Implementation of the InsertImplementation interface of
  SwitchByCellType to define a visitor that create cells 
  according to their types and put them in a single
  vtkCellArray (for vtkUnstructuredGrid construction)
  */
  class SingleCellArrayInsertImplementation
  {
    vtkCellArray* m_Cells;
    int* m_TypeArray;
  public:
    /*! Set the vtkCellArray that will be constructed
    */
    void SetCellArray(vtkCellArray* cells)
    {
      m_Cells = cells;
    }

    /*!
    Set the type array for storing the vtk cell types
    */
    void SetTypeArray(int* i)
    {
      m_TypeArray = i;
    }

    void InsertLine(vtkIdType *pts)
    {
      this->cellId = m_Cells->InsertNextCell(2, pts);
      m_TypeArray[this->cellId] = VTK_LINE;
    }

    void InsertTriangle(vtkIdType *pts)
    {
      this->cellId = m_Cells->InsertNextCell(3, pts);
      m_TypeArray[this->cellId] = VTK_TRIANGLE;
    }

    void InsertPolygon(vtkIdType npts, vtkIdType *pts)
    {
      this->cellId = m_Cells->InsertNextCell(npts, pts);
      m_TypeArray[this->cellId] = VTK_POLYGON;
    }

    void InsertQuad(vtkIdType *pts)
    {
      this->cellId = m_Cells->InsertNextCell(4, pts);
      m_TypeArray[this->cellId] = VTK_QUAD;
    }
  };

  /*!
  \brief Implementation of the InsertImplementation interface of
  SwitchByCellType to define a visitor that distributes cells 
  according to their types (for vtkPolyData construction)
  */
  class DistributeInsertImplementation
  {
    vtkCellArray* m_LineCells;
    vtkCellArray* m_TriangleCells;
    vtkCellArray* m_PolygonCells;
    vtkCellArray* m_QuadCells;
  public:
    /*! Set the vtkCellArray that will be constructed
    */
    void SetCellArrays(vtkCellArray* lines, vtkCellArray* triangles, vtkCellArray* polygons, vtkCellArray* quads) 
    {
      m_LineCells = lines;
      m_TriangleCells = triangles;
      m_PolygonCells = polygons;
      m_QuadCells = quads;
    }

    void InsertLine(vtkIdType *pts)
    {
      m_LineCells->InsertNextCell(2, pts);
    }

    void InsertTriangle(vtkIdType *pts)
    {
      m_TriangleCells->InsertNextCell(3, pts);
    }

    void InsertPolygon(vtkIdType npts, vtkIdType *pts)
    {
      m_PolygonCells->InsertNextCell(npts, pts);
    }

    void InsertQuad(vtkIdType *pts)
    {
      m_QuadCells->InsertNextCell(4, pts);
    }
  };

  //typedef typename MeshType::CellType                CellType;
  //typedef typename itk::LineCell< CellType >         LineType;
  //typedef typename itk::PolygonCell< CellType >      PolygonType;
  //typedef typename itk::TriangleCell< CellType >     TriangleType;

  typedef SwitchByCellType<SingleCellArrayInsertImplementation> SingleCellArrayUserVisitorType;
  typedef SwitchByCellType<DistributeInsertImplementation> DistributeUserVisitorType;
  typedef ExactSwitchByCellType<DistributeInsertImplementation> ExactUserVisitorType;

public:

  /*!
  create an itkMesh object from a vtkPolyData
  */
  static typename MeshType::Pointer MeshFromPolyData(vtkPolyData* poly)
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
  static vtkUnstructuredGrid* MeshToUnstructuredGrid(typename MeshType::Pointer mesh)
  {
    /*! 
    default SingleCellArray line cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::LineCell< typename MeshType::CellType >, 
      SingleCellArrayUserVisitorType>			          SingleCellArrayLineVisitor;

    /*! 
    default SingleCellArray line cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::PolygonCell< typename MeshType::CellType >, 
      SingleCellArrayUserVisitorType>			          SingleCellArrayPolygonVisitor;

    /*! 
    default SingleCellArray triangle cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::TriangleCell<itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      SingleCellArrayUserVisitorType>			          SingleCellArrayTriangleVisitor;


    /*! 
    default SingleCellArray quad cell visitior definition
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType, typename MeshType::CellTraits,
      itk::QuadrilateralCell< itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      SingleCellArrayUserVisitorType>               SingleCellArrayQuadrilateralVisitor;



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
    typename SingleCellArrayTriangleVisitor::Pointer      tv = SingleCellArrayTriangleVisitor::New();
    typename SingleCellArrayQuadrilateralVisitor::Pointer qv = SingleCellArrayQuadrilateralVisitor::New();
    // 3. Set up the visitors
    int vtkCellCount = 0; // running counter for current cell being inserted into vtk
    int numCells = mesh->GetNumberOfCells();
    int *types = new int[numCells]; // type array for vtk 
    // create vtk cells and estimate the size
    vtkCellArray* cells = vtkCellArray::New();
    cells->Allocate(numCells);
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
  static vtkPolyData* MeshToPolyData(typename MeshType::Pointer mesh, bool onlyTriangles = false, bool useScalarAccessor = false, vtkPolyData* polydata = NULL)
  {
    /*! 
    default Distribute line cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::LineCell< typename MeshType::CellType >, 
      DistributeUserVisitorType>			          DistributeLineVisitor;

    /*! 
    default Distribute line cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::PolygonCell< typename MeshType::CellType >, 
      DistributeUserVisitorType>			          DistributePolygonVisitor;

    /*! 
    default Distribute triangle cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::TriangleCell<itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      DistributeUserVisitorType>			          DistributeTriangleVisitor;


    /*! 
    default Distribute quad cell visitior definition
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType, typename MeshType::CellTraits,
      itk::QuadrilateralCell< itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      DistributeUserVisitorType>               DistributeQuadrilateralVisitor;


    /*! 
    default Distribute triangle cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::TriangleCell<itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      ExactUserVisitorType>			              ExactTriangleVisitor;


    // Get the number of points in the mesh
    int numPoints = mesh->GetNumberOfPoints();
    if(numPoints == 0)
    {
      mesh->Print(std::cerr);
      std::cerr << "no points in Grid " << std::endl;
    }
    // Create a vtkUnstructuredGrid
    if(polydata == NULL)
      polydata = vtkPolyData::New();
    else
      polydata->Initialize();

    // Create the vtkPoints object and set the number of points
    vtkPoints* vpoints = vtkPoints::New();
    vpoints->SetNumberOfPoints(numPoints);

    vtkFloatArray * scalars = vtkFloatArray::New();
    scalars->SetNumberOfComponents(1);
    scalars->SetNumberOfTuples(numPoints);

    // iterate over all the points in the itk mesh filling in
    // the vtkPoints object as we go
    typename MeshType::PointsContainer::Pointer points = mesh->GetPoints();

    int idx=0;
    for(typename MeshType::PointsContainer::Iterator i = points->Begin();
      i != points->End(); ++i, ++idx)
    {
      // Get the point index from the point container iterator
      // int idx = i->Index(); //using this together with vtk::Points::SetPoint (no range checking!!) is unsafe!!
      
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
    polydata->SetPoints(vpoints);
    polydata->GetPointData()->SetScalars(scalars);
    polydata->GetPointData()->CopyAllOn();


    // Now create the cells using the MulitVisitor
    // 1. Create a MultiVisitor
    typedef typename MeshType::CellType::MultiVisitor MeshMV;
    typename MeshMV::Pointer mv = MeshMV::New();

    int numCells = mesh->GetNumberOfCells();

    if (onlyTriangles)
    {
      // create vtk cells and allocate
      vtkCellArray* trianglecells = vtkCellArray::New();
      trianglecells->Allocate(numCells);

      // 2. Create a triangle visitor and add it to the multivisitor
      typename ExactTriangleVisitor::Pointer tv = ExactTriangleVisitor::New();
      tv->SetCellArrays(NULL, trianglecells, NULL, NULL);
      mv->AddVisitor(tv);
      // 3. Now ask the mesh to accept the multivisitor which
      // will Call Visit for each cell in the mesh that matches the
      // cell types of the visitors added to the MultiVisitor
      mesh->Accept(mv);

      // 4. Set the result into our vtkPolyData
      if(trianglecells->GetNumberOfCells()>0)
        polydata->SetStrips(trianglecells); 

      // 5. Clean up vtk objects (no vtkSmartPointer ... )
      trianglecells->Delete();
    }
    else 
    {
      // create vtk cells and allocate
      vtkCellArray* linecells = vtkCellArray::New();
      vtkCellArray* trianglecells = vtkCellArray::New();
      vtkCellArray* polygoncells = vtkCellArray::New();
      linecells->Allocate(numCells);
      trianglecells->Allocate(numCells);
      polygoncells->Allocate(numCells);

      // 2. Create a visitors
      typename DistributeLineVisitor::Pointer lv          = DistributeLineVisitor::New();
      typename DistributePolygonVisitor::Pointer pv       = DistributePolygonVisitor::New();
      typename DistributeTriangleVisitor::Pointer tv      = DistributeTriangleVisitor::New();
      typename DistributeQuadrilateralVisitor::Pointer qv = DistributeQuadrilateralVisitor::New();

      lv->SetCellArrays(linecells, trianglecells, polygoncells, polygoncells);
      pv->SetCellArrays(linecells, trianglecells, polygoncells, polygoncells);
      tv->SetCellArrays(linecells, trianglecells, polygoncells, polygoncells);
      qv->SetCellArrays(linecells, trianglecells, polygoncells, polygoncells);

      // add the visitors to the multivisitor
      mv->AddVisitor(tv);
      mv->AddVisitor(lv);
      mv->AddVisitor(pv);
      mv->AddVisitor(qv);
      // 3. Now ask the mesh to accept the multivisitor which
      // will Call Visit for each cell in the mesh that matches the
      // cell types of the visitors added to the MultiVisitor
      mesh->Accept(mv);

      // 4. Set the result into our vtkPolyData
      if(linecells->GetNumberOfCells()>0)
        polydata->SetLines(linecells);  
      if(trianglecells->GetNumberOfCells()>0)
        polydata->SetStrips(trianglecells); 
      if(polygoncells->GetNumberOfCells()>0)
        polydata->SetPolys(polygoncells);  

      // 5. Clean up vtk objects (no vtkSmartPointer ... )
      linecells->Delete();
      trianglecells->Delete();
      polygoncells->Delete();
    }
    vpoints->Delete();

    //std::cout << "meshToUnstructuredGrid end" << std::endl;
    return polydata;
  }

  static typename MeshType::Pointer TranslateMesh(typename MeshType::PointType vec, typename MeshType::Pointer inputMesh)
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
