#include "mitkMesh.h"
#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkLineOperation.h>
#include <mitkLineOperation.h>
#include <mitkPointOperation.h>
#include <mitkVector.h>
#include <mitkStatusBar.h>
#include <mitkInteractionConst.h>
#include <mitkLine.h>


mitk::Mesh::Mesh()
{
  m_ItkData = DataType::New();
  PointDataContainer::Pointer pointData = PointDataContainer::New();
  m_ItkData->SetPointData(pointData);

  m_Geometry3D->Initialize();
}

mitk::Mesh::~Mesh()
{}

mitk::Mesh::DataType::Pointer mitk::Mesh::GetMesh() const
{
  return m_ItkData;
}


unsigned long mitk::Mesh::GetNumberOfCells()
{
  return m_ItkData->GetNumberOfCells();
}

//search a line that is close enough according to the given position 
bool mitk::Mesh::SearchLine(Point3D point, float distance , unsigned long &lineId, unsigned long &cellId)
{
//
//
//returns true if a line is found

  //iterate through all cells.
  ConstCellIterator cellIt = m_ItkData->GetCells()->Begin();
  ConstCellIterator cellEnd = m_ItkData->GetCells()->End();
  while( cellIt != cellEnd)
  {
    if (cellIt.Value()->GetNumberOfPoints() >1)
    {
      //then iterate through all indexes of points in it->Value()
      PointIdIterator inAIt = cellIt.Value()->PointIdsBegin();//for the first point
      PointIdIterator inBIt = cellIt.Value()->PointIdsBegin();//for the second point
      PointIdIterator inEnd = cellIt.Value()->PointIdsEnd();
      ++inAIt;//so it points to the point before inBIt
      int currentLineId = 0;
      while(inAIt != inEnd)
      {
        mitk::PointSet::PointType pointA, pointB;
        if ( m_ItkData->GetPoint((*inAIt), &pointA) &&
              m_ItkData->GetPoint((*inBIt), &pointB))
        {
          Line<CoordinateType> *line = new Line<CoordinateType>();
          line->SetPoints(pointA, pointB);
          double thisDistance = line->distance(point);
          if (thisDistance < distance)
          {
            cellId = cellIt->Index();
            lineId = currentLineId;
            return true;
          }
        }
        ++inAIt;
        ++inBIt;
        ++currentLineId;
      }

      //if the cell is closed, then check the line from the last index to the first index
      //if inAIt points to inEnd, then inBIt points to the last index.
      CellDataType cellData;
      bool dataOk = m_ItkData->GetCellData(cellIt->Index(), &cellData);
      if (dataOk)
      {
        if (cellData.closed)
        {
          //get the points
          PointIdIterator inAIt = cellIt.Value()->PointIdsBegin();//for the first point
          //inBIt points to last.
          mitk::PointSet::PointType pointA, pointB;
          if ( m_ItkData->GetPoint((*inAIt), &pointA) &&
                m_ItkData->GetPoint((*inBIt), &pointB))
          {
            Line<CoordinateType> *line = new Line<CoordinateType>();
            line->SetPoints(pointA, pointB);
            double thisDistance = line->distance(point);
            if (thisDistance < distance)
            {
              cellId = cellIt->Index();
              lineId = currentLineId;
              return true;
            }
          }
        }
      }
    }
    ++cellIt;
  }
	return false;
}

int mitk::Mesh::SearchFirstCell(unsigned long pointId)
//iterate through all cells and find the cell the given pointId is inside
{
  ConstCellIterator it = m_ItkData->GetCells()->Begin();
  ConstCellIterator end = m_ItkData->GetCells()->End();
  while( it != end)
  {
    PointIdIterator position = std::find(it->Value()->PointIdsBegin(), it->Value()->PointIdsEnd(), pointId);
    if ( position != it->Value()->PointIdsEnd())
    {
      return it->Index();
    }
    ++it;
  }
  return -1;
}

