/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITKMESHUTIL_H_INCLUDED
#define MITKMESHUTIL_H_INCLUDED


#if(_MSC_VER==1200)
#error MeshUtils currently not supported for MS Visual C++ 6.0. Sorry.
#endif

//#include <itkMesh.h>
#include <itkLineCell.h>
#include <itkTriangleCell.h>
#include <itkPolygonCell.h>
#include <itkQuadrilateralCell.h>
#include <itkCellInterface.h>
//#include <itkDefaultDynamicMeshTraits.h> 
#include <itkSphereMeshSource.h> 
//#include <itkTransformMeshFilter.h>
//#include <itkTranslationTransform.h>
//#include <itkMinimumMaximumImageCalculator.h>
#include <itkAutomaticTopologyMeshSource.h>
#include <itkRegularSphereMeshSource.h>
#include <vnl/vnl_cross.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkProperty.h>
#include <vtkFloatArray.h>

#include <mitkGeometry3D.h>
#include <mitkSurface.h>

template <typename MeshType>
class NullScalarAccessor
{
public:
  static inline vtkFloatingPointType GetPointScalar(typename MeshType::PointDataContainer* /*pointData*/, typename MeshType::PointIdentifier /*idx*/, MeshType* /*mesh*/ = NULL, unsigned int /*type*/ = 0)
  {
    return (vtkFloatingPointType) 0.0;
  };

  static inline vtkFloatingPointType GetCellScalar(typename MeshType::CellDataContainer* /*cellData*/, typename MeshType::CellIdentifier /*idx*/, MeshType* /*mesh*/ = NULL, unsigned int /*type*/ = 0)
  {
    return (vtkFloatingPointType) 0.0;
  };
};

template <typename MeshType>
class MeshScalarAccessor
{
public:
  static inline vtkFloatingPointType GetPointScalar(typename MeshType::PointDataContainer* pointData, typename MeshType::PointIdentifier idx, MeshType* /*mesh*/ = NULL, unsigned int /*type*/ = 0)
  {
    return (vtkFloatingPointType)pointData->GetElement(idx);
  };

  static inline vtkFloatingPointType GetCellScalar(typename MeshType::CellDataContainer* cellData, typename MeshType::CellIdentifier idx, MeshType* /*mesh*/ = NULL, unsigned int /*type*/ = 0)
  {
    return (vtkFloatingPointType)cellData->GetElement(idx);
  };
};

template <typename MeshType>
class MeanCurvatureAccessor : public NullScalarAccessor<MeshType>
{
public:
  static inline vtkFloatingPointType GetPointScalar(typename MeshType::PointDataContainer* /*point*/, typename MeshType::PointIdentifier idx, MeshType* mesh, unsigned int /*type*/ = 0)
  {
    typename MeshType::PixelType dis = 0;    
    mesh->GetPointData(idx, &dis);
    return (vtkFloatingPointType) dis;
  };
};

template <typename MeshType>
class SimplexMeshAccessor : public NullScalarAccessor<MeshType>
{
public:
  static inline vtkFloatingPointType GetPointScalar(typename MeshType::PointDataContainer* point, typename MeshType::PointIdentifier idx, MeshType* mesh, unsigned int type = 0 )
  {
    typename MeshType::GeometryMapPointer geometryData = mesh->GetGeometryData();

    if (type == 0)
    {
      double val = mesh->GetMeanCurvature( idx );
      mesh->SetPointData(idx, val);
      return val;
    }
    else if (type == 1)
    {
      double val = geometryData->GetElement(idx)->meanTension;
      mesh->SetPointData(idx, val);
      return val;
    }
    else if (type == 2)
    {
      double val = geometryData->GetElement(idx)->externalForce.GetNorm();
      mesh->SetPointData(idx, val);
      return val;
    }
    else if (type == 3)
      return geometryData->GetElement(idx)->internalForce.GetNorm();
    else if (type == 4)
      return geometryData->GetElement(idx)->externalForce.GetNorm() *
      mesh->GetDistance(idx);
    else if (type == 5)
    {
      typename MeshType::PixelType dis = 0;    
      mesh->GetPointData(idx, &dis);
      return (vtkFloatingPointType) dis;
    }
    else if (type == 6)
    {
      return (vtkFloatingPointType) ((geometryData->GetElement(idx))->allowSplitting);
    }
    else
      return (vtkFloatingPointType) 0;
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
  void InsertTetra(vtkIdType *pts);
  void InsertHexahedron(vtkIdType *pts);
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
    typedef itk::TetrahedronCell<CellInterfaceType>   floatTetrahedronCell;
    typedef itk::HexahedronCell<CellInterfaceType>    floatHexahedronCell;
    typedef typename CellInterfaceType::PointIdConstIterator PointIdIterator;

  public:
    /*!
    Visit a line and create the VTK_LINE cell   
    */
    void Visit(unsigned long cellId, floatLineCell* t)
    {
      vtkIdType pts[2];
      int i=0;
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType vtkCellId = -1;
      if (num==2) { // useless because itk::LineCell always returns 2
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertLine( (vtkIdType*)pts );
      }

      if (this->m_UseCellScalarAccessor && vtkCellId >= 0)
      {
        this->m_CellScalars->InsertTuple1(vtkCellId,
          ScalarAccessor::GetCellScalar(this->m_CellData, cellId));
      }
    }

    /*!
    Visit a polygon and create the VTK_POLYGON cell   
    */
    void Visit(unsigned long cellId, floatPolygonCell* t)
    {
      vtkIdType pts[4096];
      int i=0;
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType vtkCellId = -1;
      if (num > 4096) {
        LOG_ERROR << "Problem in mitkMeshUtil: Polygon with more than maximum number of vertices encountered." << std::endl;
      }
      else if (num > 3) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertPolygon( num, (vtkIdType*)pts );
      }
      else if (num == 3) { 
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertTriangle( (vtkIdType*)pts );
      }
      else if (num==2) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertLine( (vtkIdType*)pts );
      }

