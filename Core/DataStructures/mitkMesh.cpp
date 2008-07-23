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


#include "mitkMesh.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkLineOperation.h"
#include "mitkLineOperation.h"
#include "mitkPointOperation.h"
#include "mitkVector.h"
#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"
#include "mitkLine.h"
#include "mitkRenderingManager.h"


mitk::Mesh::Mesh()
{
}

mitk::Mesh::~Mesh()
{
}


const mitk::Mesh::DataType *mitk::Mesh::GetMesh( int t ) const
{
  return m_PointSetSeries[t];
}


mitk::Mesh::DataType* mitk::Mesh::GetMesh( int t )
{
  return m_PointSetSeries[t];
}


void mitk::Mesh::SetMesh( DataType *mesh, int t )
{
  this->Expand( t+1 );
  m_PointSetSeries[t] = mesh;
}


unsigned long mitk::Mesh::GetNumberOfCells( int t )
{
  return m_PointSetSeries[t]->GetNumberOfCells();
}


//search a line that is close enough according to the given position 
bool mitk::Mesh::SearchLine( Point3D point, float distance, 
  unsigned long &lineId, unsigned long &cellId, int t )
{
  //returns true if a line is found
  ScalarType bestDist = distance;

  //iterate through all cells.
  ConstCellIterator cellIt = m_PointSetSeries[t]->GetCells()->Begin();
  ConstCellIterator cellEnd = m_PointSetSeries[t]->GetCells()->End();
  while( cellIt != cellEnd)
  {
    if (cellIt.Value()->GetNumberOfPoints() >1)
    {
      //then iterate through all indexes of points in it->Value()
      PointIdIterator inAIt = cellIt.Value()->PointIdsBegin(); // first point
      PointIdIterator inBIt = cellIt.Value()->PointIdsBegin(); // second point
      PointIdIterator inEnd = cellIt.Value()->PointIdsEnd();
      
      ++inAIt; //so it points to the point before inBIt

      int currentLineId = 0;
      while(inAIt != inEnd)
      {
        mitk::PointSet::PointType pointA, pointB;
        if ( m_PointSetSeries[t]->GetPoint((*inAIt), &pointA) &&
          m_PointSetSeries[t]->GetPoint((*inBIt), &pointB))
        {
          Line<CoordinateType> *line = new Line<CoordinateType>();
          line->SetPoints(pointA, pointB);
          double thisDistance = line->Distance(point);
          if (thisDistance < bestDist)
          {
            cellId = cellIt->Index();
            lineId = currentLineId;
            bestDist = thisDistance;
          }
        }
        ++inAIt;
        ++inBIt;
        ++currentLineId;
      }

      // If the cell is closed, then check the line from the last index to
      // the first index if inAIt points to inEnd, then inBIt points to the
      // last index.
      CellDataType cellData;
      bool dataOk = m_PointSetSeries[t]->GetCellData(cellIt->Index(), &cellData);
      if (dataOk)
      {
        if (cellData.closed)
        {
          // get the points
          PointIdIterator inAIt = cellIt.Value()->PointIdsBegin();//first point
          // inBIt points to last.
          mitk::PointSet::PointType pointA, pointB;
          if ( m_PointSetSeries[t]->GetPoint((*inAIt), &pointA) &&
            m_PointSetSeries[t]->GetPoint((*inBIt), &pointB))
          {
            Line<CoordinateType> *line = new Line<CoordinateType>();
            line->SetPoints(pointA, pointB);
            double thisDistance = line->Distance(point);
            if (thisDistance < bestDist)
            {
              cellId = cellIt->Index();
              lineId = currentLineId;
              bestDist = thisDistance;
            }
          }
        }
      }
    }
    ++cellIt;
  }
  return (bestDist < distance);
}

