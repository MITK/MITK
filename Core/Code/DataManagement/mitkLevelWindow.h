/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C
#define LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C

#include "mitkVector.h"
#include <MitkCoreExports.h>

namespace mitk {

class Image;

/**
 * @brief The LevelWindow class Class to store level/window values.
 *
 * Current min and max value are stored in m_LowerWindowBound and m_UpperWindowBound.
 * The maximum and minimum of valid value range is stored in m_RangeMin and m_RangeMax.
 * m_DefaultLevel amd m_DefaultWindow store the initial Level/Window values for the image.
 * m_DefaultRangeMin and m_DefaultRangeMax store the initial minrange and maxrange for the image.
 *
 * See documentation of SetAuto for information on how the level window is initialized from an image.
 *
 * @ingroup DataManagement
 *
 * @note If you want to apply the mitk::LevelWindow to an mitk::Image, make sure
 * to use the mitk::LevelWindowProperty and set the mitk::RenderingModeProperty
 * to a mode which supports level window (e.g. LEVELWINDOW_COLOR).
 * Make sure to check the documentation of the mitk::RenderingModeProperty. For a
 * code example how to use the mitk::LevelWindowProperty check the
 * mitkImageVtkMapper2DLevelWindowTest.cpp in Core\Code\Testing.
 */
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
  void SetLevelWindow(ScalarType level, ScalarType window, bool expandRangesIfNecessary = true);

  /*!
  * Set the lower and upper bound of the window
  */
  void SetWindowBounds(ScalarType lowerBound, ScalarType upperBound, bool expandRangesIfNecessary = true);

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
  ScalarType GetDefaultLowerBound() const;

  /*!
  * Get the default range maximum value
  */
  ScalarType GetDefaultUpperBound() const;

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
  * set the default Bounderies
  */
  void SetDefaultBoundaries(ScalarType low, ScalarType up);

  /**!
  * \brief sets level/window to the min/max greyvalues of the given Image
  */
  void SetAuto(const Image* image, bool tryPicTags = true, bool guessByCentralSlice = true);

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
  ScalarType m_DefaultLowerBound;

  /*!
  * default maximum gray value of the window
  */
  ScalarType m_DefaultUpperBound;


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
  inline void EnsureConsistency();

};

} // namespace mitk
#endif /* LEVELWINDOW_H_HEADER_INCLUDED_C1F4F02C */
