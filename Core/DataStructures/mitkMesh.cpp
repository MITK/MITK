#include "mitkMesh.h"

#include <mitkOperation.h>
#include <mitkOperationActor.h>

#include <mitkLineOperation.h>
#include <mitkLineVecOperation.h>
#include <mitkPointOperation.h>
#include "mitkVector.h"

#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"

mitk::Mesh::Mesh()
{
  m_ItkData = DataType::New();
  PointDataContainer::Pointer pointData = PointDataContainer::New();
  m_ItkData->SetPointData(pointData);
  m_Geometry3D->Initialize();
}

mitk::Mesh::~Mesh()
{}

unsigned long mitk::Mesh::GetNumberOfCells()
{
  return m_ItkData->GetNumberOfCells();
}

const mitk::Mesh::DataType::Pointer mitk::Mesh::GetMesh() const
{
  return m_ItkData;
}

//##Documentation
//## @brief executes the given Operation
//##
//## implemented Operations are: OpNOTHING, 
//## OpINSERTLINE (inserts a new line to the cell. if the cell already contains a line, 
//## then the celltype will be set to Triangle(which actually adds two lines cause the object is closed),
//## OpMOVELINE (moves the points of the given cellId),
//## OpREMOVECELL (removes all lines of the given cell),
//## OpSELECTLINE (select the given line),
//## OpDESELECTLINE (deselects it)
//## 
//## Removing a line has to be done by an Interactor, which knows 
//## if the resultcell shall be closed or not.
void mitk::Mesh::ExecuteOperation(Operation* operation)
//adding only the operations, that aren't implemented by the pointset.
//if already implemented, then call superclass funktion (in default)
{
	mitk::LineOperation *lineOp = dynamic_cast<mitk::LineOperation *>(operation);
	if (lineOp == NULL)//if no lineoperation, then call superclass pointSet
	{
    	mitk::PointOperation *lineOp = dynamic_cast<mitk::PointOperation *>(operation);
    	if (lineOp == NULL)//if no PointOperation, then display error and return 
      {
        (StatusBar::GetInstance())->DisplayText("Message from mitkMesh: Recieved wrong type of operation!See mitkMeshInteractor.cpp", 10000);
        return;
      }
      Superclass::ExecuteOperation(operation);
	}

	switch (operation->GetOperationType())
	{
	case OpNOTHING:
		break;
	case OpINSERTLINE:
    //insert a line in the given cellId
    //the points with the given PointIds have to be in the pointset.
    //needed: CellId, first PointID and second PointID
    //if a cell aready exists, that the line gets added to the cell. 
    //To identify where to add the Line, we detect the given PointId, that has to be in the Cell, too, and 
    //insert the line behind that position. Ids have to be in order: <existingId>, <newId>
    //Example: existing:1,2,4; inserting 2,3 so that:1,2,3,4
		{
      int cellId = lineOp->GetCellId();
      int pIdA = lineOp->GetPIdA();
      int pIdB = lineOp->GetPIdB();

      //the points of the given PointIds have to exist in the PointSet
      bool ok;
      ok = m_ItkData->GetPoints()->IndexExists(pIdA);
      if (!ok) 
        return;
      ok = m_ItkData->GetPoints()->IndexExists(pIdB);
      if (!ok) 
        return;

      //so the points do exsist. So now check, if there is already a cell with the given Id
      DataType::CellAutoPointer cell;
      ok = m_ItkData->GetCell(cellId, cell);
      if (ok)//it already exists
      {
        //now we check, which PointIds of the given PointIds already exist in the cell
        //exactly one equal Id has to exist; if it doesn't, then we quit!
        PointIdIterator pit = cell->PointIdsBegin();
        PointIdIterator end = cell->PointIdsEnd();
        bool aEx = false; //bool for pIdA exists
        bool bEx = false; //bool for pIdB exists
        int posA, posB;//position of the PointId in the cell
        int i = 0;
        while( pit != end )
        {
          if (*pit = pIdA)
          {
            aEx = true;
            posA = i;
          }
          if (*pit = pIdB)
          {
            bEx = true;
            posB = i;
          }
          i++;
        }
        if (aEx && bEx)
          return;//none and both Ids are not allowed, cause then they already should be connected 
                 //or a new cell has to be made by an Interactor

        //now store the old points into an array and add the new PointIds into the desired place
        unsigned int nuPoints = cell->GetNumberOfPoints();
        //const unsigned int newNuPoints = nuPoints +2;
        std::vector<unsigned int> newPoints;//pointIds[newNuPoints];
        pit = cell->PointIdsBegin();
        end = cell->PointIdsEnd();
        i = 0;
        while( pit != end )
        {
          if ((*pit) = pIdA)
          {
            //now we have found the place to add the other after
            newPoints[i] = (*pit);//of pIdA
            i++;
            newPoints[i] = pIdB;
          }
          else
            newPoints[i] = (*pit);

          pit++;
        }

        //now we have the Ids, that existed before combined with the new ones so delete the old cell
        //doesn't seem to be necessary!

        //and build up a new one:
        switch (newPoints.size())
        {
        case 3:
          cell.TakeOwnership( new TriangleType );
        default:
          cell.TakeOwnership( new PolygonType );
        }

        //add the new pointIds to the Cell
        for (i = 0; i < newPoints.size(); i++)
        {
          cell->SetPointId(i, newPoints[i]);
        }

      }
      else //if the cell didn't exist before, then create it
      {
        CellAutoPointer line;
        line.TakeOwnership( new LineType );
        line->SetPointId( 0, pIdA ); // line between points 0 and 1
        line->SetPointId( 1, pIdB );
        m_ItkData->SetCell( cellId, line );
      }
		}
		break;
	case OpMOVELINE://(moves the points of the given cellId)
		{
      mitk::LineVecOperation *lineVecOp = dynamic_cast<mitk::LineVecOperation *>(operation);
      if (lineVecOp == NULL)
      {
        (StatusBar::GetInstance())->DisplayText("Message from mitkMesh: Recieved wrong type of operation!See mitkMeshInteractor.cpp", 10000);
          return;
      }

      //create two operations out of the one operation and call superclass

      ITKPoint3D pointA, pointB;
      m_ItkData->GetPoint(lineOp->GetPIdA(), &pointA);
      m_ItkData->GetPoint(lineOp->GetPIdB(), &pointB);

      pointA[0] += lineVecOp->GetVector()[0];
      pointA[1] += lineVecOp->GetVector()[1];
      pointA[2] += lineVecOp->GetVector()[2];
      pointB[0] += lineVecOp->GetVector()[0];
      pointB[1] += lineVecOp->GetVector()[1];
      pointB[2] += lineVecOp->GetVector()[2];

      mitk::PointOperation* operationA = new mitk::PointOperation(OpMOVE, pointA, lineOp->GetPIdA());
      mitk::PointOperation* operationB = new mitk::PointOperation(OpMOVE, pointB, lineOp->GetPIdB());

      Superclass::ExecuteOperation(operationA);
      Superclass::ExecuteOperation(operationB);
		}
		break;
	case OpREMOVECELL:
		{
      //int cellId = lineOp->GetCellId();
      //DataType::CellAutoPointer cell;
      //bool ok = m_ItkData->GetCell(lineOp->GetCellId(), cell);
      //m_ItkData->SetCell(lineOp->GetCellId(),NULL)
      
		}
		break;
	case OpSELECTLINE://(select the given line)
		{
      m_ItkData->SetCellData(lineOp->GetCellId(), true);
		}
		break;
	case OpDESELECTLINE://(deselect the given line)
		{
      m_ItkData->SetCellData(lineOp->GetCellId(), false);
		}
		break;
	default:
    return;
	}
	//to tell the mappers, that the data is modifierd and has to be updated
	this->Modified();

  ((const itk::Object*)this)->InvokeEvent(itk::EndEvent());

  //UpdateAllWidgets();
}
