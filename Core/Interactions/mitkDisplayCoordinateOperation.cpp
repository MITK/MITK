#include "mitkDisplayCoordinateOperation.h"


//##ModelId=3EF1CCBB0392
mitk::DisplayCoordinateOperation::DisplayCoordinateOperation(mitk::OperationType operationType, 
        mitk::BaseRenderer* renderer,
        const mitk::Point2D& startDisplayCoordinate, 
        const mitk::Point2D& lastDisplayCoordinate, 
        const mitk::Point2D& currentDisplayCoordinate
)
  : mitk::Operation(operationType), 
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


//##ModelId=3EF1D5CF0235
mitk::BaseRenderer* mitk::DisplayCoordinateOperation::GetRenderer()
{
    return m_Renderer;
}

//##ModelId=3F01770902D6
mitk::Vector2D mitk::DisplayCoordinateOperation::GetLastToCurrentDisplayVector()
{
    return m_CurrentDisplayCoordinate-m_LastDisplayCoordinate;
}

//##ModelId=3F01770902D7
mitk::Vector2D mitk::DisplayCoordinateOperation::GetStartToCurrentDisplayVector()
{
    return m_CurrentDisplayCoordinate-m_StartDisplayCoordinate;
}

//##ModelId=3F01770902E6
mitk::Vector2D mitk::DisplayCoordinateOperation::GetStartToLastDisplayVector()
{
    return m_LastDisplayCoordinate-m_StartDisplayCoordinate;
}