int mitk::Mesh::SearchFirstCell( unsigned long pointId, int t )
{
  //iterate through all cells and find the cell the given pointId is inside
  ConstCellIterator it = m_PointSetSeries[t]->GetCells()->Begin();
  ConstCellIterator end = m_PointSetSeries[t]->GetCells()->End();
  while( it != end)
  {
    PointIdIterator position = std::find(it->Value()->PointIdsBegin(), 
      it->Value()->PointIdsEnd(), pointId);

    if ( position != it->Value()->PointIdsEnd())
    {
      return it->Index();
    }
    ++it;
  }
  return -1;
}

// Due to not implemented itk::CellInterface::EvaluatePosition and errors in
// using vtkCell::EvaluatePosition (changing iterator!) we must implement
// it in mitk::Mesh
// make it easy and look for hit points and hit lines: needs to be done anyway!
bool mitk::Mesh::EvaluatePosition( mitk::Point3D point, 
  unsigned long &cellId, float precision, int t )
{
  int pointId = this->SearchPoint( point, precision, t );
  if (pointId > -1)
  {
    //search the cell the point lies inside
    cellId = this->SearchFirstCell( pointId, t );
    return true;
  }
  unsigned long lineId = 0;
  if ( this->SearchLine(point, precision, lineId, cellId, t) )
  {
    return true;
  }

  return false;
}

unsigned long mitk::Mesh::GetNewCellId( int t )
{
  long nextCellId = -1;
  ConstCellIterator it = m_PointSetSeries[t]->GetCells()->Begin();
  ConstCellIterator end = m_PointSetSeries[t]->GetCells()->End();
  
  while (it!=end)
  {
    nextCellId = it.Index();
    ++it;
  }
  ++nextCellId;
  return nextCellId;
}

int mitk::Mesh::SearchSelectedCell( int t )
{
  CellDataIterator cellDataIt, cellDataEnd;
  cellDataEnd = m_PointSetSeries[t]->GetCellData()->End();
  for ( cellDataIt = m_PointSetSeries[t]->GetCellData()->Begin(); 
        cellDataIt != cellDataEnd;
        cellDataIt++ )
  {
    //then declare an operation which unselects this line; UndoOperation as well!
    if ( cellDataIt->Value().selected )
    {
      return cellDataIt->Index();
    }
  }
  return -1;
}

// get the cell; then iterate through the Ids times lineId. Then IdA ist the
// one, IdB ist ne next.don't forget the last closing line if the cell is 
// closed
bool mitk::Mesh::GetPointIds( unsigned long cellId, unsigned long lineId, 
  int &idA, int &idB, int t )
{
  bool ok = false;
  bool found = false;
  CellAutoPointer cellAutoPointer;
  ok = m_PointSetSeries[t]->GetCell(cellId, cellAutoPointer);
  if (ok) 
  {

    CellType * cell = cellAutoPointer.GetPointer();

    //Get the cellData to also check the closing line 
    CellDataType cellData;
    m_PointSetSeries[t]->GetCellData(cellId, &cellData);
    bool closed = cellData.closed;

    PointIdIterator pointIdIt = cell->PointIdsBegin();
    PointIdIterator pointIdEnd = cell->PointIdsEnd();
    unsigned int counter = 0;
    while (pointIdIt != pointIdEnd)
    {
      if(counter == lineId)
      {
        idA = (*pointIdIt);
        ++pointIdIt;
        found = true;
        break;
      }
      ++counter;
      ++pointIdIt;
    }
    if(found)
    {
      //if in the middle
      if (pointIdIt != pointIdEnd)
      {
        idB = (*pointIdIt);
      }
      // if found but on the end, then it is the closing connection, so the
      // last and the first point
      else if (closed)
      {
        pointIdIt = cell->PointIdsBegin();
        idB = (*pointIdIt);
      }
    }
    else
      ok = false;
  }
  return ok;
}


