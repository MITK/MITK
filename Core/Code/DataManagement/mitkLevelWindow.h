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
//## Current min and max value are stored in m_LowerWindowBound and m_UpperWindowBound.
//## The maximum and minimum of valid value range is stored in
//## m_RangeMin and m_RangeMax.
//## m_DefaultLevel amd m_DefaultWindow store the initial Level/Window values for the image.
//## m_DefaultRangeMin and m_DefaultRangeMax store the initial minrange and maxrange for the image.
//##
//## See documentation of SetAuto for information on how the
//## level window is initialized from an image.
//##
//## @ingroup DataManagement
class MITK_CORE_EXPORT LevelWindow
{
public:
  LevelWindow(ScalarType level=127.5, ScalarType window=255.0);
  LevelWindow(const mitk::LevelWindow& levWin);
  virtual ~LevelWindow();

  /*!
  * \brief method that returns the level value, i.e. the center of
  * the current grey value interval
  */
  ScalarType GetLevel() const;

  /*!
  * \brief returns the current window size, i.e the range size of the current grey value interval
  */
  ScalarType GetWindow() const;

  /*!
  * \brief method returns the default level value for the image
  */
  ScalarType GetDefaultLevel() const;

  /*!
  * \brief returns the default window size for the image
  */
  ScalarType GetDefaultWindow() const;

  /*!
  * \brief Resets the level and the window value to the default values 
  */
  void ResetDefaultLevelWindow();

  /*!
  * Returns the minimum Value of the window
  */
  ScalarType GetLowerWindowBound() const;

  /*!
  * Returns the upper window bound value of the window
  */
  ScalarType GetUpperWindowBound() const;

  /*!
  * To set the level and the window value
  */
  void SetLevelWindow(ScalarType level, ScalarType window);

  /*!
  * Set the lower and upper bound of the window
  */
  void SetWindowBounds(ScalarType lowerBound, ScalarType upperBound);

  /*!
  * sets the window to its maximum Size in scaleRange
  */
  void SetToMaxWindowSize();

  /*!
  * Set the range minimum and maximum value
  */
  void SetRangeMinMax(ScalarType min, ScalarType max);

  /*!
  * Get the range minimum value
  */
  ScalarType GetRangeMin() const;

  /*!
  * Get the range maximum value
  */
  ScalarType GetRangeMax() const;

  /*!
  * Get the default range minimum value
  */
  ScalarType GetDefaultRangeMin() const;

  /*!
  * Get the default range maximum value
  */
  ScalarType GetDefaultRangeMax() const;

  /*!
  * \brief the default min and max range for image will be reset
  */
  void ResetDefaultRangeMinMax();

  /**!
  * \brief returns the size of the grey value range
  */
  ScalarType GetRange() const;

  /*!
  * set the default level and window value
  */
  void SetDefaultLevelWindow(ScalarType level, ScalarType window);

  /*!
  * set the default total range maximum value
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
  * \brief equality operator implementation that allows to compare two level windows
  */
  virtual bool operator==(const LevelWindow& levWin) const;

  /*!
  * \brief non equality operator implementation that allows to compare two level windows
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
  * lower bound of current window
  */
  ScalarType m_LowerWindowBound;

  /*!
  * upper bound of current window
  */
  ScalarType m_UpperWindowBound;

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
  * if m_LowerWindowBound > m_UpperWindowBound, then the values for m_LowerWindowBound and m_UpperWindowBound will be exchanged
  *
  * if m_LowerWindowBound < m_RangeMin, m_LowerWindowBound will be set to m_RangeMin. m_UpperWindowBound will be decreased the same as m_LowerWindowBound will be increased, but minimum value for m_UpperWindowBound is also m_RangeMin.
  * 
  * if m_UpperWindowBound > m_RangeMax, m_UpperWindowBound will be set to m_RangeMax. m_LowerWindowBound will be increased the same as m_UpperWindowBound will be decreased, but maximum value for m_LowerWindowBound is also m_RangeMax.
  *
  */
  inline void testValues()
  {
    if ( m_LowerWindowBound > m_UpperWindowBound )
      std::swap(m_LowerWindowBound,m_UpperWindowBound);
    else if (m_LowerWindowBound == m_UpperWindowBound )
      m_LowerWindowBound = m_UpperWindowBound - 1;
    ScalarType diff;
    if ( m_LowerWindowBound < m_RangeMin )
    {
      diff = m_RangeMin - m_LowerWindowBound;
      m_LowerWindowBound = m_RangeMin;
      if (!((m_UpperWindowBound - diff) > m_RangeMin))
        m_UpperWindowBound = m_RangeMin + 1;
      else
        m_UpperWindowBound -= diff;
    }

    if ( m_UpperWindowBound > m_RangeMax )
    {
      diff = m_UpperWindowBound - m_RangeMax;
      m_UpperWindowBound = m_RangeMax;
      if (!((m_LowerWindowBound + diff) < m_RangeMax))
        m_LowerWindowBound = m_RangeMax - 1;
      else
        m_LowerWindowBound += diff;
    }
  }
  
};

} // namespace mitk
#endif /* LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C */
