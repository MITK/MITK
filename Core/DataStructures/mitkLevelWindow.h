/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkCommon.h"
#include "mitkVector.h"
#include <ipPic/ipPic.h>

namespace mitk {

class Image;

//##Documentation
//## @brief Class to store level/window values
//##
//## Current min and max value are stored in m_Min and m_Max.
//## The maximum and minimum of valid value range is stored in
//## m_RangeMin and m_RangeMax.
//##
//## See documentation of SetAuto for information on how the
//## level window is initialized from an image.
//##
//## @ingroup DataTree
class LevelWindow
{
public:
  LevelWindow(ScalarType level=128, ScalarType window=256);
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
  * getter for window minimum value
  */
  ScalarType GetMin() const;

  /*!
  * getter for window maximum value
  */
  ScalarType GetMax() const;

  /*!
  * setter for level value
  */
  void SetLevel(ScalarType level);

  /*!
  * setter for window value
  */
  void SetWindow(ScalarType window);

  /*!
  * setter for level and window value
  */
  void SetLevelWindow(ScalarType level, ScalarType window);

  /*!
  * setter for window minimum value
  */
  void SetMin(ScalarType min);

  /*!
  * setter for window maximum value
  */
  void SetMax(ScalarType max);

  /*!
  * setter for window min and max values
  */
  void SetMinMax(ScalarType min, ScalarType max);

  /*!
  * setter for total range minimum value
  */
  void SetRangeMin(ScalarType min);

  /*!
  * setter for total range maximum value
  */
  void SetRangeMax(ScalarType max);

  /*!
  * getter for total range minimum value
  */
  ScalarType GetRangeMin() const;

  /*!
  * getter for total range maximum value
  */
  ScalarType GetRangeMax() const;

  /**!
  * \brief method returns the size of the grey value range
  */
  ScalarType GetRange() const;

  /**!
  * \brief sets level/window according to the tags in the given ipPicDescriptor
  * \return @a true if tags where found
  */
  bool SetAutoByPicTags(const ipPicDescriptor* pic);

  /**!
  * \brief sets level/window to the second min/max greyvalues of the given Image
  */
  void SetAuto(mitk::Image* image, bool tryPicTags = true, bool guessByCentralSlice = true);

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
  * confidence tests
  */
  inline void testValues()
  {
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
