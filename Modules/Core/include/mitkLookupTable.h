/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkLookupTable_h
#define mitkLookupTable_h

#include <mitkCommon.h>
#include <MitkCoreExports.h>

#include <itkDataObject.h>
#include <itkObjectFactory.h>

#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class vtkColorTransferFunction;
class vtkPiecewiseFunction;

namespace mitk
{
  /**
   * \brief MITK wrapper for a vtkLookupTable.
   *
   * Wraps a vtkLookupTable and provides convenience methods for creating
   * predefined color lookup tables (grayscale, hot iron, jet, PET, etc.),
   * modifying opacity values, and querying color mappings.
   *
   * \note To use this as an image property, wrap it in a
   *       mitk::LookupTableProperty and set the mitk::RenderingModeProperty
   *       to a mode that supports lookup tables (e.g. LOOKUPTABLE_COLOR).
   *       See the documentation of mitk::RenderingModeProperty and the test
   *       mitkImageVtkMapper2DLookupTableTest.cpp for examples.
   *
   * \sa LookupTableProperty, RenderingModeProperty, TransferFunction
   * \ingroup DataManagement
   */
  class MITKCORE_EXPORT LookupTable : public itk::DataObject
  {
  public:
    /**
     * \brief Convenience typedef for the raw lookup table element type.
     */
    typedef unsigned char RawLookupTableType;

    mitkClassMacroItkParent(LookupTable, itk::DataObject);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Get the internally wrapped vtkLookupTable.
     * \return Smart pointer to the vtkLookupTable.
     */
    virtual vtkSmartPointer<vtkLookupTable> GetVtkLookupTable() const;

    /**
     * \brief Get the raw lookup table array from the wrapped vtkLookupTable.
     * \return Pointer to the raw RGBA byte array, or nullptr if not built.
     */
    virtual RawLookupTableType *GetRawLookupTable() const;

    /**
     * \brief Set the internal vtkLookupTable.
     * \param[in] lut The vtkLookupTable to wrap.
     */
    virtual void SetVtkLookupTable(vtkSmartPointer<vtkLookupTable> lut);

    /**
     * \brief Set the opacity (alpha) for all entries in the table.
     * \param[in] opacity Opacity value between 0.0 (transparent) and 1.0 (opaque).
     */
    virtual void ChangeOpacityForAll(float opacity);

    /**
     * \brief Set the opacity (alpha) for a specific table entry.
     * \param[in] index   Zero-based index into the lookup table.
     * \param[in] opacity Opacity value between 0.0 (transparent) and 1.0 (opaque).
     */
    virtual void ChangeOpacity(int index, float opacity);

    /**
     * \brief Map a scalar value to an RGB color via the lookup table.
     *
     * Wraps vtkLookupTable::GetColor().
     *
     * \param[in]  value The scalar value to map.
     * \param[out] rgb   Array of 3 doubles filled with RGB values in [0, 1].
     */
    virtual void GetColor(double value, double rgb[3]);

    /**
     * \brief Number of curated colors in the MULTILABEL palette.
     *
     * They occupy lookup-table slots 1..N (slot 0 is the transparent
     * background); higher slots hold algorithmically generated colors.
     * Derived from the color table, so callers never hardcode the count.
     */
    static int GetMultiLabelColorCount();

    /**
     * \brief RGB of the index-th MULTILABEL color, in [0,1].
     *
     * Indices 0..GetMultiLabelColorCount()-1 are the curated palette;
     * higher indices are algorithmically generated. The MULTILABEL lookup
     * table and the label-color selection both draw their colors from here,
     * so callers do not have to round-trip through the table.
     */
    static void GetMultiLabelColor(int index, double rgb[3]);

    /**
     * \brief Get the RGBA tuple at a specific table index.
     *
     * Wraps vtkLookupTable::GetTableValue().
     *
     * \param[in]  index Zero-based index into the table.
     * \param[out] rgba  Array of 4 doubles filled with RGBA values in [0, 1].
     */
    virtual void GetTableValue(int index, double rgba[4]);

    /**
     * \brief Set the RGBA tuple at a specific table index.
     *
     * Wraps vtkLookupTable::SetTableValue().
     *
     * \param[in] index Zero-based index into the table.
     * \param[in] rgba  Array of 4 doubles with RGBA values in [0, 1].
     */
    virtual void SetTableValue(int index, double rgba[4]);

    itkSetMacro(Window, float);
    itkSetMacro(Level, float);
    itkSetMacro(Opacity, float);

    /**
     * \brief Compare two LookupTable objects for equality.
     * \param[in] LookupTable The LookupTable to compare against.
     * \return True if both tables contain the same entries.
     */
    virtual bool operator==(const mitk::LookupTable &LookupTable) const;

    /**
     * \brief Compare two LookupTable objects for inequality.
     * \param[in] LookupTable The LookupTable to compare against.
     * \return True if the tables differ.
     */
    virtual bool operator!=(const LookupTable &LookupTable) const;

    /**
     * \brief Copy assignment operator.
     *
     * Necessary because itk::Object makes operator= private.
     *
     * \param[in] LookupTable The LookupTable to copy from.
     * \return Reference to this object.
     */
    virtual LookupTable &operator=(const LookupTable &LookupTable);

