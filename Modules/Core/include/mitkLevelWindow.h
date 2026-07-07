/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkLevelWindow_h
#define mitkLevelWindow_h

#include <mitkNumericTypes.h>
#include <MitkCoreExports.h>
#include <nlohmann/json_fwd.hpp>

namespace mitk
{
  class Image;

  /**
  * @brief The LevelWindow class Class to store level/window values.
  *
  * Current min and max value are stored in m_LowerWindowBound and m_UpperWindowBound.
  * m_DefaultLevel amd m_DefaultWindow store the initial Level/Window values for the image.
  * m_DefaultRangeMin and m_DefaultRangeMax store the initial minrange and maxrange for the image.
  *
  * The finite maximum and minimum of valid value range is stored in m_RangeMin and m_RangeMax.
  * If deduced from an image by default the minimum or maximum of it statistics is used. If one
  * of these values are infinite the 2nd extrimum (which is guaranteed to be finite), will be used.
  *
  * See documentation of SetAuto for information on how the level window is initialized from an image.
  *
  * @note If you want to apply the mitk::LevelWindow to an mitk::Image, make sure
  * to use the mitk::LevelWindowProperty and set the mitk::RenderingModeProperty
  * to a mode which supports level window (e.g. LOOKUPTABLE_LEVELWINDOW_COLOR).
  * Make sure to check the documentation of the mitk::RenderingModeProperty. For a
  * code example how to use the mitk::LevelWindowProperty check the
  * mitkImageVtkMapper2DLevelWindowTest.cpp in Core/Code/Testing.
  */
  class MITKCORE_EXPORT LevelWindow
  {
  public:
    /**
     * \brief Construct a LevelWindow with given level (center) and window (range) values.
     * \param[in] level The center of the grey value interval (default: 127.5).
     * \param[in] window The width of the grey value interval (default: 255.0).
     */
    LevelWindow(ScalarType level = 127.5, ScalarType window = 255.0);

    /**
     * \brief Copy constructor.
     * \param[in] levWin The LevelWindow to copy.
     */
    LevelWindow(const mitk::LevelWindow &levWin);

    /** \brief Virtual destructor. */
    virtual ~LevelWindow();

    /**
     * \brief Get the level value, i.e. the center of the current grey value interval.
     * \return The level (center) value.
     */
    ScalarType GetLevel() const;

    /**
     * \brief Get the current window size, i.e. the width of the current grey value interval.
     * \return The window (range) size.
     */
    ScalarType GetWindow() const;

    /**
     * \brief Get the default level value for the image.
     * \return The default level value.
     */
    ScalarType GetDefaultLevel() const;

    /**
     * \brief Get the default window size for the image.
     * \return The default window size.
     */
    ScalarType GetDefaultWindow() const;

    /**
     * \brief Reset the level and window to their default values.
     */
    void ResetDefaultLevelWindow();

    /**
     * \brief Get the lower bound of the current window.
     * \return The lower window bound.
     */
    ScalarType GetLowerWindowBound() const;

    /**
     * \brief Get the upper bound of the current window.
     * \return The upper window bound.
     */
    ScalarType GetUpperWindowBound() const;

    /**
     * \brief Set the level and window values.
     * \param[in] level The center of the grey value interval.
     * \param[in] window The width of the grey value interval.
     * \param[in] expandRangesIfNecessary If true, the range is expanded to fit the new window.
     */
    void SetLevelWindow(ScalarType level, ScalarType window, bool expandRangesIfNecessary = true);

    /**
     * \brief Set the lower and upper window bounds directly.
     *
     * Values are clamped to the range [-10^300, 10^300].
     *
     * \param[in] lowerBound The lower window bound.
     * \param[in] upperBound The upper window bound.
     * \param[in] expandRangesIfNecessary If true, the range is expanded to fit the new bounds.
     */
    void SetWindowBounds(ScalarType lowerBound, ScalarType upperBound, bool expandRangesIfNecessary = true);

    /**
     * \brief Set the window to cover the entire valid range.
     */
    void SetToMaxWindowSize();

    /**
     * \brief Set the minimum and maximum of the valid value range.
     * \param[in] min The range minimum.
     * \param[in] max The range maximum.
     */
    void SetRangeMinMax(ScalarType min, ScalarType max);

    /**
     * \brief Get the minimum of the valid value range.
     * \return The range minimum.
     */
    ScalarType GetRangeMin() const;

    /**
     * \brief Get the maximum of the valid value range.
     * \return The range maximum.
     */
    ScalarType GetRangeMax() const;

    /**
     * \brief Get the default lower bound (default range minimum).
     * \return The default lower bound.
     */
    ScalarType GetDefaultLowerBound() const;

