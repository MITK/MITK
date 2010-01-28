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


#ifndef MITKMESH_H_HEADER_INCLUDED
#define MITKMESH_H_HEADER_INCLUDED

#include "mitkPointSet.h"
#include "MitkExtExports.h"

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <itkPolygonCell.h>

#if (VTK_MAJOR_VERSION >= 5)
#include <vtkSystemIncludes.h>
#else
#include <vtkIdType.h>
#endif
#include <vtkPoints.h>

namespace mitk {

/**
 * \brief DataStructure which stores a set of points (incl. pointdata) where
 * each point can be associated to an element of a cell.
 *
 * A mesh contains several cells that can be of different celltypes
 * (Line, Triangle, Polygone...). A cell is always closed. If a linestrip is
 * to be created, then declare several cells, each containing one line.
 *
 * The operations take care of the coherence. If a line is added to an
 * existing LineCell, then a TriangleCell is built with the old and the new 
 * parameter (and so on). Deletion is done the opposite way.
 *
 * Example for inserting a line into a TriangleCell:
 * existing PIds ind the cell: 1, 2, 4; 
 * inserting (2, 3) so that new PIds in Cell: 1, 2, 3, 4 
 *
 * The cell is now of type QuadrilateralCell
 *
 * \ingroup Data
 */
class MitkExt_EXPORT Mesh : public PointSet
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

  /** \brief returns the current number of cells in the mesh */
  virtual unsigned long GetNumberOfCells( int t = 0 );

  /** \brief returns the mesh */
  virtual const DataType *GetMesh( int t = 0 ) const;

  /** \brief returns the mesh */
  virtual DataType *GetMesh( int t = 0 );

  void SetMesh( DataType *mesh, int t = 0 );

  /** \brief checks if the given point is in a cell and returns that cellId.
   * Basicaly it searches lines and points that are hit.
   */
  virtual bool EvaluatePosition( Point3D point, unsigned long &cellId, 
    float precision, int t = 0 );

  /** \brief searches for the next new cellId and returns that id */
  unsigned long GetNewCellId( int t = 0 );

  /** \brief returns the first cell that includes the given pointId */
  virtual int SearchFirstCell( unsigned long pointId, int t = 0 );

  /** \brief searches for a line, that is hit by the given point.
   * Then returns the lineId and the cellId
   */
  virtual bool SearchLine( Point3D point, float distance,
    unsigned long &lineId, unsigned long &cellId, int t = 0 );

  /** \brief searches a line according to the cellId and lineId and returns
   * the PointIds, that assign the line; if successful, then return
   * param = true;
   */
  virtual bool GetPointIds( unsigned long cellId, unsigned long lineId, 
    int &idA, int &idB, int t = 0 );

  /** \brief searches a selected cell and returns the id of that cell. If no
   * cell is found, then -1 is returned
   */
  virtual int SearchSelectedCell( int t = 0 );

  /** \brief creates a BoundingBox and computes it with the given points of
   * the cell.
   *
   * Returns the BoundingBox != IsNull() if successful.
   */
  virtual DataType::BoundingBoxPointer GetBoundingBoxFromCell( 
    unsigned long cellId, int t = 0 );

  /** \brief executes the given Operation */
  virtual void ExecuteOperation(Operation* operation);

protected:
  Mesh();
  virtual ~Mesh();

};

} // namespace mitk



#endif /* MITKMESH_H_HEADER_INCLUDED */
