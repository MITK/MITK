#include "mitkBaseController.h"
#include "mitkBaseRenderer.h"

//##ModelId=3E3AE32B0048
mitk::BaseController::BaseController() : m_Renderer(NULL), m_LastUpdateTime(0)
{
  m_Slice = Stepper::New();
  m_Time  = Stepper::New();
}

//##ModelId=3E3AE32B0070
mitk::BaseController::~BaseController()
{
}

//##ModelId=3DF8F5CA03D8
mitk::Stepper* mitk::BaseController::GetSlice()
{
  return m_Slice.GetPointer();
}

//##ModelId=3DF8F61101E3
mitk::Stepper* mitk::BaseController::GetTime()
{
  return m_Time.GetPointer();
}

//##ModelId=3DD528500222
/*!
  \todo implement it!
  */
bool mitk::BaseController::AddRenderer(mitk::BaseRenderer* renderer)
{
  m_RendererReference = m_Renderer = renderer;
	return true;
}