    /**
     * \brief Get the default upper bound (default range maximum).
     * \return The default upper bound.
     */
    ScalarType GetDefaultUpperBound() const;

    /**
     * \brief Reset the range min/max to the default boundaries.
     */
    void ResetDefaultRangeMinMax();

    /**
     * \brief Get the total size of the grey value range (max - min).
     * \return The range size.
     */
    ScalarType GetRange() const;

    /**
     * \brief Set the default level and window values.
     * \param[in] level The default level (center) value.
     * \param[in] window The default window (range) size.
     */
    void SetDefaultLevelWindow(ScalarType level, ScalarType window);

    /**
     * \brief Set the default lower and upper boundaries.
     * \param[in] low The default lower bound.
     * \param[in] up The default upper bound.
     */
    void SetDefaultBoundaries(ScalarType low, ScalarType up);

    /**
     * \brief Automatically set level/window to optimize image contrast.
     *
     * Analyzes the image statistics (optionally from the central slice) to
     * determine appropriate level/window settings that maximize contrast.
     * Handles special cases like binary images, uniform images, and images
     * with outlier values.
     *
     * \param[in] image The image to analyze.
     * \param[in] tryPicTags Currently unused, reserved for backward compatibility.
     * \param[in] guessByCentralSlice If true, analyze only the central slice first for performance.
     * \param[in] selectedComponent The image component to analyze (for multi-component images).
     */
    void SetAuto(const Image *image,
                 bool tryPicTags = true,
                 bool guessByCentralSlice = true,
                 unsigned selectedComponent = 0);

    /**
     * \brief Set level/window to the full min/max grey value range of the given image.
     * \param[in] image The image whose value range determines the level/window.
     */
    void SetToImageRange(const Image *image);

    /**
     * \brief Lock or unlock the level window settings.
     *
     * When fixed, all Set methods will be no-ops.
     *
     * \param[in] fixed True to lock settings, false to allow modifications.
     */
    void SetFixed(bool fixed);

    /**
     * \brief Return whether the level window settings are fixed.
     * \return True if fixed, false otherwise.
     * \sa SetFixed
     */
    bool GetFixed() const;

    /**
     * \brief Return whether the level window settings are fixed.
     * \return True if fixed, false otherwise.
     * \sa SetFixed
     */
    bool IsFixed() const;

    /**
     * \brief Compare two LevelWindow objects for equality.
     * \param[in] levWin The LevelWindow to compare with.
     * \return True if all settings match within epsilon tolerance.
     */
    virtual bool operator==(const LevelWindow &levWin) const;

    /**
     * \brief Compare two LevelWindow objects for inequality.
     * \param[in] levWin The LevelWindow to compare with.
     * \return True if any setting differs.
     */
    virtual bool operator!=(const LevelWindow &levWin) const;

    /**
     * \brief Assignment operator.
     * \param[in] levWin The LevelWindow to copy from.
     * \return Reference to this LevelWindow.
     */
    virtual LevelWindow &operator=(const LevelWindow &levWin);

    /**
     * \brief Check whether the image uses floating-point values.
     * \return True if the image has float or double pixel type.
     */
    bool IsFloatingValues() const;

    /**
     * \brief Set whether the image uses floating-point values.
     * \param[in] value True for floating-point images, false otherwise.
     */
    void SetFloatingValues(bool value);

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
    * Image with floating values
    */
    bool m_IsFloatingImage;

    /*!
    * Defines whether the level window settings may be changed after
    * initialization or not.
    */
    bool m_Fixed;

    /*!
    * confidence tests
    *
    * if m_LowerWindowBound > m_UpperWindowBound, then the values for m_LowerWindowBound and m_UpperWindowBound will be
    * exchanged
    *
    * if m_LowerWindowBound < m_RangeMin, m_LowerWindowBound will be set to m_RangeMin. m_UpperWindowBound will be
    * decreased the same as m_LowerWindowBound will be increased, but minimum value for m_UpperWindowBound is also
    * m_RangeMin.
    *
    * if m_UpperWindowBound > m_RangeMax, m_UpperWindowBound will be set to m_RangeMax. m_LowerWindowBound will be
    * increased the same as m_UpperWindowBound will be decreased, but maximum value for m_LowerWindowBound is also
    * m_RangeMax.
    *
    */
    inline void EnsureConsistency();
  };

  MITKCORE_EXPORT void to_json(nlohmann::json& j, const LevelWindow& lw);
  MITKCORE_EXPORT void from_json(const nlohmann::json& j, LevelWindow& lw);

} // namespace mitk
#endif
