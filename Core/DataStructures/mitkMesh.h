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
//## @ingroup Geometry
//##
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
  typedef itk::VertexCell<CellType> VertexType;
  typedef itk::LineCell<CellType> LineType;
  typedef itk::TriangleCell<CellType> TriangleType;
  typedef itk::TetrahedronCell<CellType> TetrahedronType;
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
