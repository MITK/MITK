#include "mitkStepper.h"

//##ModelId=3DF8B9DD0141
unsigned int mitk::Stepper::GetPos()
{
	return m_Pos;
}

//##ModelId=3DF8BA0002D7
unsigned int mitk::Stepper::GetSteps()
{
	return m_Steps;
}

//##ModelId=3DF8B9B20019
void mitk::Stepper::SetPos(unsigned int pos)
{
	if((pos>=0) && (pos<=m_Steps))
		m_Pos=pos;
}

//##ModelId=3DF8BA8301FC
unsigned int mitk::Stepper::GetStepSize()
{
	return m_StepSize;
}

//##ModelId=3DF8B9410142
void mitk::Stepper::Previous()
{
	if(m_Pos>0)
		--m_Pos;
}

//##ModelId=3DF8B92F01DF
void mitk::Stepper::Last()
{
	m_Pos = m_Steps;
}

//##ModelId=3DF8BA9301EC
void mitk::Stepper::SetStepSize(unsigned int stepSize)
{
	if((stepSize>0) && (stepSize<m_Steps))
		m_StepSize = stepSize;
}

//##ModelId=3DF8B91502F8
void mitk::Stepper::First()
{
	m_Pos = 0;
}

//##ModelId=3DF8B92703A4
void mitk::Stepper::Next()
{
	if(m_Pos<m_Steps)
		++m_Pos;
}

//##ModelId=3DF8F6B40153
void mitk::Stepper::SetSteps(unsigned int steps)
{
	m_Steps=steps;
}