//due to not implemented itk::CellInterface::EvaluatePosition and errors in using vtkCell::EvaluatePosition (changing iterator!) we must implement it in mitk::Mesh
//make it easy and look for hit points and hit lines: needs to be done anyway!
bool mitk::Mesh::EvaluatePosition(mitk::Point3D point, unsigned long &cellId, float precision)
{
  int pointId = this->SearchPoint(point, precision);
  if (pointId > -1)
  {
    //search the cell the point lies inside
    cellId = this->SearchFirstCell(pointId);
    return true;
  }
  unsigned long lineId = 0;
  if( this->SearchLine(point, precision, lineId, cellId))
  {
    return true;
  }
  
  //-------------------------first version of evaluate Position, with use of vtkPolygon::EvaluatePosition()
//  //iterate through all cells
//  ConstCellIterator cellIter = m_ItkData->GetCells()->Begin();
//  ConstCellIterator end = m_ItkData->GetCells()->End();
//  
//  while(cellIter != end)
//  {
//    if( cellIter->Value()->GetType() == CellType::POLYGON_CELL )
//    {
//      PolygonType *polygon = static_cast<PolygonType *>( cellIter->Value() );
//
//      //convert all point coordinates of itkPolygon to vtkPolygon
//      vtkPolygon *vtkPoly = vtkPolygon::New();
//      unsigned long pnts = polygon->GetNumberOfVertices();
//      
//      std::vector<vtkIdType> vtkids;
//      PointIdConstIterator idIt = polygon->PointIdsBegin();
//      PointIdConstIterator idEnd = polygon->PointIdsEnd();
//      while(idIt != idEnd)
//      {
//        vtkids.push_back((vtkIdType)(*idIt));
//        ++idIt;
//      }
//      
//      PointsIterator coordIt = m_ItkData->GetPoints()->Begin();
//      PointsIterator coordEnd = m_ItkData->GetPoints()->End();
//      vtkPoints *vtkpoints = vtkPoints::New();
//
//      while (coordIt != coordEnd)
//      {
//        vtkpoints->InsertNextPoint(coordIt.Value()[0], coordIt.Value()[1], coordIt.Value()[2]);
//        std::cout << coordIt.Value()[0] << ", " << coordIt.Value()[1] << ", " << coordIt.Value()[2] << std::endl;
//        ++coordIt;
//      }
//      vtkPoly->Initialize(pnts, &vtkids[0], vtkpoints);
//
//      float mypoint[3];
//      mypoint[0] = point.x;
//      mypoint[1] = point.y;
//      mypoint[2] = point.z;
//      float closestPoint = 0;
//      int subId=0;
//      float pcoords[3]={0, 0, 0};
//      float dist2=0;
//      float weights=0;
//
//      //doesn't work! it changes the iterator cellIter!!!
//      //int hit = vtkPoly->EvaluatePosition( mypoint, &closestPoint, subId, pcoords, dist2, &weights);
//      
//      if (hit)
//      {
//        unsigned int cellidx = cellIter->Index();
//        cellId = cellIter->Index();
//        return true;
//      }
//      /*vtkpoints->Delete();
//      vtkPoly->Delete();*/
//    }    
//    cellIter++;
  //}
  return false;
}

unsigned long mitk::Mesh::GetNewCellId()
{
  long nextCellId = -1;
  ConstCellIterator it = m_ItkData->GetCells()->Begin();
  ConstCellIterator end = m_ItkData->GetCells()->End();
  while (it!=end)
  {
    nextCellId = it.Index();
    ++it;
  }
  ++nextCellId;
  return nextCellId;
}

int mitk::Mesh::SearchSelectedCell()
{
  CellDataIterator cellDataIt, cellDataEnd;
  cellDataEnd = m_ItkData->GetCellData()->End();
  for (cellDataIt = m_ItkData->GetCellData()->Begin(); cellDataIt != cellDataEnd; cellDataIt++)
	{
    if ( cellDataIt->Value().selected )//then declare an operation which unselects this line; UndoOperation as well!
		{
      return cellDataIt->Index();
		}
	}
  return -1;
}

