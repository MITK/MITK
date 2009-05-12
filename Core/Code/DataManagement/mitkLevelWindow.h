/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C
#define LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C

#include "mitkVector.h"
#include "mitkCommon.h"

struct ipPicDescriptor;

namespace mitk {

class Image;

//##Documentation
//## @brief Class to store level/window values
//##
//## Current min and max value are stored in m_Min and m_Max.
//## The maximum and minimum of valid value range is stored in
//## m_RangeMin and m_RangeMax.
//## m_DefaultLevel amd m_DefaultWindow store the initial Level/Window values for the image.
//## m_DefaultRangeMin and m_DefaultRangeMax store the initial minrange and maxrange for the image.
//##
//## See documentation of SetAuto for information on how the
//## level window is initialized from an image.
//##
//## @ingroup DataTree
class MITK_CORE_EXPORT LevelWindow
{
public:
  LevelWindow(ScalarType level=127.5, ScalarType window=255.0);
  LevelWindow(const mitk::LevelWindow& levWin);
  virtual ~LevelWindow();

  /*!
  * \brief method returns the level value, i.e. the center of
  * current grey value interval
  */
  ScalarType GetLevel() const;

  /*!
  * \brief returns the current window size
  */
  ScalarType GetWindow() const;

  /*!
  * \brief method returns the default level value for image
  */
  ScalarType GetDefaultLevel() const;

  /*!
  * \brief returns the default window size for image
  */
  ScalarType GetDefaultWindow() const;

  /*!
  * \brief the default level and window for image will be reset
  */
  void ResetDefaultLevelWindow();

  /*!
  * getter for window minimum value
  */
  ScalarType GetMin() const;

  /*!
  * getter for window maximum value
  */
  ScalarType GetMax() const;

  /*!
  * setter for level and window value
  */
  void SetLevelWindow(ScalarType level, ScalarType window);

  /*!
  * setter for window min and max values
  */
  void SetMinMax(ScalarType min, ScalarType max);

  /*!
  * sets the window to its maximum Size in scaleRange
  */
  void SetToMaxWindowSize();

  /*!
  * setter for total range minimum and maximum value
  */
  void SetRangeMinMax(ScalarType min, ScalarType max);

  /*!
  * getter for total range minimum value
  */
  ScalarType GetRangeMin() const;

  /*!
  * getter for default total range minimum value
  */
  ScalarType GetDefaultRangeMin() const;

  /*!
  * getter for total range maximum value
  */
  ScalarType GetRangeMax() const;

  /*!
  * getter for default total range maximum value
  */
  ScalarType GetDefaultRangeMax() const;

  /*!
  * \brief the default min and max range for image will be reset
  */
  void ResetDefaultRangeMinMax();

  /**!
  * \brief method returns the size of the grey value range
  */
  ScalarType GetRange() const;

  /*!
  * setter for default level and window value
  */
  void SetDefaultLevelWindow(ScalarType level, ScalarType window);

  /*!
  * setter for default total range maximum value
  */
  void SetDefaultRangeMinMax(ScalarType min, ScalarType max);

  /**!
  * \brief sets level/window to the min/max greyvalues of the given Image
  */
  void SetAuto(const mitk::Image* image, bool tryPicTags = true, bool guessByCentralSlice = true);
  
  /**
   * If a level window is set to fixed, the set and get methods won't accept
   * modifications to the level window settings anymore. This behaviour can
   * be turned of by setting fixed to false;
   */
  void SetFixed( bool fixed );
  
  /** 
   * Returns whether the level window settings are fixed (@see SetFixed(bool)) or not
   */
  bool GetFixed() const;
  
  /** 
   * Returns whether the level window settings are fixed (@see SetFixed(bool)) or not
   */
  bool IsFixed() const;

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
  *  private in itk::Object
  */
  virtual LevelWindow& operator=(const LevelWindow& levWin);

protected:

  /**!
  * \brief sets level/window according to the tags in the given ipPicDescriptor
  * \return @a true if tags where found
  */
  bool SetAutoByPicTags(const ipPicDescriptor* pic);

  /*!
  * lower limit of current window
  */
  ScalarType m_Min;

  /*!
  * upper limit of current window
  */
  ScalarType m_Max;

  /*!
  * minimum gray value of the window
  */
  ScalarType m_RangeMin;

  /*!
  * maximum gray value of the window
  */
  ScalarType m_RangeMax;

  /*!
  * default minimum gray value of the window
  */
  ScalarType m_DefaultRangeMin;

  /*!
  * default maximum gray value of the window
  */
  ScalarType m_DefaultRangeMax;

  /*!
  * default value of the level for image
  */
  ScalarType m_DefaultLevel;

  /*!
  * default value of the window for image
  */
  ScalarType m_DefaultWindow;

  /*!
   * Defines whether the level window settings may be changed after
   * initialization or not.
   */
  bool m_Fixed;

  /*!
  * confidence tests
  *
  * if m_Min > m_Max, then the values for m_Min and m_Max will be exchanged
  *
  * if m_Min < m_RangeMin, m_Min will be set to m_RangeMin. m_Max will be decreased the same as m_Min will be increased, but minimum value for m_Max is also m_RangeMin.
  * 
  * if m_Max > m_RangeMax, m_Max will be set to m_RangeMax. m_Min will be increased the same as m_Max will be decreased, but maximum value for m_Min is also m_RangeMax.
  *
  */
  inline void testValues()
  {
    if ( m_Min > m_Max )
      std::swap(m_Min,m_Max);
    else if (m_Min == m_Max )
      m_Min = m_Max - 1;
    ScalarType diff;
    if ( m_Min < m_RangeMin )
    {
      diff = m_RangeMin - m_Min;
      m_Min = m_RangeMin;
      if (!((m_Max - diff) > m_RangeMin))
        m_Max = m_RangeMin + 1;
      else
        m_Max -= diff;
    }

    if ( m_Max > m_RangeMax )
    {
      diff = m_Max - m_RangeMax;
      m_Max = m_RangeMax;
      if (!((m_Min + diff) < m_RangeMax))
        m_Min = m_RangeMax - 1;
      else
        m_Min += diff;
    }
  }
  
};

} // namespace mitk
#endif /* LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C */
