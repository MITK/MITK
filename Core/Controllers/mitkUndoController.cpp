#include "UndoController.h"
#include "LimitedLinearUndo.h"

//static member-variables init.
mitk::UndoModel *m_CurUndoModel;
mitk::UndoModelMap m_UndoModelList;
mitk::UndoType m_CurUndoType;

static const mitk::UndoType DEFAULTUNDOMODEL = mitk::LIMITEDLINEARUNDO;


mitk::UndoController::UndoController(UndoType undoType)
{
	switch (undoType)
	{
		case LIMITEDLINEARUNDO:
			m_CurUndoModel = new mitk::LimitedLinearUndo;
			m_CurUndoType = undoType;
			m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
			break;
		//case ###
			//insert here, in add- and RemoveUndoModel new sets of UndoModels!
            //break;
		default :
			m_CurUndoModel = new LimitedLinearUndo;
			m_CurUndoType = undoType;
			m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
	}

}

//##ModelId=3E5F543402C5
bool mitk::UndoController::SetOperationEvent(OperationEvent* operationEvent)
{
	m_CurUndoModel->SetOperationEvent(operationEvent);
	return true;
}

//##ModelId=3E5F55C80207
bool mitk::UndoController::Undo()
{
	return m_CurUndoModel->Undo();
}

//##ModelId=3E5F55E6003E
bool mitk::UndoController::Redo()
{
	return m_CurUndoModel->Redo();
}

//##ModelId=3E5F56EB017A
//##Documentation
//##Switches the UndoModel to the given Type
//##if there is no equal Type in List, then return false
bool mitk::UndoController::SwitchUndoModel(UndoType undoType)
{
	if (m_CurUndoType == undoType)
	{
		return true;//already switched, don't need to be switched!
	}

	UndoModelMapIter undoModelIter = m_UndoModelList.find(undoType);
	if (undoModelIter == m_UndoModelList.end())
	{//undoType not found in List
		return false;
	}

	//found-> switch to UndoModel
	m_CurUndoModel = (undoModelIter)->second;
	m_CurUndoType = (undoModelIter)->first;
	return true;
}

//##ModelId=3E9C45D502B9
//##Documentation
//##adds a new kind of UndoModel to the set of UndoModels
//##and switches to that UndoModel
//##if the UndoModel exists already in the List, then nothing is done
bool mitk::UndoController::AddUndoModel(UndoType undoType)
{
	if (m_UndoModelList.find(undoType) != m_UndoModelList.end())
	{ //UndoModel already exists
		return false;
	}
	//doesn't already exist in list
	switch (undoType)
	{
		case LIMITEDLINEARUNDO:
			m_CurUndoModel = new LimitedLinearUndo;
			m_CurUndoType = undoType;
			m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
		break;

		//case:... insert new UndoModels
		//break;
		
		default:
			//that undoType is not implemented!
			return false;
	}
	return true;
}

//##ModelId=3E9C45D502EB
//##Documentation
//##Removes an UndoModel from the set of UndoModels
//##If that UndoModel is currently selected, then the DefaultUndoModel(const) is set. 
//##If the default is not in List, then the first UndoModel is set.
//##UndoList may not be empty, so if the UndoType is the last, then return false;
bool mitk::UndoController::RemoveUndoModel(UndoType undoType)
{
	if (m_UndoModelList.size() < 2)
	{//for no empty m_UndoModelList
		return false;
	}
	//try deleting Element
	int ok = m_UndoModelList.erase(undoType);
	if (ok == 0) 
	{//delete unsucessful; Element of undoType not found
		return false;
	}
	
	//if m_CurUndoModel is the one removed, then change it to default or to the next or first
	if (m_CurUndoType == undoType)
	{//we have to change m_CurUndoModel and m_CurUndoType to an existing Model
	
		//if defaultUndoModel exists, then set to default
		UndoModelMapIter undoModelIter = m_UndoModelList.find(DEFAULTUNDOMODEL);
		if (undoModelIter == m_UndoModelList.end())
		{//DefaultUndoModel does not exists in m_CurUndoModelList
			undoModelIter = m_UndoModelList.begin();
		}
		m_CurUndoModel = (undoModelIter)->second;
		m_CurUndoType = (undoModelIter)->first;
		return true;
	}
	//m_CurUndoType was not undoType and is not changed
	return true;
}