bool mitk::Mesh::GetPointIds(unsigned long cellId, unsigned long lineId, int &idA, int &idB)
//get the cell;
//then iterate through the Ids times lineId. Then IdA ist the one, IdB ist ne next.
//don't forget the last closing line if the cell is closed
{
  bool ok = false;
  bool found = false;
  CellAutoPointer cellAutoPointer;
  ok = m_ItkData->GetCell(cellId, cellAutoPointer);
  if (ok) 
  {

    CellType * cell = cellAutoPointer.GetPointer();

    //Get the cellData to also check the closing line 
    CellDataType cellData;
    m_ItkData->GetCellData(cellId, &cellData);
    bool closed = cellData.closed;

    PointIdIterator pointIdIt = cell->PointIdsBegin();
    PointIdIterator pointIdEnd = cell->PointIdsEnd();
    int counter = 0;
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
      if (pointIdIt != pointIdEnd)//if in the middle
      {
        idB = (*pointIdIt);
      }
      else if (closed)//if found but on the end, then it is the closing connection, so the last and the first point
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

//##Documentation
//## @brief executes the given Operation
void mitk::Mesh::ExecuteOperation(Operation* operation)
//adding only the operations, that aren't implemented by the pointset.
{

	switch (operation->GetOperationType())
	{
	case OpNOTHING:
		break;
  case OpNEWCELL:
		{
     	mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
    	if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
	    {
        Superclass::ExecuteOperation(operation);
      }

      bool ok;
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      ok = m_ItkData->GetCell(cellId, cellAutoPointer);
	    //if it doesn't already exist
      if (!ok) 
      {
        cellAutoPointer.TakeOwnership( new PolygonType );
        m_ItkData->SetCell(cellId, cellAutoPointer);
        CellDataType cellData;
        cellData.selected = true;
        cellData.selectedLines.clear();
        cellData.closed = false;
        m_ItkData->SetCellData(cellId, cellData);
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
      m_ItkData->GetCells()->DeleteIndex((unsigned)lineOp->GetCellId());
      m_ItkData->GetCellData()->DeleteIndex((unsigned)lineOp->GetCellId());
    }
    break;
  case OpCLOSECELL:
    //sets the bolean flag closed from a specified cell to true.
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
    	if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
	    {
        Superclass::ExecuteOperation(operation);//then search the selected cell!//TODO
      }
      bool ok;
      int cellId = lineOp->GetCellId();
      if (cellId<0)//cellId isn't set
      {
        cellId = this->SearchSelectedCell();
        if (cellId < 0 )//still not found
          return;
      }
      CellAutoPointer cellAutoPointer;
      ok = m_ItkData->GetCell(cellId, cellAutoPointer);//get directly the celldata!TODO
      if (ok) 
      {
        CellDataType cellData;
        m_ItkData->GetCellData(cellId, &cellData);
        cellData.closed = true;
        m_ItkData->SetCellData(cellId, cellData);
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
      ok = m_ItkData->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_ItkData->GetCellData(cellId, &cellData);
        cellData.closed = false;;
        m_ItkData->SetCellData(cellId, cellData);
      }
    }
    break;
	case OpADDLINE: 
    //deselects all lines and 
    //inserts the ID of the selected point into the indexes of lines in the selected cell
    //afterwars the added line is selected
		{
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      int cellId = -1;
      int pId = -1;

    	if (lineOp == NULL)
	    {
        cellId = this->SearchSelectedCell();
        if (cellId == -1)
          return;

        pId = this->SearchSelectedPoint();
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
      ok = m_ItkData->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellType * cell = cellAutoPointer.GetPointer();
        if( cell->GetType() == CellType::POLYGON_CELL )
        {
          PolygonType * polygon = static_cast<PolygonType *>( cell );

          //deselect all other lines
          CellDataType cellData;
          ok = m_ItkData->GetCellData(cellId, &cellData);
          if (ok)
          {
            //set the new line selected
            //if we are to add pId = 0, then we don't select it, cause adding pId 0 to the 
            //index of a cell only 'initiates' the line-drawing.
            //A line between point 0 and 1 has the Id 0. A line between 1 and 2 has a Id = 1.
            //So we add pId to the cell and for selection add pId - 1.
            if (pId > 0)
              cellData.selectedLines.push_back(pId-1);
          }
          m_ItkData->SetCellData(cellId, cellData);

          //even if it is the pointId = 0, then we have our start.
          polygon->AddPointId(pId);
          m_ItkData->SetCell(cellId, cellAutoPointer);
        }
      }
    }
		break;
  case OpDELETELINE:
    //deleted the last line through removing the last index in the given cellId
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      int cellId = -1;
      int pId = -1;

    	if (lineOp == NULL)
	    {
        cellId = this->SearchSelectedCell();
        if (cellId == -1)
          return;
        pId = this->SearchSelectedPoint();
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
      ok = m_ItkData->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellType * cell = cellAutoPointer.GetPointer();
        if( cell->GetType() == CellType::POLYGON_CELL )
        {
          PolygonType * polygon = static_cast<PolygonType *>( cell );
          CellAutoPointer oldCell;
  //        polygon->MakeCopy(oldCell);
  //due to bug in itk::PolygonCell::MakeCopy
  PolygonType * newPolygonCell = new PolygonType;
  oldCell.TakeOwnership( newPolygonCell );
  unsigned long numberOfPoints = polygon->GetNumberOfPoints();
  newPolygonCell->SetPointIds(0, numberOfPoints, polygon->GetPointIds());

//          unsigned long numberOfPoints = oldCell->GetNumberOfPoints();
          PointIdConstIterator oldIter;
          oldIter = oldCell.GetPointer()->PointIdsBegin();
          //decrease the number so which is equal to deleting the last index
          --numberOfPoints;
          int dummy = 0;//dummy for for itkPolygonCell->SetPointIds(...)
          polygon->SetPointIds( dummy, numberOfPoints, oldIter );
        }
      }
    }
    break;
  case OpREMOVELINE:
      //Remove the given Index in the given cell
      //through copying everything into a new cell accept the one that has to be deleted.
    {
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
    	if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
	    {
        Superclass::ExecuteOperation(operation);
      }

      bool ok;
      CellAutoPointer cellAutoPointer;
      int cellId = lineOp->GetCellId();
      ok = m_ItkData->GetCell(cellId, cellAutoPointer);
	    if (!ok) 
        return;
      
      CellType * cell = cellAutoPointer.GetPointer();
      CellAutoPointer newCellAutoPointer;
      newCellAutoPointer.TakeOwnership( new PolygonType );
      CellType * newCell = newCellAutoPointer.GetPointer();
      PolygonType * newPolygon = static_cast<PolygonType *>( cell );
      
      PointIdIterator it = cell->PointIdsBegin();
      PointIdIterator end = cell->PointIdsEnd();
      int pointId = lineOp->GetPIdA();
      while (it!=end)
      {
        if ((*it)==pointId)
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
      m_ItkData->SetCell(cellId, newCellAutoPointer);
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
  //    ok = m_ItkData->GetPoints()->IndexExists(pIdA);
  //    if (!ok) 
  //      return;
  //    ok = m_ItkData->GetPoints()->IndexExists(pIdB);
  //    if (!ok) 
  //      return;
		//	ok = m_ItkData->GetPoints()->IndexExists(pIdC);
  //    if (!ok) 
  //      return;

  //    //so the points do exist. So now check, if there is already a cell with the given Id
  //    DataType::CellAutoPointer cell;
  //    ok = m_ItkData->GetCell(cellId, cell);
		//	if (!ok)
		//		return;

		//	//pIdA and pIdB should exist in the cell
  //    
		//	PointIdIterator pit = cell->PointIdsBegin();
  //    PointIdIterator end = cell->PointIdsEnd();
  //    
		//	//now arrange the new Ids in the cell like desired; pIdC between pIdA and pIdB
  //    unsigned int nuPoints = cell->GetNumberOfPoints();

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

		//	//now we have the Ids, that existed before combined with the new ones so delete the old cell
  //    //doesn't seem to be necessary!
		//	//cell->ClearPoints();
		//	pit = cell->PointIdsBegin();
		//	cell->SetPointIds(pit);
		//}
		break;
	case OpMOVELINE://(moves two points)
		{
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
      if (lineOp == NULL)
      {
        mitk::StatusBar::DisplayText("Message from mitkMesh: Recieved wrong type of operation!See mitkMeshInteractor.cpp", 10000);
          return;
      }

      //create two operations out of the one operation and call superclass
      //through the transmitted pointIds get the koordinates of the points.
      //then add the transmitted vestor to them
      //create two operations and send them to superclass
      Point3D pointA, pointB;
			m_ItkData->GetPoint(lineOp->GetPIdA(), &pointA);
		  m_ItkData->GetPoint(lineOp->GetPIdB(), &pointB);

      pointA[0] += lineOp->GetVector()[0];
      pointA[1] += lineOp->GetVector()[1];
      pointA[2] += lineOp->GetVector()[2];
      pointB[0] += lineOp->GetVector()[0];
      pointB[1] += lineOp->GetVector()[1];
      pointB[2] += lineOp->GetVector()[2];

      mitk::PointOperation* operationA = new mitk::PointOperation(OpMOVE, pointA, lineOp->GetPIdA());
      mitk::PointOperation* operationB = new mitk::PointOperation(OpMOVE, pointB, lineOp->GetPIdB());

      Superclass::ExecuteOperation(operationA);
      Superclass::ExecuteOperation(operationB);
		}
		break;
	case OpSELECTLINE://(select the given line)
		{
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
    	if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
	    {
        Superclass::ExecuteOperation(operation);
      }
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      bool ok = m_ItkData->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_ItkData->GetCellData(cellId, &cellData);
        SelectedLinesType *selectedLines = &(cellData.selectedLines);
        SelectedLinesIter position = std::find(selectedLines->begin(), selectedLines->end(), lineOp->GetId());
			  if (position == selectedLines->end())//if not alsready selected
        {
          cellData.selectedLines.push_back(lineOp->GetId());
        }
        m_ItkData->SetCellData(lineOp->GetCellId(), cellData);
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
      bool ok = m_ItkData->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_ItkData->GetCellData(cellId, &cellData);
        SelectedLinesType *selectedLines = &(cellData.selectedLines);
        SelectedLinesIter position = std::find(selectedLines->begin(), selectedLines->end(), lineOp->GetId());
			  if (position != selectedLines->end())//if found
        {
          selectedLines->erase(position);
        }
        m_ItkData->SetCellData(cellId, cellData);
      }
		}
		break;
	case OpSELECTCELL://(select the given cell)
		{
      mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
    	if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
	    {
        Superclass::ExecuteOperation(operation);
      }
      int cellId = lineOp->GetCellId();
      CellAutoPointer cellAutoPointer;
      bool ok = m_ItkData->GetCell(cellId, cellAutoPointer);//directly get the data!//TODO
      if (ok) 
      {
        CellDataType cellData;
        m_ItkData->GetCellData(cellId, &cellData);
        cellData.selected = true;
        m_ItkData->SetCellData(cellId, cellData);
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
      bool ok = m_ItkData->GetCell(cellId, cellAutoPointer);
      if (ok) 
      {
        CellDataType cellData;
        m_ItkData->GetCellData(cellId, &cellData);
        cellData.selected = false;
        m_ItkData->SetCellData(cellId, cellData);
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
      bool ok = m_ItkData->GetCell(cellId, cellAutoPointer);
      if (!ok)
        return;

      CellDataType cellData;
      m_ItkData->GetCellData(cellId, &cellData);
      //iterate through the pointIds of the CellData and move those points in the pointset
      PointIdIterator it = cellAutoPointer->PointIdsBegin();
      PointIdIterator end = cellAutoPointer->PointIdsEnd();
      while(it != end)
      {
        unsigned int position = (*it);
        PointType point;
        m_ItkData->GetPoint(position, &point);
        point = point + vector;
        m_ItkData->SetPoint(position, point);
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

  ((const itk::Object*)this)->InvokeEvent(itk::EndEvent());

}

mitk::Mesh::DataType::BoundingBoxPointer mitk::Mesh::GetBoundingBoxFromCell(unsigned long cellId)
//itk::CellInterface has also a GetBoundingBox, but it returns CoordRepType [PointDimension *2]
{
  DataType::BoundingBoxPointer bBoxPointer = NULL;
  CellAutoPointer cellAutoPointer;
  if ( m_ItkData->GetCell(cellId, cellAutoPointer))
  {
    DataType::PointsContainerPointer pointsContainer = DataType::PointsContainer::New();
    PointIdIterator bbIt = cellAutoPointer.GetPointer()->PointIdsBegin();
    PointIdIterator bbEnd = cellAutoPointer.GetPointer()->PointIdsEnd();
    while(bbIt != bbEnd)
    {
      mitk::PointSet::PointType point;
      bool pointOk = m_ItkData->GetPoint((*bbIt), &point);
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

