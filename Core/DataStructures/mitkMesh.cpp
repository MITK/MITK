#include "mitkMesh.h"

#include <mitkOperation.h>
#include <mitkOperationActor.h>

#include <mitkLineOperation.h>
#include <mitkPointOperation.h>
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
//## OpREMOVELINE (removes the line between the two given points of the cellId),
//## OpSELECTLINE (select the given line),
//## OpDESELECTLINE (deselects it)
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
    //needed: CellId, first PointID and second PointID
		{
      int cellId = lineOp->GetCellId();
      int pIdA = lineOp->GetPIdA();
      int pIdB = lineOp->GetPIdB();

      DataType::CellAutoPointer cell;
      //check if the cellId already exists. If yes, then manually add the line, if no, then only add the line.
      bool ok = m_ItkData->GetCell(cellId, cell);
      if (ok)//it already exists
      {
        if( cell->GetType() == CellType::LINE_CELL )
        {
//          LineType * line = static_cast<LineType *>( cell );
        }
      }
      else //create it
      {}


		}
		break;
	case OpMOVELINE://(moves the points of the given cellId)
		{
		}
		break;
	case OpREMOVELINE://(removes the line between the two given points of the cellId)
		{
		}
		break;
	case OpSELECTLINE://(select the given line)
		{
		}
		break;
	case OpDESELECTLINE://(deselect the given line)
		{

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
