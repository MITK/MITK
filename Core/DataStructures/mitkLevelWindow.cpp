#include "LevelWindow.h"
#include <algorithm>

//##ModelId=3E0B12640203
float mitk::LevelWindow::GetLevel()
{
	return (m_Max-m_Min)/2.0;
}

//##ModelId=3E0B12960165
float mitk::LevelWindow::GetWindow()
{
	return (m_Max-m_Min);
}

//##ModelId=3E0B130203B9
float mitk::LevelWindow::GetMin()
{
	return m_Min;
}

//##ModelId=3E0B130A0049
float mitk::LevelWindow::GetMax()
{
	return m_Max;
}

//##ModelId=3E0B130E037A
void mitk::LevelWindow::SetLevel(float level)
{
	double window = GetWindow();

	m_Min = level - window / 2.0;
	m_Max = level + window / 2.0;
  
	if( m_Min < m_RangeMin )
    {
		m_Min = m_RangeMin;
		m_Max = m_Min + window;
    }
	if( m_Max > m_RangeMax )
    {
		m_Max = m_RangeMax;
		m_Min = m_Max - window;
    }
}

//##ModelId=3E0B131C0168
void mitk::LevelWindow::SetWindow(float window)
{
	double level = GetLevel();

	m_Min = level - window / 2.0;
	m_Max = level + window / 2.0;
  
	if( m_Min < m_RangeMin )
    {
		m_Min = m_RangeMin;
		m_Max = m_Min + window;
    }
	if( m_Max > m_RangeMax )
    {
		m_Max = m_RangeMax;
		m_Min = m_Max - window;
    }
}

//##ModelId=3E0B132303A3
void mitk::LevelWindow::SetLevelWindow(float level, float window)
{
	m_Min = level - window / 2.0;
	m_Max = level + window / 2.0;
  
	if( m_Min < m_RangeMin )
    {
		m_Min = m_RangeMin;
		m_Max = m_Min + window;
    }
	if( m_Max > m_RangeMax )
    {
		m_Max = m_RangeMax;
		m_Min = m_Max - window;
    }
}

//##ModelId=3E0B13320187
void mitk::LevelWindow::SetMin(float min)
{
	if( m_Min < m_RangeMin )
		m_Min = m_RangeMin;
	else
		m_Min = min;
	if(m_Min>m_Max)
		m_Max=m_Min;
}

//##ModelId=3E0B1339006F
void mitk::LevelWindow::SetMax(float max)
{
	if( max > m_RangeMax )
		m_Max = m_RangeMax;
	else
		m_Max = max;
	if(m_Max<m_Min)
		m_Min=m_Max;
}

//##ModelId=3E0B133D0292
void mitk::LevelWindow::SetMinMax(float min, float max)
{
	if(min>max)
		std::swap(min,max);

	if( m_Min < m_RangeMin )
		m_Min = m_RangeMin;
	else
		m_Min = min;
	if( max > m_RangeMax )
		m_Max = m_RangeMax;
	else
		m_Max = max;
}

//##ModelId=3E345B580260
mitk::LevelWindow::LevelWindow()
{
}


//##ModelId=3E345B58029C
mitk::LevelWindow::~LevelWindow()
{
}

