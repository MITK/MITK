#ifndef LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C
#define LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C

#include "mitkCommon.h"
#include <itkObjectFactory.h>

namespace mitk {

/*!
* class implements level window data structure
* current min and max value are stored in m_Min and m_Max.
* The maximum and minimum of valid value range is stored in
* m_RangeMin and m_RangeMax
*/
class LevelWindow
{
public:
    LevelWindow();
    LevelWindow(const mitk::LevelWindow& levWin);
    ~LevelWindow();

//    mitkClassMacro(LevelWindow, itk::Object);

    /*!
    *	\brief method returns the level value, i.e. the center of
    *				current grey value interval
    */
    //##ModelId=3E0B12640203
    float GetLevel() const;

    /*!
    * \brief returns the current window size
    */
    //##ModelId=3E0B12960165
    float GetWindow() const;

    /*!
    * getter for window minimum value
    */
    //##ModelId=3E0B130203B9
    float GetMin() const;

    /*!
    * getter for window maximum value
    */
    //##ModelId=3E0B130A0049
    float GetMax() const;

    /*!
    * setter for level value
    */
    //##ModelId=3E0B130E037A
    void SetLevel(float level);

    /*!
    * setter for window value
    */
    //##ModelId=3E0B131C0168
    void SetWindow(float window);

    /*!
    *
    */
    //##ModelId=3E0B132303A3
    void SetLevelWindow(float level, float window);

    /*!
    * setter for window minimum value
    */
    //##ModelId=3E0B13320187
    void SetMin(float min);

    /*!
    * setter for window maximum value
    */
    //##ModelId=3E0B1339006F
    void SetMax(float max);

    /*!
    * setter for window min and max values
    */
    //##ModelId=3E0B133D0292
    void SetMinMax(float min, float max);

    /*!
    * setter for total range minimum value
    */
    //##ModelId=3EA969CD005D
    void SetRangeMin(float min);

    /*!
    * setter for total range maximum value
    */
    //##ModelId=3EA969CD0067
    void SetRangeMax(float max);

    /*!
    * getter for total range minimum value
    */
    //##ModelId=3EA969CD0069
    float GetRangeMin() const;

    /*!
    * getter for total range maximum value
    */
    //##ModelId=3EA969CD006B
    float GetRangeMax() const;

    /**!
    * \brief method returns the size of the grey value range
    */
    //##ModelId=3EA969CD0072
    float GetRange() const;


    /*!
    * \brief equality operator implementation
    */
    //##ModelId=3EA969CD0074
    virtual bool operator==(const LevelWindow& levWin) const;

    /*!
    * \brief non equality operator implementation
    */
    //##ModelId=3EA969CD007C
    virtual bool operator!=(const LevelWindow& levWin) const;

    /*!
    * \brief implementation necessary because operator made
    *	private in itk::Object
    */
    //##ModelId=3EA969CD007F
    virtual LevelWindow& operator=(const LevelWindow& levWin);


protected:

    /*!
    * lower limit of current window
    */
    //##ModelId=3E0B122C0393
    float m_Min;

    /*!
    * upper limit of current window
    */
    //##ModelId=3E0B123D0167
    float m_Max;

    /*!
    * minimum gray value of the window
    */
    //##ModelId=3E19538C0312
    float m_RangeMin;

    /*!
    * maximum gray value of the window
    */
    //##ModelId=3E19538C0326
    float m_RangeMax;

    /*!
    * confidence tests
    */
    //##ModelId=3EA969CD0087
    inline void testValues() {
        if ( m_Min < m_RangeMin )
            m_Min = m_RangeMin;

        if ( m_Max > m_RangeMax )
            m_Max = m_RangeMax;

        if ( m_Min > m_Max )
            m_Min = m_Max;
    }

};

} // namespace mitk
#endif /* LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C */
