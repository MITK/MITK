#include "BaseController.h"

//##ModelId=3DF8F5CA03D8
mitk::Stepper &mitk::BaseController::GetSlice()
{
	itkExceptionMacro("BaseController not yet implemented."); 	
	return m_Slice;
}

//##ModelId=3DF8F61101E3
mitk::Stepper &mitk::BaseController::GetTime()
{
	itkExceptionMacro("BaseController not yet implemented."); 	
	return m_Time;
}

//##ModelId=3DD528260235
bool mitk::BaseController::RemoveNavigator(mitk::Navigator* navigator)
{
	itkExceptionMacro("BaseController not yet implemented."); 	
	return false;
}

//##ModelId=3DD52870008D
bool mitk::BaseController::RemoveRenderer(mitk::BaseRenderer* renderer)
{
	itkExceptionMacro("BaseController not yet implemented."); 	
	return false;
}

//##ModelId=3DD528500222
bool mitk::BaseController::AddRenderer(mitk::BaseRenderer* renderer)
{
	itkWarningMacro("BaseController not yet implemented."); 	
	return true;
}

//##ModelId=3DD5268F0013
bool mitk::BaseController::AddNavigator(mitk::Navigator* navigator)
{
	itkExceptionMacro("BaseController not yet implemented."); 	
	return false;
}

//##ModelId=3E3AE32B0048
mitk::BaseController::BaseController()
{
}


//##ModelId=3E3AE32B0070
mitk::BaseController::~BaseController()
{
}

