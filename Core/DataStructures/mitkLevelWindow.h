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
class LevelWindow : public itk::Object
{
  public:

		/** Standard class typedefs. */
    //##ModelId=3E33EEF40183
	typedef LevelWindow              Self;
    //##ModelId=3E33EEF401AB
    typedef itk::Object              Superclass;
    //##ModelId=3E33EEF401C9
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E33EEF401F1
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
    itkTypeMacro(itk::Object,LevelWindow);

		/*!
		*	\brief method returns the level value, i.e. the center of  
		*				current grey value interval
		*/
		float GetLevel() const;

    /*!
		 * \brief returns the current window size
		 */
    float GetWindow() const;

    /*!
		 * getter for window minimum value
		 */
    float GetMin() const;

    /*!
		 * getter for window maximum value
		 */
    float GetMax() const;

    /*!
		 * setter for level value
		 */
    void SetLevel(float level);

    /*!
		 * setter for window value
		 */
    void SetWindow(float window);

    /*!
		 * 
		 */
    void SetLevelWindow(float level, float window);

    /*!
		 * setter for window minimum value
		 */
    void SetMin(float min);

    /*!
		 * setter for window maximum value
		 */
    void SetMax(float max);

    /*!
		 * setter for window min and max values
		 */
    void SetMinMax(float min, float max);

    /*!
		 * setter for total range minimum value
		 */
    void SetRangeMin(float min);

    /*!
		 * setter for total range maximum value
		 */
    void SetRangeMax(float max);

    /*!
		 * getter for total range minimum value
		 */
    float GetRangeMin() const;

    /*!
		 * getter for total range maximum value
		 */
    float GetRangeMax() const;

		/**! 
		* \brief method returns the size of the grey value range
		*/
		float GetRange() const; 

	
    /*!
     * \brief equality operator implementation
		 */
		virtual bool operator==(const LevelWindow& levWin) const;

    /*!
    	* \brief non equality operator implementation
		 */
		virtual bool operator!=(const LevelWindow& levWin) const;

		/*!
     * \brief implementation necessary because operator made 
		 *	private in itk::Object
		 */
		virtual LevelWindow& operator=(const LevelWindow& levWin);

	
	protected:

    /*!
		 * lower limit of current window
		 */
    float m_Min;

    /*!
		 * upper limit of current window
		 */
    float m_Max;

    /*!
		 * minimum gray value of the window
		 */
		float m_RangeMin;

    /*!
		 * maximum gray value of the window
		 */
		float m_RangeMax;

		/*!
		 * confidence tests
		 */
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
