#include "mitkDisplayCoordinateOperation.h"

//##ModelId=3EF1D5C80393
const int mitk::DisplayCoordinateOperation::NOTHING = 0;
//##ModelId=3EF1D5C803C6
const int mitk::DisplayCoordinateOperation::PRINTPOINT = 1;


//##ModelId=3EF1CCBB0392
mitk::DisplayCoordinateOperation::DisplayCoordinateOperation(mitk::OperationType operationType, int execId, 
        mitk::BaseRenderer* renderer,
        const mitk::Point2D& startDisplayCoordinate, 
        const mitk::Point2D& lastDisplayCoordinate, 
        const mitk::Point2D& currentDisplayCoordinate
)
  : mitk::Operation(operationType, execId), 
    m_Renderer(renderer), 
    m_StartDisplayCoordinate(startDisplayCoordinate), 
    m_LastDisplayCoordinate(lastDisplayCoordinate),
    m_CurrentDisplayCoordinate(currentDisplayCoordinate)
{
}


//##ModelId=3EF1CCBB03B0
mitk::DisplayCoordinateOperation::~DisplayCoordinateOperation()
{
}

//##ModelId=3EF1D3BB03A3
void mitk::DisplayCoordinateOperation::Execute()
{
	switch (m_ExecutionId)
	{
	case PRINTPOINT:
        std::cout<<"Message from DisplayCoordinateOperation.cpp::Execute() : "
            << "StartDisplayCoordinate:" <<     m_StartDisplayCoordinate 
            << "LastDisplayCoordinate:" <<      m_LastDisplayCoordinate 
            << "CurrentDisplayCoordinate:" <<   m_CurrentDisplayCoordinate 
            << "LastToCurrentDisplayVector:" << GetLastToCurrentDisplayVector()
            << std::endl;
		break;
	default:
		std::cout<<"Message from DisplayCoordinateOperation.cpp::Execute() : No unique ExecuteId!"<<std::endl;
	}
}

//##ModelId=3EF1D5CF0235
mitk::BaseRenderer* mitk::DisplayCoordinateOperation::GetRenderer()
{
    return m_Renderer;
}

//##ModelId=3EF1D5DB0337
mitk::Vector2D mitk::DisplayCoordinateOperation::GetLastToCurrentDisplayVector()
{
    return m_CurrentDisplayCoordinate-m_LastDisplayCoordinate;
}

mitk::Vector2D mitk::DisplayCoordinateOperation::GetStartToCurrentDisplayVector()
{
    return m_CurrentDisplayCoordinate-m_StartDisplayCoordinate;
}

mitk::Vector2D mitk::DisplayCoordinateOperation::GetStartToLastDisplayVector()
{
    return m_LastDisplayCoordinate-m_StartDisplayCoordinate;
}
