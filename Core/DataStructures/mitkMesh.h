/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKMESH_H_HEADER_INCLUDED
#define MITKMESH_H_HEADER_INCLUDED

#include "mitkPointSet.h"


#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkIdType.h>
#include <vtkPoints.h>

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

  typedef Superclass::DataType::CellType CellType;
  typedef CellType::CellAutoPointer CellAutoPointer;
  typedef Superclass::MeshTraits::CellTraits CellTraits;
  typedef CellTraits::PointIdConstIterator PointIdConstIterator;
  typedef CellTraits::PointIdIterator PointIdIterator;
  typedef DataType::CellDataContainer CellDataContainer;
  typedef DataType::CellDataContainerIterator CellDataIterator;
  typedef Superclass::DataType::CellsContainer::Iterator CellIterator;
  typedef Superclass::DataType::CellsContainer::ConstIterator ConstCellIterator;
  typedef itk::PolygonCell< CellType > PolygonType;
  typedef MeshType::CellType::MultiVisitor MeshMultiVisitor;    

  //##Documentation
	//## @brief returns the current number of cells in the mesh
	virtual unsigned long GetNumberOfCells();

  //##Documentation
	//## @brief returns the mesh 
  virtual DataType::Pointer GetMesh() const;

  //##Documentation
	//## @brief checks if the given point is in a cell and returns that cellId.
  //## Basicaly it searches lines and points that are hit.
  virtual bool EvaluatePosition(Point3D point, unsigned long &cellId, float precision);

  //##Documentation
	//## @brief searches for the next new cellId and returns that id
  unsigned long GetNewCellId();

  //##Documentation
	//## @brief returns the first cell that includes the given pointId
  virtual int SearchFirstCell(unsigned long pointId);

  //##Documentation
	//## @brief searches for a line, that is hit by the given point.
  //## Then returns the lineId and the cellId
  virtual bool SearchLine(Point3D point, float distance , unsigned long &lineId, unsigned long &cellId);

  //##Documentation
	//## @brief searches a line according to the cellId and lineId and 
  //## returns the PointIds, that assign the line; if successful, then return param = true;
  virtual bool GetPointIds(unsigned long cellId, unsigned long lineId, int &idA, int &idB);

  //##Documentation
  //## @brief searches a selected cell and returns the id of that cell. if no cell is found, then -1 is returned
  virtual int SearchSelectedCell();

  //##Documentation
	//## @brief creates a BoundingBox and computes it with the given points of the cell
  //## Returns the BoundingBox != IsNull() if successful.
  virtual DataType::BoundingBoxPointer GetBoundingBoxFromCell(unsigned long cellId);

  //##Documentation
  //## @brief executes the given Operation
	virtual void ExecuteOperation(Operation* operation);

protected:
	Mesh();
	virtual ~Mesh();

};

} // namespace mitk



#endif /* MITKMESH_H_HEADER_INCLUDED */