    /**
     * \brief Update output information by querying the source object.
     */
    void UpdateOutputInformation() override;

    /**
     * \brief Set the requested region to the largest possible region.
     *
     * No-op for lookup tables since the requested-region mechanism
     * is not supported.
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Check whether the requested region is outside the buffered region.
     * \return The result of VerifyRequestedRegion() negated.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Check whether the requested region is valid.
     *
     * Always returns true because lookup tables are always processed as a whole.
     *
     * \return True.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Set the requested region from another data object.
     *
     * No-op for lookup tables since the region mechanism is not supported.
     *
     * \param[in] data The data object whose region would be copied (ignored).
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /** \brief Default constructor. Builds a GRAYSCALE table by default. */
    LookupTable();
    /** \brief Destructor. */
    ~LookupTable() override;

    /**
     * \brief Create a vtkColorTransferFunction from this lookup table.
     * \return Smart pointer to the new color transfer function.
     */
    vtkSmartPointer<vtkColorTransferFunction> CreateColorTransferFunction();

    /**
     * \brief Create a vtkPiecewiseFunction for opacity from this lookup table.
     * \return Smart pointer to the new opacity transfer function.
     */
    vtkSmartPointer<vtkPiecewiseFunction> CreateOpacityTransferFunction();

    /**
     * \brief Create a vtkPiecewiseFunction for gradient opacity from this lookup table.
     * \return Smart pointer to the new gradient transfer function.
     */
    vtkSmartPointer<vtkPiecewiseFunction> CreateGradientTransferFunction();

    /**
     * @brief The LookupTableType enum for different predefined lookup tables.
     *
     * \li GRAYSCALE Our default level-window (sometimes referred to as window-level by other sources) setup for a test
     * image looks like this:
     * \image html ExampleLevelWindowColor.png
     * \li INVERSE_GRAYSCALE Inverse LookupTable of GRAYSCALE.
     * \li HOT_IRON A LookupTable for red colors.
     * \li JET A LookupTable for JET color rendering.
     * \li LEGACY_BINARY A LookupTable for binary images.
     * \li LEGACY_RAINBOW_COLOR A rainbow-like LookupTable.
     * \li MULTILABEL A LookupTable for multilabel images.
     * \li PET_COLOR A LookupTable for PET color rendering.
     * \li PET_20 A LookupTable for PET_20 color rendering.
     * \li PET_BLACK_WHITE A LookupTable for PET white to black color rendering.
     * \li TURBO An improved rainbow LookupTable for visualization.
     *
     * The different LookupTableTypes can be applied in the MitkWorkbench via right-clicking
     * on an image and choosing a color map.
     */
    enum LookupTableType
    {
      GRAYSCALE,
      INVERSE_GRAYSCALE,
      HOT_IRON,
      JET,
      JET_TRANSPARENT,
      PLASMA,
	    INFERNO,
	    VIRIDIS,
      MAGMA,
      LEGACY_BINARY,
      LEGACY_RAINBOW_COLOR,
      MULTILABEL,
      PET_COLOR,
      PET_20,
      PET_BLACK_WHITE,
      TURBO
    };

    /** \brief List of all predefined lookup table type names. */
    static std::vector<std::string> typenameList;

    /**
     * \brief Set the lookup table type by enum value.
     *
     * Rebuilds the internal vtkLookupTable for the given type. Does nothing
     * if the type is already active or if the type is not recognized.
     *
     * \param[in] type The desired LookupTableType enum value.
     */
    virtual void SetType(const LookupTableType type);

    /**
     * \brief Set the lookup table type by name string.
     *
     * Looks up \p typeName in typenameList and delegates to SetType(LookupTableType).
     * Does nothing if the name is not found.
     *
     * \param[in] typeName Case-sensitive name of the lookup table type.
     */
    virtual void SetType(const std::string &typeName);

    /**
     * \brief Get the currently active lookup table type.
     * \return The active LookupTableType enum value.
     */
    virtual LookupTableType GetActiveType() const;

    /**
     * \brief Get the currently active lookup table type as a string.
     * \return The name of the active type.
     */
    virtual std::string GetActiveTypeAsString() const;

  protected:
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    LookupTable(const Self &other);

    mitkCloneMacro(Self);

    virtual void BuildGrayScaleLookupTable();
    virtual void BuildLegacyBinaryLookupTable();
    virtual void BuildLegacyRainbowColorLookupTable();
    virtual void BuildInverseGrayScaleLookupTable();
    virtual void BuildHotIronLookupTable();
    virtual void BuildPlasmaLookupTable();
    virtual void BuildInfernoLookupTable();
    virtual void BuildViridisLookupTable();
    virtual void BuildMagmaLookupTable();
    virtual void BuildJetLookupTable(bool transparent = false);
    virtual void BuildPETColorLookupTable();
    virtual void BuildPET20LookupTable();
    virtual void BuildPETBWLookupTable();
    virtual void BuildMultiLabelLookupTable();
    virtual void BuildTurboLookupTable();

    vtkSmartPointer<vtkLookupTable> m_LookupTable;

    float m_Window;
    float m_Level;
    float m_Opacity;

    LookupTableType m_Type;

  };
} // namespace mitk

#endif
