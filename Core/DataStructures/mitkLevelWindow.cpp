#include "LevelWindow.h"
#include <ipFunc/ipFunc.h>
#include <algorithm>

//##ModelId=3ED91D060288
mitk::LevelWindow::LevelWindow() : m_Min(-1024), m_Max(4096), m_RangeMin(-1024), m_RangeMax(4096)
{

}

//##ModelId=3ED91D060298
mitk::LevelWindow::LevelWindow(const mitk::LevelWindow& levWin)
{
    *this=levWin;
}

//##ModelId=3ED91D06029A
mitk::LevelWindow::~LevelWindow()
{
}

//##ModelId=3E0B12640203
float mitk::LevelWindow::GetLevel() const
{
	return (m_Max-m_Min)/2.0 + m_Min;
}

//##ModelId=3E0B12960165
float mitk::LevelWindow::GetWindow() const
{
	return (m_Max-m_Min);
}

//##ModelId=3E0B130203B9
float mitk::LevelWindow::GetMin() const
{
	return m_Min;
}

//##ModelId=3E0B130A0049
float mitk::LevelWindow::GetMax() const
{
	return m_Max;
}

//##ModelId=3E0B130E037A
void mitk::LevelWindow::SetLevel(float level)
{
	float halfWindow = (m_Max - m_Min) / 2;
	m_Min = level - halfWindow;
	m_Max = level + halfWindow;
	//double window = GetWindow();

	//m_Min = level - window / 2.0;
	//m_Max = level + window / 2.0;

	testValues();
	//if( m_Min < m_RangeMin )
 //   {
	//	m_Min = m_RangeMin;
	//	m_Max = m_Min + window;
 //   }
	//if( m_Max > m_RangeMax )
 //   {
	//	m_Max = m_RangeMax;
	//	m_Min = m_Max - window;
 //   }
}

//##ModelId=3E0B131C0168
void mitk::LevelWindow::SetWindow(float window)
{
	float level = m_Min + (m_Max - m_Min) / 2;	
	float halfWindow = window / 2;

	m_Min = level - halfWindow;
	m_Max = level + halfWindow;
	testValues();

	//double level = GetLevel();

	//m_Min = level - window / 2.0;
	//m_Max = level + window / 2.0;

	//testValues();

}

//##ModelId=3E0B132303A3
void mitk::LevelWindow::SetLevelWindow(float level, float window)
{
	m_Min = level - window / 2.0;
	m_Max = level + window / 2.0;

	testValues();
	//if( m_Min < m_RangeMin )
 //   {
	//	m_Min = m_RangeMin;
	//	m_Max = m_Min + window;
 //   }
	//if( m_Max > m_RangeMax )
 //   {
	//	m_Max = m_RangeMax;
	//	m_Min = m_Max - window;
 //   }
}

//##ModelId=3E0B13320187
void mitk::LevelWindow::SetMin(float min)
{
	m_Min = min;
	testValues(); 
}

//##ModelId=3E0B1339006F
void mitk::LevelWindow::SetMax(float max)
{
	m_Max = max	;
	testValues();
}

//##ModelId=3E0B133D0292
void mitk::LevelWindow::SetMinMax(float min, float max)
{
	if(min>max)
		std::swap(min,max);

	SetMin(min);
	SetMax(max);
}

//##ModelId=3EA969CD005D
void mitk::LevelWindow::SetRangeMin(float min)
{
	m_RangeMin = min;
	testValues();
}

//##ModelId=3EA969CD0067
void mitk::LevelWindow::SetRangeMax(float max)
{
		m_RangeMax = max;
		testValues();
}

//##ModelId=3EA969CD0069
float mitk::LevelWindow::GetRangeMin() const
{
	return m_RangeMin;
}

//##ModelId=3EA969CD006B
float mitk::LevelWindow::GetRangeMax() const
{
	return m_RangeMax;
}

//##ModelId=3EA969CD0072
float mitk::LevelWindow::GetRange() const
{
	return  (m_RangeMax > 0) ? (m_RangeMax - m_RangeMin) : (m_RangeMin - m_RangeMax);
}

void mitk::LevelWindow::SetAuto(ipPicDescriptor* pic)
{
    if ( pic == NULL )
    {
        return;        
    }
    ipPicTSV_t *tsv = ipPicQueryTag( pic, "LEVEL/WINDOW" );
	if( tsv != NULL )
	{
		double level;
		double window;
        #define GET_C_W( type, tsv, C, W )			\
		level = ((type *)tsv->value)[0];    \
		window = ((type *)tsv->value)[1];
		
		ipPicFORALL_2( GET_C_W, tsv, level, window );

		SetLevelWindow( level, window );
	}
    else
    {
        if( pic->data != NULL )
        {
            ipFloat8_t min = 0.0;
            ipFloat8_t max = 0.0;
            ipFuncExtr( pic, &min, &max );
            this->SetMin( static_cast<float>( min ) );
            this->SetMax( static_cast<float>( max ) );
        }
    }
}

/*!
 * \brief equality operator inplementation
 */
//##ModelId=3EA969CD0074
bool mitk::LevelWindow::operator==(const mitk::LevelWindow& levWin) const
{
		if ( m_RangeMin == levWin.GetRangeMin() && 
			 m_RangeMax == levWin.GetRangeMax() && 
			 m_Min == levWin.GetMin() && m_Max == levWin.GetMax()) {
         
				 return true;
			 }
		else {
			return false;	
		}
}

/*!
 * \brief equality operator inplementation
 */
//##ModelId=3EA969CD007C
bool mitk::LevelWindow::operator!=(const mitk::LevelWindow& levWin) const
{
		if ( m_RangeMin == levWin.GetRangeMin() && 
			 m_RangeMax == levWin.GetRangeMax() && 
			 m_Min == levWin.GetMin() && m_Max == levWin.GetMax()) {
         
				 return false;
			 }
		else {
			return true;	
		}
}

/*!
 * \brief non equality operator inplementation
 */
//##ModelId=3EA969CD007F
mitk::LevelWindow& mitk::LevelWindow::operator=(const mitk::LevelWindow& levWin)
{
	if (this == &levWin) {
		return *this;
	}
	else {
		m_RangeMin = levWin.GetRangeMin();
		m_RangeMax = levWin.GetRangeMax();
		m_Min= levWin.GetMin();
		m_Max= levWin.GetMax();
		return *this;
	}
}

