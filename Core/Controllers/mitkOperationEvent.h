#ifndef OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC

#define OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC



#include "mitkCommon.h"

#include "Operation.h"

#include "OperationActor.h"

#include "UndoModel.h"





namespace mitk {

//##ModelId=3E5F60F301A4

class OperationEvent

{		

  public:

    //##ModelId=3E957AE700E6

    OperationEvent(OperationActor* destination, Operation* operation, Operation* undoOperation, int objectEventId, int groupEventId);



    //##ModelId=3E5F610D00BB

    Operation* GetOperation();



    //##ModelId=3E9B07B501A7

	static int GenerateObjectEventId();

    //##ModelId=3E9B07B50220

	static int GenerateGroupEventId();

    //##ModelId=3E9B07B502AC

	OperationActor* GetDestination();



	friend class UndoModel;	



  protected:

    //##ModelId=3E957C1102E3

    void swapOperations();



  private:

    //##ModelId=3E9B07B40374

	static int m_NextObjectEventId;



    //##ModelId=3E9B07B5002B

	static int m_NextGroupEventId;



    //##ModelId=3E5F61DB00D6

    OperationActor* m_Destination;



    //##ModelId=3E5F6B1E0107

    Operation* m_Operation;



    //##ModelId=3E5F6B2E0060

    Operation* m_UndoOperation;



    //##ModelId=3E7F488E022B

    int m_ObjectEventId;



    //##ModelId=3E7F48C60335

    int m_GroupEventId;



    //##ModelId=3E9B07B500D5

	bool m_Swaped;//true, if operation and undooperation have been swaped

};

} //namespace mitk





#endif /* OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC */

