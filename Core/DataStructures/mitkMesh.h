#ifndef MITKMESH_H_HEADER_INCLUDED
#define MITKMESH_H_HEADER_INCLUDED

#include "mitkPointSet.h"
#include <mitkCommon.h>
#include <itkDefaultDynamicMeshTraits.h>
#include "itkMesh.h"
#include "itkVertexCell.h"
#include "itkLineCell.h"
#include "itkTriangleCell.h"
#include "itkTetrahedronCell.h"
#include "itkPolygonCell.h"
  
//const unsigned int Dimension = 3;

namespace mitk {

//##Documentation
//##@brief DataStructure which stores a list of Points with data and cells
//## @ingroup Data
//##
//## A mesh contains several cells that can be of different celltypes (Line, Triangle, Polygone...).
//## A cell is always closed. If a linestrip is to be created, then declare several cells, each containing one line.
//## The operations take care about the coherence. If a line is added to an existing LineCell, 
//## then a TriangleCell is built with the old and the new parameter (and so on).
//## Deletion is done the opposite way.
//## Example for inserting a line into a TriangleCell:
//## existing PIds ind the cell:1,2,4; 
//## inserting 2,3 so that new PIds in Cell:1,2,3,4 
//## The cell is now of type QuadrilateralCell
class Mesh : public PointSet
{
public:
  mitkClassMacro(Mesh, PointSet);

  itkNewMacro(Self);

  typedef mitk::ScalarType PixelType;
  typedef itk::DefaultDynamicMeshTraits<bool, 3, 3, mitk::ScalarType> MeshTraits;
  typedef itk::Mesh<PixelType, Dimension, MeshTraits> MeshType;  
  typedef MeshType DataType;

  typedef MeshType::CellType CellType;
  typedef CellType::CellAutoPointer CellAutoPointer;
  typedef MeshType::CellsContainer::Pointer CellContainerPointer;
  typedef MeshType::CellDataContainer::Iterator CellDataContainerIterator;
  typedef MeshType::CellsContainer::Iterator CellIterator;
  typedef MeshTraits::CellTraits CellTraits;
  typedef CellTraits::PointIdIterator PointIdIterator;

  typedef itk::VertexCell<CellType> VertexType;
  typedef itk::LineCell<CellType> LineType;
  typedef itk::TriangleCell<CellType> TriangleType;
    typedef itk::PolygonCell<CellType> PolygonType;
  
  //##Documentation
	//## @brief returns the current number of cells in the mesh
	virtual unsigned long GetNumberOfCells();

  //##Documentation
	//## @brief returns the mesh 
  const DataType::Pointer GetMesh() const;

  //##Documentation
  //## @brief executes the given Operation
	virtual void ExecuteOperation(Operation* operation);

protected:
	Mesh();
	virtual ~Mesh();
};

} // namespace mitk



#endif /* MITKMESH_H_HEADER_INCLUDED */
