/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkLookupTableProperty_h
#define mitkLookupTableProperty_h

#include <mitkBaseProperty.h>
#include <mitkLookupTable.h>
#include <MitkCoreExports.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Property to associate a mitk::LookupTable with a DataNode.
   *
   * Stores a smart pointer to a LookupTable, which defines a color mapping
   * for rendering images with discrete or continuous color scales.
   *
   * \ingroup DataManagement
   *
   * \note If you want to use this property to colorize an mitk::Image, make sure
   * to set the mitk::RenderingModeProperty to a mode which supports lookup tables
   * (e.g. LOOKUPTABLE_COLOR). See the documentation of mitk::RenderingModeProperty.
   * For a code example see mitkImageVtkMapper2DLookupTableTest.cpp in
   * Core/Code/Testing.
   *
   * \sa BaseProperty
   * \sa LookupTable
   * \sa RenderingModeProperty
   */
  class MITKCORE_EXPORT LookupTableProperty : public BaseProperty
  {
  protected:
    LookupTable::Pointer m_LookupTable;

    LookupTableProperty();

    LookupTableProperty(const LookupTableProperty &);

    LookupTableProperty(const mitk::LookupTable::Pointer lut);

    mitkCloneMacro(LookupTableProperty);

  public:
    /** \brief The type of the value stored by this property. */
    typedef LookupTable::Pointer ValueType;

    mitkClassMacro(LookupTableProperty, BaseProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
      mitkNewMacro1Param(LookupTableProperty, const mitk::LookupTable::Pointer);

    /**
     * \brief Get the lookup table (ITK macro-generated accessor).
     * \return A pointer to the stored LookupTable.
     */
    itkGetObjectMacro(LookupTable, LookupTable);

    /**
     * \brief Get the lookup table as a smart pointer.
     * \return The stored LookupTable smart pointer.
     */
    ValueType GetValue() const;

    /**
     * \brief Set the lookup table.
     *
     * Marks the property as modified if the new table differs from the current one.
     *
     * \param[in] aLookupTable The new LookupTable to set.
     */
    void SetLookupTable(const mitk::LookupTable::Pointer aLookupTable);

    /**
     * \brief Set the lookup table (alias for SetLookupTable()).
     *
     * Takes the new LookupTable smart pointer to set.
     */
    void SetValue(const ValueType &);

    /**
     * \brief Return a summary string of the lookup table.
     *
     * Includes the number of colors, the table range, and the first few RGBA entries.
     *
     * \return A string representation of the lookup table.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Serialize the lookup table to JSON.
     *
     * Serializes all VTK lookup table parameters including scale, ramp, ranges,
     * and the full RGBA color table.
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the lookup table from JSON.
     *
     * \param[in] j The JSON object containing lookup table data.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  private:
    // purposely not implemented
    LookupTableProperty &operator=(const LookupTableProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif
} // namespace mitk

#endif
