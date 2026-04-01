/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIsoDoseLevel_h
#define mitkIsoDoseLevel_h

#include <itkRGBPixel.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

#include <mitkCommon.h>
#include <mitkDoseValueType.h>
#include <MitkRTExports.h>

namespace mitk
{

  /**
   * \brief Stores values needed for the representation/visualization of dose iso levels.
   *
   * The dose iso level is defined in the topology of a dose distribution by the dose value
   * that indicates the lower boundary of the iso level. The upper boundary is implicitly defined
   * by the next IsoDoseLevel greater (higher dose value) than the current iso level.
   * Color and the visibility options are used to indicate the visualization style.
   *
   * \sa IsoDoseLevelSet
   * \sa IsoDoseLevelVector
   * \sa IsoDoseLevelSetProperty
   * \ingroup MitkRTModule
   */
  class MITKRT_EXPORT IsoDoseLevel: public itk::Object
  {
  public:
    /** \brief RGB color type using float components in [0,1]. */
    typedef ::itk::RGBPixel<float> ColorType;

    /** \brief Type for the relative dose value of this iso level. */
    typedef DoseValueRel DoseValueType;

    mitkClassMacroItkParent(IsoDoseLevel, itk::Object);
    itkNewMacro(Self);
    mitkNewMacro4Param(Self,DoseValueType, ColorType, bool, bool);

    /**
     * \brief Greater-than comparison based on dose values.
     * \param[in] right The iso dose level to compare against.
     * \return True if this level's dose value is greater than \p right's dose value.
     */
    bool operator> ( const IsoDoseLevel& right ) const;

    /**
     * \brief Less-than comparison based on dose values.
     * \param[in] right The iso dose level to compare against.
     * \return True if this level's dose value is less than \p right's dose value.
     */
    bool operator< ( const IsoDoseLevel& right ) const;

    /**
     * \brief Equality comparison. Checks dose value, color, and both visibility flags.
     * \param[in] right The iso dose level to compare against.
     * \return True if all fields (dose value, color, iso line visibility, color wash visibility) are equal.
     */
    bool operator == ( const IsoDoseLevel& right) const;

    /** \brief Set the relative dose value (fraction of reference dose). */
    itkSetMacro(DoseValue,DoseValueType);
    /** \brief Get the relative dose value (fraction of reference dose). */
    itkGetConstMacro(DoseValue,DoseValueType);

    /** \brief Set the RGB display color for this iso level. */
    itkSetMacro(Color,ColorType);
    /** \brief Get the RGB display color for this iso level. */
    itkGetConstMacro(Color,ColorType);

    /** \brief Set whether the iso line should be visible for this level. */
    itkSetMacro(VisibleIsoLine,bool);
    /** \brief Get whether the iso line is visible for this level. */
    itkGetConstMacro(VisibleIsoLine,bool);
    /** \brief Toggle iso line visibility. */
    itkBooleanMacro(VisibleIsoLine);

    /** \brief Set whether the color wash should be visible for this level. */
    itkSetMacro(VisibleColorWash,bool);
    /** \brief Get whether the color wash is visible for this level. */
    itkGetConstMacro(VisibleColorWash,bool);
    /** \brief Toggle color wash visibility. */
    itkBooleanMacro(VisibleColorWash);

  protected:
    IsoDoseLevel();
    IsoDoseLevel(const IsoDoseLevel & other);
    IsoDoseLevel(const DoseValueType & value, const ColorType& color, bool visibleIsoLine = true, bool visibleColorWash = true );
    ~IsoDoseLevel() override;

    mitkCloneMacro(IsoDoseLevel);

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  private:
    /** Relative dose value and lower boundary of the iso level instance
     */
    DoseValueType m_DoseValue;

    /** RGB color code that should be used for the iso level.*/
    ColorType m_Color;

    /** indicates if an iso line should be shown for the iso level
     (the lower boundary indicated by m_DoseValue)*/
    bool m_VisibleIsoLine;

    /** indicates if a color wash should be shown for the iso level.*/
    bool m_VisibleColorWash;

    /** Not implemented on purpose*/
    IsoDoseLevel& operator = (const IsoDoseLevel& source);
  };

} // namespace mitk

#endif