void mitk::Mesh::ExecuteOperation(Operation* operation)
{
  //adding only the operations, that aren't implemented by the pointset.
  switch (operation->GetOperationType())
  {
  case OpNOTHING:
    break;

  case OpNEWCELL:
    {
      mitk::LineOperation *lineOp = 
        dynamic_cast<mitk::LineOperation *>(operation);
      
      // if no lineoperation, then call superclass pointSet
      if (lineOp == NULL)
      {
        Superclass::ExecuteOperation(operation);
      }

      bool ok;
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      
      // if it doesn't already exist
      if (!ok) 
      {
        cellAutoPointer.TakeOwnership( new PolygonType );
        m_PointSetSeries[0]->SetCell(cellId, cellAutoPointer);
        CellDataType cellData;
        cellData.selected = true;
        cellData.selectedLines.clear();
        cellData.closed = false;
        m_PointSetSeries[0]->SetCellData(cellId, cellData);
      }
    }
    break;

  case OpDELETECELL:
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
      {
        Superclass::ExecuteOperation(operation);
      }
      m_PointSetSeries[0]->GetCells()->DeleteIndex((unsigned)lineOp->GetCellId());
      m_PointSetSeries[0]->GetCellData()->DeleteIndex((unsigned)lineOp->GetCellId());
    }
    break;

  case OpCLOSECELL:
    //sets the bolean flag closed from a specified cell to true.
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
      {
        //then search the selected cell!//TODO
        Superclass::ExecuteOperation(operation);
      }
      bool ok;
      int cellId = lineOp->GetCellId();
      if (cellId<0)//cellId isn't set
      {
        cellId = this->SearchSelectedCell( 0 );
        if (cellId < 0 )//still not found
          return;
      }
      CellAutoPointer cellAutoPointer;
      
      //get directly the celldata!TODO
      ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_PointSetSeries[0]->GetCellData(cellId, &cellData);
        cellData.closed = true;
        m_PointSetSeries[0]->SetCellData(cellId, cellData);
      }
    }
    break;

  case OpOPENCELL:
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
      {
        Superclass::ExecuteOperation(operation);
      }
      bool ok;
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_PointSetSeries[0]->GetCellData(cellId, &cellData);
        cellData.closed = false;;
        m_PointSetSeries[0]->SetCellData(cellId, cellData);
      }
    }
    break;

  case OpADDLINE: 
    // inserts the ID of the selected point into the indexes of lines in the
    // selected cell afterwars the added line is selected
    {
      mitk::LineOperation *lineOp = 
        dynamic_cast<mitk::LineOperation *>(operation);

      int cellId = -1;
      int pId = -1;

      if (lineOp == NULL)
      {
        cellId = this->SearchSelectedCell( 0 );
        if (cellId == -1)
          return;

        pId = this->SearchSelectedPoint( 0 );
        if (pId == -1)
          return;
      }
      else
      {
        cellId = lineOp->GetCellId();
        if (cellId == -1)
          return;
        pId = lineOp->GetPIdA();
        if (pId == -1)
          return;
      }

      bool ok;
      CellAutoPointer cellAutoPointer;
      ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellType * cell = cellAutoPointer.GetPointer();
        if( cell->GetType() == CellType::POLYGON_CELL )
        {
          PolygonType * polygon = static_cast<PolygonType *>( cell );
          // add the pointId to the Cell. filling the empty cell with 
          // one id doesn't mean to add a line, it means, that the 
          // initilal PointId is set. The next addition of a pointId adds
          // a line
          polygon->AddPointId(pId);

          // select the line, if we really added a line, so now have more than
          // 1 pointID in the cell
          CellDataType cellData;
          ok = m_PointSetSeries[0]->GetCellData(cellId, &cellData);
          if (ok)
          {
            // A line between point 0 and 1 has the Id 0. A line between
            // 1 and 2 has a Id = 1. So we add getnumberofpoints-2.
            if (polygon->GetNumberOfPoints()>1)
              cellData.selectedLines.push_back(polygon->GetNumberOfPoints()-2);
          }
          m_PointSetSeries[0]->SetCellData(cellId, cellData);
          m_PointSetSeries[0]->SetCell(cellId, cellAutoPointer);
        }
      }
    }
    break;

  case OpDELETELINE:
    {
      // deleted the last line through removing the index PIdA
      // (if set to -1, use the last point) in the given cellId
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      int cellId = -1;
      int pId = -1;

      if (lineOp == NULL)
      {
        cellId = this->SearchSelectedCell( 0 );
        if (cellId == -1)
          return;
        pId = this->SearchSelectedPoint( 0 );
      }
      else
      {
        cellId = lineOp->GetCellId();
        if (cellId == -1)
          return;
        pId = lineOp->GetPIdA();
      }

      bool ok;
      CellAutoPointer cellAutoPointer;
      ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellType * cell = cellAutoPointer.GetPointer();
        if( cell->GetType() == CellType::POLYGON_CELL )
        {
          PolygonType * oldPolygon = static_cast<PolygonType *>( cell );

          PolygonType * newPolygonCell = new PolygonType;
          CellAutoPointer newCell;
          newCell.TakeOwnership( newPolygonCell );

          PointIdConstIterator it, oldend;
          oldend = oldPolygon->PointIdsEnd();
          if(pId >= 0)
          {
            for(it = oldPolygon->PointIdsBegin(); it != oldend; ++it)
            {
              if((*it) != (MeshType::PointIdentifier)pId)
              {
                newPolygonCell->AddPointId(*it);
              }
            }
          }
          else
          {
            --oldend;
            for(it = oldPolygon->PointIdsBegin(); it != oldend; ++it)
              newPolygonCell->AddPointId(*it);
          }
          oldPolygon->SetPointIds(0, newPolygonCell->GetNumberOfPoints(), 
            newPolygonCell->PointIdsBegin());
        }
      }
    }
    break;

  case OpREMOVELINE:
    //Remove the given Index in the given cell through copying everything
    // into a new cell accept the one that has to be deleted.
    {
      mitk::LineOperation *lineOp = 
        dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
      {
        Superclass::ExecuteOperation(operation);
      }

      bool ok;
      CellAutoPointer cellAutoPointer;
      int cellId = lineOp->GetCellId();
      ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (!ok) 
        return;

      CellType * cell = cellAutoPointer.GetPointer();
      CellAutoPointer newCellAutoPointer;
      newCellAutoPointer.TakeOwnership( new PolygonType );
      PolygonType * newPolygon = static_cast<PolygonType *>( cell );

      PointIdIterator it = cell->PointIdsBegin();
      PointIdIterator end = cell->PointIdsEnd();
      int pointId = lineOp->GetPIdA();
      if (pointId<0)//if not initialized!!
        return;

      while (it!=end)
      {
        if ((*it)==(unsigned int)pointId)
        {
          break;
        }
        else
        {
          newPolygon ->AddPointId(*it);
        }
        ++it;
      }
      while (it!=end)
      {
        newPolygon ->AddPointId(*it);
        it++;
      }
      m_PointSetSeries[0]->SetCell(cellId, newCellAutoPointer);
    }
    break;

  case OpINSERTLINE:
    //  //insert line between two other points.
    ////before A--B   after  A--C--B
    //  //the points A, B and C have to be in the pointset.
    //  //needed: CellId, Id of C , Id A and Id B
    ////the cell has to exist!
    //{
    //mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
    //	if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
    // {
    //    Superclass::ExecuteOperation(operation);
    //  }
    //    int cellId = lineOp->GetCellId();
    //    int pIdC = lineOp->GetPIdC();
    //	int pIdA = lineOp->GetPIdA();
    //    int pIdB = lineOp->GetPIdB();

    //    //the points of the given PointIds have to exist in the PointSet
    //    bool ok;
    //    ok = m_PointSetSeries[0]->GetPoints()->IndexExists(pIdA);
    //    if (!ok) 
    //      return;
    //    ok = m_PointSetSeries[0]->GetPoints()->IndexExists(pIdB);
    //    if (!ok) 
    //      return;
    //	ok = m_PointSetSeries[0]->GetPoints()->IndexExists(pIdC);
    //    if (!ok) 
    //      return;

    //    // so the points do exist. So now check, if there is already a cell
    //    // with the given Id
    //    DataType::CellAutoPointer cell;
    //    ok = m_PointSetSeries[0]->GetCell(cellId, cell);
    //	if (!ok)
    //		return;

    //	//pIdA and pIdB should exist in the cell
    //    
    //	PointIdIterator pit = cell->PointIdsBegin();
    //    PointIdIterator end = cell->PointIdsEnd();
    //    
    //	//now arrange the new Ids in the cell like desired; pIdC between 
    //  // pIdA and pIdB
    //  unsigned int nuPoints = cell->GetNumberOfPoints();

    //	std::vector<unsigned int> newPoints;
    //	pit = cell->PointIdsBegin();
    //    end = cell->PointIdsEnd();
    //    int i = 0;
    //    while( pit != end )
    //    {
    //      if ((*pit) = pIdA)
    //      {
    //        //now we have found the place to add pIdC after
    //        newPoints[i] = (*pit);
    //        i++;
    //        newPoints[i] = pIdC;
    //      }
    //      else
    //        newPoints[i] = (*pit);
    //      pit++;
    //    }

    //	//now we have the Ids, that existed before combined with the new ones
    //  //so delete the old cell
    //  //doesn't seem to be necessary!
    //	//cell->ClearPoints();
    //	pit = cell->PointIdsBegin();
    //	cell->SetPointIds(pit);
    //}
    break;

  case OpMOVELINE://(moves two points)
    {
      mitk::LineOperation *lineOp = 
        dynamic_cast<mitk::LineOperation *>(operation);

      if (lineOp == NULL)
      {
        mitk::StatusBar::GetInstance()->DisplayText(
          "Message from mitkMesh: Recieved wrong type of operation! See mitkMeshInteractor.cpp", 10000);
        return;
      }

      //create two operations out of the one operation and call superclass
      //through the transmitted pointIds get the koordinates of the points.
      //then add the transmitted vestor to them
      //create two operations and send them to superclass
      Point3D pointA, pointB;
      pointA.Fill(0.0);
      pointB.Fill(0.0);
      m_PointSetSeries[0]->GetPoint(lineOp->GetPIdA(), &pointA);
      m_PointSetSeries[0]->GetPoint(lineOp->GetPIdB(), &pointB);

      pointA[0] += lineOp->GetVector()[0];
      pointA[1] += lineOp->GetVector()[1];
      pointA[2] += lineOp->GetVector()[2];
      pointB[0] += lineOp->GetVector()[0];
      pointB[1] += lineOp->GetVector()[1];
      pointB[2] += lineOp->GetVector()[2];

      mitk::PointOperation* operationA = 
        new mitk::PointOperation(OpMOVE, pointA, lineOp->GetPIdA());
      mitk::PointOperation* operationB = 
        new mitk::PointOperation(OpMOVE, pointB, lineOp->GetPIdB());

      Superclass::ExecuteOperation(operationA);
      Superclass::ExecuteOperation(operationB);
    }
    break;

  case OpSELECTLINE://(select the given line)
    {
      mitk::LineOperation *lineOp = 
        dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
      {
        Superclass::ExecuteOperation(operation);
      }
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      bool ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_PointSetSeries[0]->GetCellData(cellId, &cellData);
        SelectedLinesType *selectedLines = &(cellData.selectedLines);
        SelectedLinesIter position = std::find(selectedLines->begin(), 
          selectedLines->end(), (unsigned int) lineOp->GetId());

        if (position == selectedLines->end())//if not alsready selected
        {
          cellData.selectedLines.push_back(lineOp->GetId());
        }
        m_PointSetSeries[0]->SetCellData(lineOp->GetCellId(), cellData);
      }
    }
    break;

  case OpDESELECTLINE://(deselect the given line)
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL) 
      {
        Superclass::ExecuteOperation(operation);
      }
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      bool ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_PointSetSeries[0]->GetCellData(cellId, &cellData);
        SelectedLinesType *selectedLines = &(cellData.selectedLines);
        SelectedLinesIter position = std::find(selectedLines->begin(),
          selectedLines->end(), (unsigned int) lineOp->GetId());

        if (position != selectedLines->end())//if found
        {
          selectedLines->erase(position);
        }
        m_PointSetSeries[0]->SetCellData(cellId, cellData);
      }
    }
    break;

  case OpSELECTCELL://(select the given cell)
    {
      mitk::LineOperation *lineOp = 
        dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
      {
        Superclass::ExecuteOperation(operation);
      }

      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      
      //directly get the data!//TODO
      bool ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_PointSetSeries[0]->GetCellData(cellId, &cellData);
        cellData.selected = true;
        m_PointSetSeries[0]->SetCellData(cellId, cellData);
      }
    }
    break;

  case OpDESELECTCELL://(deselect the given cell)
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
      {
        Superclass::ExecuteOperation(operation);
      }
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      bool ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_PointSetSeries[0]->GetCellData(cellId, &cellData);
        cellData.selected = false;
        m_PointSetSeries[0]->SetCellData(cellId, cellData);
      }
    }
    break;

  case OpMOVECELL:
    //moves all Points of one cell according to the given vector
    {
      mitk::CellOperation *lineOp = dynamic_cast<mitk::CellOperation *>(operation);
      if (lineOp == NULL)//if no celloperation, then call superclass pointSet
      {
        Superclass::ExecuteOperation(operation);
      }

      int cellId = lineOp->GetCellId();
      Vector3D vector = lineOp->GetVector();

      //get the cell
      CellAutoPointer cellAutoPointer;
      bool ok = m_PointSetSeries[0]->GetCell(cellId, cellAutoPointer);
      if (!ok)
        return;

      CellDataType cellData;
      m_PointSetSeries[0]->GetCellData(cellId, &cellData);
      // iterate through the pointIds of the CellData and move those points in 
      // the pointset
      PointIdIterator it = cellAutoPointer->PointIdsBegin();
      PointIdIterator end = cellAutoPointer->PointIdsEnd();
      while(it != end)
      {
        unsigned int position = (*it);
        PointType point;
        m_PointSetSeries[0]->GetPoint(position, &point);
        point = point + vector;
        m_PointSetSeries[0]->SetPoint(position, point);
        ++it;
      }

    }
    break;

  default:
    //if the operation couldn't be handled here, then send it to superclass
    Superclass::ExecuteOperation(operation);
    return;
  }
  //to tell the mappers, that the data is modifierd and has to be updated
  this->Modified();

  mitk::OperationEndEvent endevent(operation);
  ((const itk::Object*)this)->InvokeEvent(endevent);

  // As discussed lately, don't mess with rendering from inside data structures
  //*todo has to be done here, cause of update-pipeline not working yet
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

mitk::Mesh::DataType::BoundingBoxPointer 
mitk::Mesh::GetBoundingBoxFromCell( unsigned long cellId, int t )
{
  // itk::CellInterface has also a GetBoundingBox, but it
  // returns CoordRepType [PointDimension *2]
  DataType::BoundingBoxPointer bBoxPointer = NULL;
  CellAutoPointer cellAutoPointer;
  if ( m_PointSetSeries[t]->GetCell(cellId, cellAutoPointer))
  {
    DataType::PointsContainerPointer pointsContainer = DataType::PointsContainer::New();
    PointIdIterator bbIt = cellAutoPointer.GetPointer()->PointIdsBegin();
    PointIdIterator bbEnd = cellAutoPointer.GetPointer()->PointIdsEnd();
    while(bbIt != bbEnd)
    {
      mitk::PointSet::PointType point;
      bool pointOk = m_PointSetSeries[t]->GetPoint((*bbIt), &point);
      if (pointOk)
        pointsContainer->SetElement((*bbIt), point);
      ++bbIt;
    }
    bBoxPointer = DataType::BoundingBoxType::New();
    bBoxPointer->SetPoints(pointsContainer);
    bBoxPointer->ComputeBoundingBox();
  }
  return bBoxPointer;
}
