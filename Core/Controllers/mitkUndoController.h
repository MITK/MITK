#ifndef UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79
#define UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79

#include "mitkCommon.h"
#include "UndoModel.h"
#include "OperationEvent.h"
#include <map>


namespace mitk {
//##ModelId=3E93EE1D001F
	enum UndoType
	{
		LIMITEDLINEARUNDO = 0,
		TREEUNDO = 1
	};
//##ModelId=3E9C45D5007E
typedef std::map<UndoType, UndoModel*> UndoModelMap;
//##ModelId=3E9C45D50092
typedef std::map<UndoType, UndoModel*>::iterator UndoModelMapIter;


//##ModelId=3E5F53AA026C
class UndoController
{
  public:
    //##ModelId=3E956E3B03A8
	UndoController(UndoType undoType);

    //##ModelId=3E5F543402C5
    bool SetOperationEvent(OperationEvent* operationEvent);

    //##ModelId=3E5F55C80207
    bool Undo();

    //##ModelId=3E5F55E6003E
    bool Redo();

	
    //##ModelId=3E5F56EB017A
    bool SwitchUndoModel(UndoType undoType);

    //##ModelId=3E9C45D502B9
	bool AddUndoModel(UndoType undoType);

    //##ModelId=3E9C45D502EB
	bool RemoveUndoModel(UndoType undoType);

  private:

    /**
     * current selected UndoModel
     **/
    //##ModelId=3E5F5DC701C5
    static UndoModel *m_CurUndoModel;
	
	/**
     * current selected Type of m_CurUndoModel
     **/
    //##ModelId=3E9C45D50256
	static UndoType m_CurUndoType;

    /**
     * different UndoModels to select and activate
     **/
    //##ModelId=3E9C45D5026A
    static UndoModelMap m_UndoModelList;

};
}//namespace mitk



#endif /* UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79 */