      if (this->m_UseCellScalarAccessor && vtkCellId >= 0)
      {
        this->m_CellScalars->InsertTuple1(vtkCellId,
          ScalarAccessor::GetCellScalar(this->m_CellData, cellId));
      }
    }

    /*!
    Visit a triangle and create the VTK_TRIANGLE cell   
    */
    void Visit(unsigned long cellId, floatTriangleCell* t)
    {
      vtkIdType pts[3];
      int i=0;
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType vtkCellId = -1;
      if (num == 3) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertTriangle( (vtkIdType*)pts );
      }
      else if (num==2) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertLine( (vtkIdType*)pts );
      }

      if (this->m_UseCellScalarAccessor && vtkCellId >= 0)
      {
        this->m_CellScalars->InsertTuple1(vtkCellId,
          ScalarAccessor::GetCellScalar(this->m_CellData, cellId));
      }
    }

    /*! 
    Visit a quad and create the VTK_QUAD cell 
    */
    void Visit(unsigned long cellId, floatQuadrilateralCell* t)
    {
      vtkIdType pts[4];
      int i=0;
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType vtkCellId = -1;
      if (num == 4) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) 
        {
          if (i == 2) pts[3] = *it;
          else if (i == 3) pts[2] = *it;
          else pts[i] = *it;
          i++;
          //pts[i++] = *it;
        }
        vtkCellId = this->InsertQuad( (vtkIdType*)pts );
      }
      else if (num == 3) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertTriangle( (vtkIdType*)pts );
      }
      else if (num==2) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertLine( (vtkIdType*)pts );
      }

      if (this->m_UseCellScalarAccessor && vtkCellId >= 0)
      {
        this->m_CellScalars->InsertTuple1(vtkCellId,
          ScalarAccessor::GetCellScalar(this->m_CellData, cellId));
      }
    }

    /*! 
    Visit a tetrahedra and create the VTK_TETRA cell 
    */
    void Visit(unsigned long cellId, floatTetrahedronCell* t)
    {
      vtkIdType pts[4];
      int i=0;
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType vtkCellId = -1;
      if (num == 4) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertTetra( (vtkIdType*)pts );
      }
      else if (num == 3) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertTriangle( (vtkIdType*)pts );
      }
      else if (num==2) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertLine( (vtkIdType*)pts );
      }

      if (this->m_UseCellScalarAccessor && vtkCellId >= 0)
      {
        this->m_CellScalars->InsertTuple1(vtkCellId,
          ScalarAccessor::GetCellScalar(this->m_CellData, cellId));
      }
    }

    /*! 
    Visit a hexahedron and create the VTK_HEXAHEDRON cell 
    */
    void Visit(unsigned long cellId, floatHexahedronCell* t)
    {
      vtkIdType pts[8];
      int i=0;
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType vtkCellId = -1;
      if (num == 8) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++)
        {
          if (i == 2)
            pts[i++] = *(it+1);
          else if (i == 3)
            pts[i++] = *(it-1);
          else if (i == 6)
            pts[i++] = *(it+1);
          else if (i == 7)
            pts[i++] = *(it-1);
          else
            pts[i++] = *it;
        }
        vtkCellId = this->InsertHexahedron( (vtkIdType*)pts );
      }
      else if (num == 4) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertQuad( (vtkIdType*)pts );
      }
      else if (num == 3) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertTriangle( (vtkIdType*)pts );
      }
      else if (num==2) {
        for (PointIdIterator it=t->PointIdsBegin(); it!=t->PointIdsEnd(); it++) pts[i++] = *it;
        vtkCellId = this->InsertLine( (vtkIdType*)pts );
      }

      if (this->m_UseCellScalarAccessor && vtkCellId >= 0)
      {
        this->m_CellScalars->InsertTuple1(vtkCellId,
          ScalarAccessor::GetCellScalar(this->m_CellData, cellId));
      }
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
    typedef itk::TetrahedronCell<CellInterfaceType>   floatTetrahedronCell;
    typedef itk::HexahedronCell<CellInterfaceType>    floatHexahedronCell;

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
    Visit a polygon and create the VTK_POLYGON cell   
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
    Visit a quadrilateral and create the VTK_QUAD cell 
    */
    void Visit(unsigned long , floatQuadrilateralCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num == 4) {
        vtkIdType tmpId = pts[3];
        pts[3] = pts[4];
        pts[4] = tmpId;
        this->InsertQuad(pts);
      }
    }

    /*! 
    Visit a tetrahedron and create the VTK_TETRA cell 
    */
    void Visit(unsigned long , floatTetrahedronCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num == 4) 
        this->InsertTetra(pts);
    }

    /*! 
    Visit a hexahedron and create the VTK_HEXAHEDRON cell 
    */
    void Visit(unsigned long , floatHexahedronCell* t)
    {
      unsigned long num = t->GetNumberOfVertices();
      vtkIdType *pts = (vtkIdType*)t->PointIdsBegin();
      if (num == 8) 
      {
        vtkIdType tmp[8];
        for (unsigned int i = 0; i < 8; i++) tmp[i] = pts[i];
        pts[2] = tmp[3];
        pts[3] = tmp[2];
        pts[6] = tmp[7];
        pts[7] = tmp[6];
        this->InsertHexahedron(pts);
      }
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
    //vtkIdType cellId;

  protected:
    bool m_UseCellScalarAccessor;
    vtkFloatArray* m_CellScalars;
    typename MeshType::CellDataContainer::Pointer m_CellData;

  public:

    SingleCellArrayInsertImplementation() : m_UseCellScalarAccessor(false) {}

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

    void SetUseCellScalarAccessor(bool flag) 
    {
      m_UseCellScalarAccessor = flag;
    }

    void SetCellScalars(vtkFloatArray* scalars)
    {
      m_CellScalars = scalars;
    }

    vtkFloatArray* GetCellScalars() { return m_CellScalars; }

    void SetMeshCellData(typename MeshType::CellDataContainer* data)
    {
      m_CellData = data;
    }

    vtkIdType InsertLine(vtkIdType *pts)
    {
      vtkIdType cellId = m_Cells->InsertNextCell(2, pts);
      m_TypeArray[cellId] = VTK_LINE;
      return cellId;
    }

    vtkIdType InsertTriangle(vtkIdType *pts)
    {
      vtkIdType cellId = m_Cells->InsertNextCell(3, pts);
      m_TypeArray[cellId] = VTK_TRIANGLE;
      return cellId;
    }

    vtkIdType InsertPolygon(vtkIdType npts, vtkIdType *pts)
    {
      vtkIdType cellId = m_Cells->InsertNextCell(npts, pts);
      m_TypeArray[cellId] = VTK_POLYGON;
      return cellId;
    }

    vtkIdType InsertQuad(vtkIdType *pts)
    {
      vtkIdType cellId = m_Cells->InsertNextCell(4, pts);
      m_TypeArray[cellId] = VTK_QUAD;
      return cellId;
    }

    vtkIdType InsertTetra(vtkIdType *pts)
    {
      vtkIdType cellId = m_Cells->InsertNextCell(4, pts);
      m_TypeArray[cellId] = VTK_TETRA;
      return cellId;
    }

    vtkIdType InsertHexahedron(vtkIdType *pts)
    {
      vtkIdType cellId = m_Cells->InsertNextCell(8, pts);
      m_TypeArray[cellId] = VTK_HEXAHEDRON;
      return cellId;
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

  protected:
    bool m_UseCellScalarAccessor;
    vtkFloatArray* m_CellScalars;
    typename MeshType::CellDataContainer::Pointer m_CellData;

  public:

    DistributeInsertImplementation() : m_UseCellScalarAccessor(false) {}


    /*! Set the vtkCellArray that will be constructed
    */
    void SetCellArrays(vtkCellArray* lines, vtkCellArray* triangles, vtkCellArray* polygons, vtkCellArray* quads) 
    {
      m_LineCells = lines;
      m_TriangleCells = triangles;
      m_PolygonCells = polygons;
      m_QuadCells = quads;
    }

    vtkIdType InsertLine(vtkIdType *pts)
    {
      return m_LineCells->InsertNextCell(2, pts);
    }

    vtkIdType InsertTriangle(vtkIdType *pts)
    {
      return m_TriangleCells->InsertNextCell(3, pts);
    }

    vtkIdType InsertPolygon(vtkIdType npts, vtkIdType *pts)
    {
      return m_PolygonCells->InsertNextCell(npts, pts);
    }

    vtkIdType InsertQuad(vtkIdType *pts)
    {
      return m_QuadCells->InsertNextCell(4, pts);
    }

    vtkIdType InsertTetra(vtkIdType *pts) { return -1; } // ignored
    vtkIdType InsertHexahedron(vtkIdType *pts) { return -1; } // ignored
  };

  //typedef typename MeshType::CellType                CellType;
  //typedef typename itk::LineCell< CellType >         LineType;
  //typedef typename itk::PolygonCell< CellType >      PolygonType;
  //typedef typename itk::TriangleCell< CellType >     TriangleType;

  typedef SwitchByCellType<SingleCellArrayInsertImplementation> SingleCellArrayUserVisitorType;
  typedef SwitchByCellType<DistributeInsertImplementation> DistributeUserVisitorType;
  typedef ExactSwitchByCellType<DistributeInsertImplementation> ExactUserVisitorType;

public:

  typedef itk::MatrixOffsetTransformBase<typename MeshType::CoordRepType,3,3> ITKTransformType;
  typedef itk::MatrixOffsetTransformBase<mitk::ScalarType,3,3>       MITKTransformType;

  /*! 
  Convert a MITK transformation to an ITK transformation 
  Necessary because ITK uses double and MITK uses float values
  */
  static void ConvertTransformToItk(const MITKTransformType* mitkTransform, ITKTransformType* itkTransform)
  {
    typename MITKTransformType::MatrixType mitkM = mitkTransform->GetMatrix();
    typename ITKTransformType::MatrixType itkM;

    typename MITKTransformType::OffsetType mitkO = mitkTransform->GetOffset();
    typename ITKTransformType::OffsetType itkO;

    for(short i = 0; i < 3; ++i)
    {
      for(short j = 0; j<3; ++j)
      {
        itkM[i][j] = (double)mitkM[i][j];
      }
      itkO[i] = (double)mitkO[i];
    }

    itkTransform->SetMatrix(itkM);
    itkTransform->SetOffset(itkO);
  }

  /*!
  create an itkMesh object from a vtkPolyData
  */
  static typename MeshType::Pointer MeshFromPolyData(vtkPolyData* poly, mitk::Geometry3D* geometryFrame=NULL, mitk::Geometry3D* polyDataGeometryFrame=NULL)
  {
    // Create a new mesh
    typename MeshType::Pointer output = MeshType::New();
    output->SetCellsAllocationMethod( MeshType::CellsAllocatedDynamicallyCellByCell );  

    typedef typename MeshType::CellDataContainer MeshCellDataContainerType;

    output->SetCellData(MeshCellDataContainerType::New());

    // Get the points from vtk
    vtkPoints* vtkpoints = poly->GetPoints();
    const unsigned int numPoints = poly->GetNumberOfPoints();

    // Create a compatible point container for the mesh
    // the mesh is created with a null points container
    // MeshType::PointsContainer::Pointer points = 
    //   MeshType::PointsContainer::New();
    // // Resize the point container to be able to fit the vtk points
    // points->Reserve(numPoints);
    // // Set the point container on the mesh
    //output->SetPoints(points);
    vtkFloatingPointType vtkpoint[3];
    typename MeshType::PointType itkPhysicalPoint;
    if(geometryFrame==NULL)
    {
      if(polyDataGeometryFrame==NULL)
      {
        for(unsigned int i=0; i < numPoints; ++i)
        {
          vtkpoints->GetPoint(i, vtkpoint);
          //LOG_INFO << "next point: " << test[0]<< "," << test[1] << "," << test[2] << std::endl;
          //typename MeshType::PixelType* apoint = (typename MeshType::PixelType*) vtkpoints->GetPoint(i);
          mitk::vtk2itk(vtkpoint, itkPhysicalPoint);
          output->SetPoint( i, itkPhysicalPoint );
        }
      }
      else
      {
        for(unsigned int i=0; i < numPoints; ++i)
        {
          vtkpoints->GetPoint(i, vtkpoint);
          //LOG_INFO << "next point: " << test[0]<< "," << test[1] << "," << test[2] << std::endl;
          //typename MeshType::PixelType* apoint = (typename MeshType::PixelType*) vtkpoints->GetPoint(i);
          mitk::Point3D mitkWorldPoint;
          mitk::vtk2itk(vtkpoint, mitkWorldPoint);
          polyDataGeometryFrame->IndexToWorld(mitkWorldPoint, mitkWorldPoint);
          mitk::vtk2itk(mitkWorldPoint, itkPhysicalPoint);
          output->SetPoint( i, itkPhysicalPoint );
        }
      }
    }
    else
    {
      mitk::Point3D mitkWorldPoint;
      if(polyDataGeometryFrame==NULL)
      {
        for(unsigned int i=0; i < numPoints; ++i)
        {
          vtkpoints->GetPoint(i, vtkpoint);
          //LOG_INFO << "next point: " << test[0]<< "," << test[1] << "," << test[2] << std::endl;
          //typename MeshType::PixelType* apoint = (typename MeshType::PixelType*) vtkpoints->GetPoint(i);
          mitk::vtk2itk(vtkpoint, mitkWorldPoint);
          geometryFrame->WorldToItkPhysicalPoint(mitkWorldPoint, itkPhysicalPoint);
          output->SetPoint( i, itkPhysicalPoint );
        }
      }
      else
      {
        for(unsigned int i=0; i < numPoints; ++i)
        {
          vtkpoints->GetPoint(i, vtkpoint);
          //LOG_INFO << "next point: " << test[0]<< "," << test[1] << "," << test[2] << std::endl;
          //typename MeshType::PixelType* apoint = (typename MeshType::PixelType*) vtkpoints->GetPoint(i);
          mitk::vtk2itk(vtkpoint, mitkWorldPoint);
          polyDataGeometryFrame->IndexToWorld(mitkWorldPoint, mitkWorldPoint);
          geometryFrame->WorldToItkPhysicalPoint(mitkWorldPoint, itkPhysicalPoint);
          output->SetPoint( i, itkPhysicalPoint );
        }
      }
    }

    vtkCellArray* vtkcells = poly->GetPolys();
    //    vtkCellArray* vtkcells = poly->GetStrips();
    //MeshType::CellsContainerPointer cells = MeshType::CellsContainer::New();
    //output->SetCells(cells);
    // extract the cell id's from the vtkUnstructuredGrid
    int numcells = vtkcells->GetNumberOfCells();
    int* vtkCellTypes = new int[numcells];
    int cellId = 0;
    // poly ids start after verts and lines!
    int cellIdOfs = poly->GetNumberOfVerts() + poly->GetNumberOfLines();
    for(; cellId < numcells; cellId++)
    {
      vtkCellTypes[cellId] = poly->GetCellType( cellId+cellIdOfs );
    }

    // cells->Reserve(numcells);
    vtkIdType npts;
    vtkIdType* pts;
    cellId = 0;

    typedef typename MeshType::MeshTraits   OMeshTraits;
    typedef typename OMeshTraits::PixelType       OPixelType; 
    typedef typename MeshType::CellTraits          CellTraits;
    typedef typename itk::CellInterface<OPixelType, CellTraits>   CellInterfaceType;
    typedef typename itk::TriangleCell<CellInterfaceType>         TriCellType;
    typedef typename TriCellType::CellAutoPointer             TriCellPointer;

    TriCellPointer newCell;
    output->GetCells()->Reserve( poly->GetNumberOfPolys() + poly->GetNumberOfStrips() );
    output->GetCellData()->Reserve( poly->GetNumberOfPolys() + poly->GetNumberOfStrips() );

    for(vtkcells->InitTraversal(); vtkcells->GetNextCell(npts, pts); cellId++)
    {
      switch(vtkCellTypes[cellId])
      {
      case VTK_TRIANGLE:
        {
          if (npts != 3) continue; // skip non-triangles;
          unsigned long pointIds[3];
          pointIds[0] = (unsigned long) pts[0];
          pointIds[1] = (unsigned long) pts[1];
          pointIds[2] = (unsigned long) pts[2];

          newCell.TakeOwnership( new TriCellType );
          newCell->SetPointIds(pointIds);//(unsigned long*)pts);
          output->SetCell(cellId, newCell );
          output->SetCellData(cellId, (typename MeshType::PixelType)3);
          break;    
        } 

      case VTK_QUAD:
        {
          if (npts != 4 ) continue; // skip non-quadrilateral
          unsigned long pointIds[3];

          pointIds[0] = (unsigned long) pts[0];
          pointIds[1] = (unsigned long) pts[1];
          pointIds[2] = (unsigned long) pts[2];
          newCell.TakeOwnership( new TriCellType );
          newCell->SetPointIds(pointIds);
          output->SetCell(cellId, newCell );
          output->SetCellData(cellId, (typename MeshType::PixelType)3);
          cellId++;

          pointIds[0] = (unsigned long) pts[2];
          pointIds[1] = (unsigned long) pts[3];
          pointIds[2] = (unsigned long) pts[0];
          newCell.TakeOwnership( new TriCellType );
          newCell->SetPointIds(pointIds);
          output->SetCell(cellId, newCell );
          output->SetCellData(cellId, (typename MeshType::PixelType)3);
          break;
        }

      case VTK_EMPTY_CELL:
        {
          if (npts != 3) 
          {
            LOG_ERROR << "Only empty triangle cell supported by now..." << std::endl; // skip non-triangle empty cells;
            continue;
          }
          unsigned long pointIds[3];
          pointIds[0] = (unsigned long) pts[0];
          pointIds[1] = (unsigned long) pts[1];
          pointIds[2] = (unsigned long) pts[2];

          newCell.TakeOwnership( new TriCellType );
          newCell->SetPointIds(pointIds);
          output->SetCell(cellId, newCell );
          output->SetCellData(cellId, (typename MeshType::PixelType)3);
          break;    
        } 

        //case VTK_VERTEX:              // If need to implement use 
        //case VTK_POLY_VERTEX:         // the poly->GetVerts() and 
        //case VTK_LINE:                // poly->GetLines() routines 
        //case VTK_POLY_LINE:           // outside of the switch..case.
      case VTK_POLYGON:
      case VTK_PIXEL:
        {
          if (npts != 4 ) continue;// skip non-quadrilateral
          unsigned long pointIds[3];
          for ( unsigned int idx = 0; idx <= 1; idx++ )
          {
            pointIds[0] = (unsigned long) pts[idx];
            pointIds[1] = (unsigned long) pts[idx+1];
            pointIds[2] = (unsigned long) pts[idx+2];
            newCell.TakeOwnership( new TriCellType );
            newCell->SetPointIds(pointIds);
            output->SetCell(cellId+idx, newCell );
            output->SetCellData(cellId+idx, (typename MeshType::PixelType)3);
          }
          cellId++;
          break;
        }

      case VTK_TETRA:
      case VTK_VOXEL:
      case VTK_HEXAHEDRON:
      case VTK_WEDGE:
      case VTK_PYRAMID:
      case VTK_PARAMETRIC_CURVE:
      case VTK_PARAMETRIC_SURFACE:
      default:
        LOG_WARN << "Warning, unhandled cell type " 
          << vtkCellTypes[cellId] << std::endl;
      }
    }

    if (poly->GetNumberOfStrips() != 0) 
    {
      vtkcells = poly->GetStrips();
      numcells = vtkcells->GetNumberOfCells();
      vtkCellTypes = new int[numcells];
      int stripId = 0;
      // strip ids start after verts, lines and polys!
      int stripIdOfs = poly->GetNumberOfVerts() + poly->GetNumberOfLines() + poly->GetNumberOfPolys(); 
      for(; stripId < numcells; stripId++)
      {
        vtkCellTypes[stripId] = poly->GetCellType( stripId+stripIdOfs );
      }
      stripId = 0;

      vtkcells->InitTraversal();
      while( vtkcells->GetNextCell(npts, pts) )
      {
        if (vtkCellTypes[stripId] != VTK_TRIANGLE_STRIP) 
        {
          LOG_ERROR << "Only triangle strips supported!" << std::endl;
          continue;
        }
        stripId++;

        unsigned int numberOfTrianglesInStrip = npts - 2;
        unsigned long pointIds[3];
        pointIds[0] = (unsigned long) pts[0];
        pointIds[1] = (unsigned long) pts[1];
        pointIds[2] = (unsigned long) pts[2];

        for( unsigned int t=0; t < numberOfTrianglesInStrip; t++ )
        {
          newCell.TakeOwnership( new TriCellType );
          newCell->SetPointIds(pointIds);
          output->SetCell(cellId, newCell );
          output->SetCellData(cellId, (typename MeshType::PixelType)3);
          cellId++;
          pointIds[0] = pointIds[1];
          pointIds[1] = pointIds[2];
          pointIds[2] = pts[t+3];
        }
      }
    }   
    //output->Print(std::cout);
    output->BuildCellLinks();
    delete[] vtkCellTypes;
    return output;
  }

  /*!
  create an itkMesh object from an mitk::Surface
  */
  static typename MeshType::Pointer MeshFromSurface(mitk::Surface* surface, mitk::Geometry3D* geometryFrame=NULL)
  {
    if(surface == NULL)
      return NULL;
    return MeshFromPolyData(surface->GetVtkPolyData(), geometryFrame, surface->GetGeometry());
  }

  /*!
  create an vtkUnstructuredGrid object from an itkMesh
  */
  static vtkUnstructuredGrid* MeshToUnstructuredGrid(
    MeshType* mesh, 
    bool usePointScalarAccessor = false, 
    bool useCellScalarAccessor = false, 
    unsigned int pointDataType = 0, 
    mitk::Geometry3D* geometryFrame=NULL)
  {
    /*! 
    default SingleCellArray line cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::LineCell< typename MeshType::CellType >, 
      SingleCellArrayUserVisitorType>                SingleCellArrayLineVisitor;

    /*! 
    default SingleCellArray polygon cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::PolygonCell< typename MeshType::CellType >, 
      SingleCellArrayUserVisitorType>                SingleCellArrayPolygonVisitor;

    /*! 
    default SingleCellArray triangle cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::TriangleCell<itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      SingleCellArrayUserVisitorType>                SingleCellArrayTriangleVisitor;


    /*! 
    default SingleCellArray quad cell visitior definition
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType, typename MeshType::CellTraits,
      itk::QuadrilateralCell< itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      SingleCellArrayUserVisitorType>               SingleCellArrayQuadrilateralVisitor;


    /*! 
    default SingleCellArray tetra cell visitior definition
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType, typename MeshType::CellTraits,
      itk::TetrahedronCell< itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      SingleCellArrayUserVisitorType>               SingleCellArrayTetrahedronVisitor;


    /*! 
    default SingleCellArray hex cell visitior definition
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType, typename MeshType::CellTraits,
      itk::HexahedronCell< itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      SingleCellArrayUserVisitorType>               SingleCellArrayHexahedronVisitor;

    // Get the number of points in the mesh
    int numPoints = mesh->GetNumberOfPoints();
    if(numPoints == 0)
    {
      //mesh->Print(std::cerr);
      LOG_FATAL << "no points in Grid " << std::endl;
      exit(-1);
    }
    // Create a vtkUnstructuredGrid
    vtkUnstructuredGrid* vgrid = vtkUnstructuredGrid::New();
    // Create the vtkPoints object and set the number of points
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    vtkPoints* vpoints = vtkPoints::New( VTK_DOUBLE );
#else
    vtkPoints* vpoints = vtkPoints::New();
#endif
    vtkFloatArray* pointScalars = vtkFloatArray::New();
    vtkFloatArray* cellScalars = vtkFloatArray::New();
    pointScalars->SetNumberOfComponents(1);
    cellScalars->SetNumberOfComponents(1);

    typename MeshType::PointsContainer::Pointer points = mesh->GetPoints();
    typename MeshType::PointsContainer::Iterator i;

    // iterate over all the points in the itk mesh to find
    // the maximal index
    unsigned int maxIndex = 0;
    for(i = points->Begin(); i != points->End(); ++i)
    {
      if(maxIndex < i->Index())
        maxIndex = i->Index();
    }

    // initialize vtk-classes for points and scalars
    vpoints->SetNumberOfPoints(maxIndex+1);
    pointScalars->SetNumberOfTuples(maxIndex+1);
    cellScalars->SetNumberOfTuples(mesh->GetNumberOfCells());

    vtkFloatingPointType vtkpoint[3];
    typename MeshType::PointType itkPhysicalPoint;
    if (geometryFrame == 0)
    {
      for(i = points->Begin(); i != points->End(); ++i)
      {
        // Get the point index from the point container iterator
        int idx = i->Index();

        itkPhysicalPoint = i->Value();
        mitk::itk2vtk(itkPhysicalPoint, vtkpoint);
        // Set the vtk point at the index with the the coord array from itk
        vpoints->SetPoint(idx, vtkpoint);

        if(usePointScalarAccessor)
        {   
          pointScalars->InsertTuple1( idx, ScalarAccessor::GetPointScalar( mesh->GetPointData(), i->Index(), mesh, pointDataType ) );
        }
      }
    }
    else
    {
      mitk::Point3D mitkWorldPoint;
      for(i = points->Begin(); i != points->End(); ++i)
      {
        // Get the point index from the point container iterator
        int idx = i->Index();

        itkPhysicalPoint = i->Value();
        geometryFrame->ItkPhysicalPointToWorld(itkPhysicalPoint, mitkWorldPoint);
        mitk::itk2vtk(mitkWorldPoint, vtkpoint);
        // Set the vtk point at the index with the the coord array from itk
        vpoints->SetPoint(idx, vtkpoint);

        if(usePointScalarAccessor)
        {   
          pointScalars->InsertTuple1( idx, ScalarAccessor::GetPointScalar( mesh->GetPointData(), i->Index(), mesh, pointDataType ) );
        }
      }
    }
    // Set the points on the vtk grid
    vgrid->SetPoints(vpoints);
    if (usePointScalarAccessor)
      vgrid->GetPointData()->SetScalars(pointScalars);

    // Now create the cells using the MultiVisitor
    // 1. Create a MultiVisitor
    typename MeshType::CellType::MultiVisitor::Pointer mv =
      MeshType::CellType::MultiVisitor::New();
    // 2. Create visitors
    typename SingleCellArrayLineVisitor::Pointer      lv = SingleCellArrayLineVisitor::New();
    typename SingleCellArrayPolygonVisitor::Pointer      pv = SingleCellArrayPolygonVisitor::New();
    typename SingleCellArrayTriangleVisitor::Pointer      tv = SingleCellArrayTriangleVisitor::New();
    typename SingleCellArrayQuadrilateralVisitor::Pointer qv = SingleCellArrayQuadrilateralVisitor::New();
    typename SingleCellArrayTetrahedronVisitor::Pointer tetv = SingleCellArrayTetrahedronVisitor::New();
    typename SingleCellArrayHexahedronVisitor::Pointer hv = SingleCellArrayHexahedronVisitor::New();
    // 3. Set up the visitors
    //int vtkCellCount = 0; // running counter for current cell being inserted into vtk
    int numCells = mesh->GetNumberOfCells();
    int *types = new int[numCells]; // type array for vtk 
    // create vtk cells and estimate the size
    vtkCellArray* cells = vtkCellArray::New();
    cells->Allocate(numCells);
    // Set the TypeArray CellCount and CellArray for the visitors
    lv->SetTypeArray(types);
    lv->SetCellArray(cells);
    pv->SetTypeArray(types);
    pv->SetCellArray(cells);
    tv->SetTypeArray(types);
    //tv->SetCellCounter(&vtkCellCount);
    tv->SetCellArray(cells);
    qv->SetTypeArray(types);
    //qv->SetCellCounter(&vtkCellCount);
    qv->SetCellArray(cells);
    tetv->SetTypeArray(types);
    tetv->SetCellArray(cells);
    hv->SetTypeArray(types);
    hv->SetCellArray(cells);

    if (useCellScalarAccessor)
    {
      lv->SetUseCellScalarAccessor(true);
      lv->SetCellScalars(cellScalars);
      lv->SetMeshCellData(mesh->GetCellData());

      pv->SetUseCellScalarAccessor(true);
      pv->SetCellScalars(cellScalars);
      pv->SetMeshCellData(mesh->GetCellData());

      tv->SetUseCellScalarAccessor(true);
      tv->SetCellScalars(cellScalars);
      tv->SetMeshCellData(mesh->GetCellData());

      qv->SetUseCellScalarAccessor(true);
      qv->SetCellScalars(cellScalars);
      qv->SetMeshCellData(mesh->GetCellData());

      tetv->SetUseCellScalarAccessor(true);
      tetv->SetCellScalars(cellScalars);
      tetv->SetMeshCellData(mesh->GetCellData());

      hv->SetUseCellScalarAccessor(true);
      hv->SetCellScalars(cellScalars);
      hv->SetMeshCellData(mesh->GetCellData());
    }

    // add the visitors to the multivisitor
    mv->AddVisitor(lv);
    mv->AddVisitor(pv);
    mv->AddVisitor(tv);
    mv->AddVisitor(qv);
    mv->AddVisitor(tetv);
    mv->AddVisitor(hv);
    // Now ask the mesh to accept the multivisitor which
    // will Call Visit for each cell in the mesh that matches the
    // cell types of the visitors added to the MultiVisitor
    mesh->Accept(mv);
    // Now set the cells on the vtk grid with the type array and cell array

    vgrid->SetCells(types, cells);
    vgrid->GetCellData()->SetScalars(cellScalars);

    // Clean up vtk objects (no vtkSmartPointer ... )
    cells->Delete();
    vpoints->Delete();
    delete[] types;

    pointScalars->Delete();
    cellScalars->Delete();
    //LOG_INFO << "meshToUnstructuredGrid end" << std::endl;
    return vgrid;
  }


  /*!
  create a vtkPolyData object from an itkMesh
  */
  static vtkPolyData* MeshToPolyData(MeshType* mesh, bool onlyTriangles = false, bool useScalarAccessor = false, unsigned int pointDataType = 0, mitk::Geometry3D* geometryFrame=NULL, vtkPolyData* polydata = NULL)
  {
    /*! 
    default Distribute line cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::LineCell< typename MeshType::CellType >, 
      DistributeUserVisitorType>                DistributeLineVisitor;

    /*! 
    default Distribute polygon cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::PolygonCell< typename MeshType::CellType >, 
      DistributeUserVisitorType>                DistributePolygonVisitor;

    /*! 
    default Distribute triangle cell visitior definition 
    */
    typedef typename itk::CellInterfaceVisitorImplementation<typename MeshType::CellPixelType,
      typename MeshType::CellTraits,
      itk::TriangleCell<itk::CellInterface<typename MeshType::CellPixelType, typename MeshType::CellTraits > >, 
      DistributeUserVisitorType>                DistributeTriangleVisitor;


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
      ExactUserVisitorType>                    ExactTriangleVisitor;


    // Get the number of points in the mesh
    int numPoints = mesh->GetNumberOfPoints();
    if(numPoints == 0)
    {
      //mesh->Print(std::cerr);
      LOG_ERROR << "no points in Grid " << std::endl;
    }
    // Create a vtkPolyData
    if(polydata == NULL)
      polydata = vtkPolyData::New();
    else
      polydata->Initialize();

    // Create the vtkPoints object and set the number of points
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    vtkPoints* vpoints = vtkPoints::New( VTK_DOUBLE );
#else
    vtkPoints* vpoints = vtkPoints::New();
#endif
    vtkFloatArray * scalars = vtkFloatArray::New();
    scalars->SetNumberOfComponents(1);

    typename MeshType::PointsContainer::Pointer points = mesh->GetPoints();
    typename MeshType::PointsContainer::Iterator i;

    // iterate over all the points in the itk mesh to find
    // the maximal index
    unsigned int maxIndex = 0;
    for(i = points->Begin(); i != points->End(); ++i)
    {
      if(maxIndex < i->Index())
        maxIndex = i->Index();
    }

    // initialize vtk-classes for points and scalars
    vpoints->SetNumberOfPoints(maxIndex+1);
    scalars->SetNumberOfTuples(maxIndex+1);

    // iterate over all the points in the itk mesh filling in
    // the vtkPoints object as we go

    vtkFloatingPointType vtkpoint[3];
    typename MeshType::PointType itkPhysicalPoint;
    if(geometryFrame==NULL)
    {
      for(i = points->Begin(); i != points->End(); ++i)
      {
        // Get the point index from the point container iterator
        int idx = i->Index();

        itkPhysicalPoint = i->Value();
        mitk::itk2vtk(itkPhysicalPoint, vtkpoint);
        // Set the vtk point at the index with the the coord array from itk
        // itk returns a const pointer, but vtk is not const correct, so
        // we have to use a const cast to get rid of the const
        //      vpoints->SetPoint(idx, const_cast<DATATYPE*>(i->Value().GetDataPointer()));
        vpoints->SetPoint(idx, vtkpoint);

        if(useScalarAccessor)
        {   
          scalars->InsertTuple1( idx, ScalarAccessor::GetPointScalar( mesh->GetPointData(), i->Index(), mesh, pointDataType ) );
        }
      }
    }
    else
    {
      mitk::Point3D mitkWorldPoint;
      for(i = points->Begin(); i != points->End(); ++i)
      {
        // Get the point index from the point container iterator
        int idx = i->Index();

        itkPhysicalPoint = i->Value();
        geometryFrame->ItkPhysicalPointToWorld(itkPhysicalPoint, mitkWorldPoint);
        mitk::itk2vtk(mitkWorldPoint, vtkpoint);
        // Set the vtk point at the index with the the coord array from itk
        // itk returns a const pointer, but vtk is not const correct, so
        // we have to use a const cast to get rid of the const
        //      vpoints->SetPoint(idx, const_cast<DATATYPE*>(i->Value().GetDataPointer()));
        vpoints->SetPoint(idx, vtkpoint);

        if(useScalarAccessor)
        {   
          scalars->InsertTuple1( idx, ScalarAccessor::GetPointScalar( mesh->GetPointData(), i->Index(), mesh, pointDataType ) );
        }
      }
    }

    // Set the points on the vtk grid
    polydata->SetPoints(vpoints);
    if (useScalarAccessor)
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

      // 2. Create visitors
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
    scalars->Delete();

    //LOG_INFO << "meshToPolyData end" << std::endl;
    return polydata;
  }

  static typename MeshType::Pointer CreateRegularSphereMesh(typename MeshType::PointType center, typename MeshType::PointType::VectorType scale, int resolution)
  {
    typedef itk::RegularSphereMeshSource<MeshType> SphereSourceType;
    typename SphereSourceType::Pointer mySphereSource = SphereSourceType::New();

    mySphereSource->SetCenter(center);
    mySphereSource->SetScale(scale);
    mySphereSource->SetResolution( resolution );
    mySphereSource->Update();

    typename MeshType::Pointer resultMesh = mySphereSource->GetOutput();
    resultMesh->Register(); // necessary ???? 
    return resultMesh;
  }

  static typename MeshType::Pointer CreateSphereMesh(typename MeshType::PointType center, typename MeshType::PointType scale, int* resolution)
  {
    typedef typename itk::SphereMeshSource<MeshType> SphereSource;

    typename SphereSource::Pointer mySphereSource = SphereSource::New();

    mySphereSource->SetCenter(center);
    mySphereSource->SetScale(scale);
    mySphereSource->SetResolutionX(resolution[0]);
    mySphereSource->SetResolutionY(resolution[1]);
    mySphereSource->SetSquareness1(1);
    mySphereSource->SetSquareness2(1);
    mySphereSource->Update();
    mySphereSource->GetOutput();

    typename MeshType::Pointer resultMesh = mySphereSource->GetOutput();
    resultMesh->Register();

    return resultMesh;
  }

  //  static typename MeshType::Pointer TranslateMesh(typename MeshType::PointType vec, MeshType* input)
  //  {
  //
  //    typename MeshType::Pointer output = MeshType::New();
  //    {
  //      output->SetPoints(input->GetPoints());
  //      output->SetPointData(input->GetPointData());
  //      output->SetCells(input->GetCells());
  //      output->SetLastCellId( input->GetLastCellId() );
  //      typename MeshType::GeometryMapIterator pointDataIterator = input->GetGeometryData()->Begin();
  //      typename MeshType::GeometryMapIterator pointDataEnd = input->GetGeometryData()->End();
  //
  //      typename MeshType::PointType inputPoint,outputPoint;
  //
  //      while (pointDataIterator != pointDataEnd)
  //      {
  //        unsigned long pointId = pointDataIterator->Index();
  //        itk::SimplexMeshGeometry* newGeometry = new itk::SimplexMeshGeometry();
  //        itk::SimplexMeshGeometry* refGeometry = pointDataIterator->Value();
  //
  //        input->GetPoint(pointId, &inputPoint );
  //        outputPoint[0] = inputPoint[0] + vec[0];
  //        outputPoint[1] = inputPoint[1] + vec[1];
  //        outputPoint[2] = inputPoint[2] + vec[2];
  //        output->SetPoint( pointId, outputPoint );
  //
  //
  //        newGeometry->pos = outputPoint;
  //        newGeometry->neighborIndices = refGeometry->neighborIndices;
  //        newGeometry->meanCurvature = refGeometry->meanCurvature;
  //        newGeometry->neighbors = refGeometry->neighbors;
  //        newGeometry->oldPos = refGeometry->oldPos;
  //        newGeometry->eps = refGeometry->eps;
  //        newGeometry->referenceMetrics = refGeometry->referenceMetrics;
  //        newGeometry->neighborSet = refGeometry->neighborSet;
  //        newGeometry->distance = refGeometry->distance;
  //        newGeometry->externalForce = refGeometry->externalForce;
  //        newGeometry->internalForce = refGeometry->internalForce;
  //        output->SetGeometryData(pointId, newGeometry);
  //        pointDataIterator++;
  //      }
  //    }
  ////    output->SetGeometryData( inputMesh->GetGeometryData() );
  //    return output;
  //  }

  static typename MeshType::Pointer CreateRegularSphereMesh2(typename MeshType::PointType center, typename MeshType::PointType scale, int resolution)
  {
    typedef typename itk::AutomaticTopologyMeshSource<MeshType> MeshSourceType;
    typename MeshSourceType::Pointer mySphereSource = MeshSourceType::New();

    typename MeshType::PointType pnt0, pnt1, pnt2, pnt3, pnt4, pnt5, pnt6, pnt7, pnt8, pnt9, pnt10, pnt11;
    double c1= 0.5 * (1.0 + sqrt(5.0));
    double c2= 1.0;
    double len = sqrt( c1*c1 + c2*c2 );
    c1 /= len;  c2 /= len;

    pnt0[0] = center[0] - c1*scale[0];    pnt0[1] = center[1];                pnt0[2] = center[2] + c2*scale[2];
    pnt1[0] = center[0];                  pnt1[1] = center[1] + c2*scale[1];  pnt1[2] = center[2] - c1*scale[2];
    pnt2[0] = center[0];                  pnt2[1] = center[1] + c2*scale[1];  pnt2[2] = center[2] + c1*scale[2];
    pnt3[0] = center[0] + c1*scale[0];    pnt3[1] = center[1];                pnt3[2] = center[2] - c2*scale[2];
    pnt4[0] = center[0] - c2*scale[0];    pnt4[1] = center[1] - c1*scale[1];  pnt4[2] = center[2];
    pnt5[0] = center[0] - c2*scale[0];    pnt5[1] = center[1] + c1*scale[1];  pnt5[2] = center[2];
    pnt6[0] = center[0];                  pnt6[1] = center[1] - c2*scale[1];  pnt6[2] = center[2] + c1*scale[2];
    pnt7[0] = center[0] + c2*scale[0];    pnt7[1] = center[1] + c1*scale[1];  pnt7[2] = center[2];
    pnt8[0] = center[0];                  pnt8[1] = center[1] - c2*scale[1];  pnt8[2] = center[2] - c1*scale[2];
    pnt9[0] = center[0] + c1*scale[0];    pnt9[1] = center[1];                pnt9[2] = center[2] + c2*scale[2];
    pnt10[0]= center[0] + c2*scale[0];    pnt10[1]= center[1] - c1*scale[1];  pnt10[2]= center[2];
    pnt11[0]= center[0] - c1*scale[0];    pnt11[1]= center[1];                pnt11[2]= center[2] - c2*scale[2];

    addTriangle( mySphereSource, scale, pnt9, pnt2, pnt6, resolution );
    addTriangle( mySphereSource, scale, pnt1, pnt11, pnt5, resolution );
    addTriangle( mySphereSource, scale, pnt11, pnt1, pnt8, resolution );
    addTriangle( mySphereSource, scale, pnt0, pnt11, pnt4, resolution );
    addTriangle( mySphereSource, scale, pnt3, pnt1, pnt7, resolution );
    addTriangle( mySphereSource, scale, pnt3, pnt8, pnt1, resolution );
    addTriangle( mySphereSource, scale, pnt9, pnt3, pnt7, resolution );
    addTriangle( mySphereSource, scale, pnt0, pnt6, pnt2, resolution );
    addTriangle( mySphereSource, scale, pnt4, pnt10, pnt6, resolution );
    addTriangle( mySphereSource, scale, pnt1, pnt5, pnt7, resolution );
    addTriangle( mySphereSource, scale, pnt7, pnt5, pnt2, resolution );
    addTriangle( mySphereSource, scale, pnt8, pnt3, pnt10, resolution );
    addTriangle( mySphereSource, scale, pnt4, pnt11, pnt8, resolution );
    addTriangle( mySphereSource, scale, pnt9, pnt7, pnt2, resolution );
    addTriangle( mySphereSource, scale, pnt10, pnt9, pnt6, resolution );
    addTriangle( mySphereSource, scale, pnt0, pnt5, pnt11, resolution );
    addTriangle( mySphereSource, scale, pnt0, pnt2, pnt5, resolution );
    addTriangle( mySphereSource, scale, pnt8, pnt10, pnt4, resolution );
    addTriangle( mySphereSource, scale, pnt3, pnt9, pnt10, resolution );
    addTriangle( mySphereSource, scale, pnt6, pnt0, pnt4, resolution );

    return mySphereSource->GetOutput();
  }



private:

  static void addTriangle( typename itk::AutomaticTopologyMeshSource<MeshType>::Pointer meshSource, typename MeshType::PointType scale, 
    typename MeshType::PointType pnt0, typename MeshType::PointType pnt1, typename MeshType::PointType pnt2, int resolution )
  {
    if (resolution==0) {
      // add triangle
      meshSource->AddTriangle( meshSource->AddPoint( pnt0 ), 
        meshSource->AddPoint( pnt1 ), 
        meshSource->AddPoint( pnt2 ) );
    }
    else {
      vnl_vector_fixed<typename MeshType::CoordRepType, 3> v1, v2, res, pv;
      v1 = (pnt1-pnt0).Get_vnl_vector();  
      v2 = (pnt2-pnt0).Get_vnl_vector();
      res = vnl_cross_3d( v1, v2 );
      pv = pnt0.GetVectorFromOrigin().Get_vnl_vector();
      //double d = res[0]*pv[0] + res[1]*pv[1] + res[2]*pv[2];

      // subdivision
      typename MeshType::PointType pnt01, pnt12, pnt20;
      for (int d=0; d<3; d++) {
        pnt01[d] = (pnt0[d] + pnt1[d]) / 2.0;
        pnt12[d] = (pnt1[d] + pnt2[d]) / 2.0;
        pnt20[d] = (pnt2[d] + pnt0[d]) / 2.0;
      }
      // map new points to sphere
      double lenPnt01=0;  for (int d=0; d<3; d++) lenPnt01 += pnt01[d]*pnt01[d];  lenPnt01 = sqrt( lenPnt01 );
      double lenPnt12=0;  for (int d=0; d<3; d++) lenPnt12 += pnt12[d]*pnt12[d];  lenPnt12 = sqrt( lenPnt12 );
      double lenPnt20=0;  for (int d=0; d<3; d++) lenPnt20 += pnt20[d]*pnt20[d];  lenPnt20 = sqrt( lenPnt20 );
      for (int d=0; d<3; d++) {
        pnt01[d] *= scale[d]/lenPnt01;
        pnt12[d] *= scale[d]/lenPnt12;
        pnt20[d] *= scale[d]/lenPnt20;
      }
      addTriangle( meshSource, scale, pnt0,  pnt01, pnt20, resolution-1 );
      addTriangle( meshSource, scale, pnt01, pnt1,  pnt12, resolution-1 );
      addTriangle( meshSource, scale, pnt20, pnt12, pnt2,  resolution-1 );
      addTriangle( meshSource, scale, pnt01, pnt12, pnt20, resolution-1 );
    }
  }

};

#endif // MITKMESHUTIL_H_INCLUDED

