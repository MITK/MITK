#ifndef MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0
#define MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0

#include "mitkCommon.h"
#include "BaseRenderer.h"
#include "Operation.h"
#include "itkMacro.h"

#define mitkGetMacro(name,type) \
  virtual type Get##name () \
  { \
    return this->m_##name; \
  }


namespace mitk {

//##ModelId=3EF1CC2B03BD
class DisplayCoordinateOperation : public Operation
{
  public:
    //##ModelId=3EF1D349027C
    DisplayCoordinateOperation(mitk::OperationType operationType, int execId, 
        mitk::BaseRenderer* renderer,
        const mitk::Point2D& startDisplayCoordinate, 
        const mitk::Point2D& lastDisplayCoordinate, 
        const mitk::Point2D& currentDisplayCoordinate
    );
  
    //##ModelId=3EF1CCBB03B0
    virtual ~DisplayCoordinateOperation();
    //##ModelId=3EF1D3BB03A3
    virtual void Execute();
    //##ModelId=3EF1D5CF0235
    mitk::BaseRenderer* GetRenderer();

    mitkGetMacro(StartDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(LastDisplayCoordinate, mitk::Point2D);
    mitkGetMacro(CurrentDisplayCoordinate, mitk::Point2D);

    mitk::Vector2D GetLastToCurrentDisplayVector();
    mitk::Vector2D GetStartToCurrentDisplayVector();
    mitk::Vector2D GetStartToLastDisplayVector();

    //##ModelId=3EF1D5C80393
	//##Documentation
	//##Constants for Operation::m_ExecutionId
	static const int NOTHING;
    //##ModelId=3EF1D5C803C6
	static const int PRINTPOINT;

  private:
    //##ModelId=3EF1D2A00053
      mitk::BaseRenderer::Pointer m_Renderer;

    //##ModelId=3EF1CD2D03A0
    const mitk::Point2D m_StartDisplayCoordinate;
    const mitk::Point2D m_LastDisplayCoordinate;
    const mitk::Point2D m_CurrentDisplayCoordinate;
};

}

#endif /* MITKDISPLAYCOORDINATEOPERATION_H_HEADER_INCLUDED_C10E33D0 */
