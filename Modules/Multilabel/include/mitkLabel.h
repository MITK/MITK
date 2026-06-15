/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabel_h
#define mitkLabel_h

#include <MitkMultilabelExports.h>
#include <mitkColorProperty.h>
#include <mitkPropertyList.h>
#include <mitkPoint.h>
#include <mitkVector.h>
#include <mitkPropertyKeyPath.h>
#include <mitkDICOMCodeSequence.h>
#include <mitkDICOMCodeSequenceWithModifiers.h>

#include <optional>
#include <map>

namespace mitk
{
  /**
   * \brief Constants for property key path elements of properties of the label.
   */
  namespace LabelPropertyConstants
  {
    /**
     * \brief Gets the property key path element for anatomic region.
     * \return The string "anatomic_region"
     */
    MITKMULTILABEL_EXPORT const std::string& GetAnatomicRegionPropertyBaseName();

    /**
     * \brief Gets the property key path element for primary anatomic structure.
     * \return The string "primary_anatomic_structure"
     */
    MITKMULTILABEL_EXPORT const std::string& GetPrimaryAnatomicStructurePropertyBaseName();

    /**
     * \brief Gets the property key path element for segmented property category.
     * \return The string "segmented_property_category"
     */
    MITKMULTILABEL_EXPORT const std::string& GetSegmentedPropertyCategoryPropertyBaseName();

    /**
     * \brief Gets the property key path element for segmented property type.
     * \return The string "segmented_property_type"
     */
    MITKMULTILABEL_EXPORT const std::string& GetSegmentedPropertyTypePropertyBaseName();

    /**
     * \brief Gets the property key path element for modifier.
     * \return The string "modifier"
     */
    MITKMULTILABEL_EXPORT const std::string& GetModifierPropertySubName();

    /**
     * \brief Gets the property key path element for code value.
     * \return The string "value"
     */
    MITKMULTILABEL_EXPORT const std::string& GetValuePropertySubName();

    /**
     * \brief Gets the property key path element for code scheme.
     * \return The string "scheme"
     */
    MITKMULTILABEL_EXPORT const std::string& GetSchemePropertySubName();

    /**
     * \brief Gets the property key path element for code meaning.
     * \return The string "meaning"
     */
    MITKMULTILABEL_EXPORT const std::string& GetMeaningPropertySubName();
  }

  /**
   * \brief A data structure describing a label in a multi-label segmentation.
   *
   * Label is the central descriptor for labeled regions in a MultiLabelSegmentation.
   * It stores metadata such as name, color, visibility, lock state, opacity, and
   * DICOM code sequences (anatomic region, primary anatomic structure, segmented
   * property category/type). Label inherits from PropertyList, so all metadata is
   * stored as properties.
   *
   * Each label has a unique pixel value (PixelType) that identifies it within
   * the segmentation image. Labels can be locked to prevent overwriting and
   * can be set visible/invisible for rendering.
   *
   * \sa MultiLabelSegmentation, LabelSetImageHelper
   * \ingroup Data
   */
  class MITKMULTILABEL_EXPORT Label : public PropertyList
  {
  public:
    mitkClassMacro(Label, mitk::PropertyList);

    /** \brief Pixel type used for label values (unsigned short). */
    typedef unsigned short PixelType;

    itkNewMacro(Self);
    mitkNewMacro2Param(Self, PixelType, const std::string&);

    /** \brief The maximum value a label can have (65535 for unsigned short). */
    static const PixelType MAX_LABEL_VALUE;

    /** \brief Value indicating pixels that are not labeled at all (background). */
    static constexpr PixelType UNLABELED_VALUE = 0;

    /**
     * \brief Sets the lock state of the label.
     * \param[in] locked If true, the label is locked and its pixels cannot be overwritten
     *            by other operations that respect locks.
     */
    void SetLocked(bool locked);

    /**
     * \brief Returns the lock state of the label.
     * \return true if the label is locked, false otherwise. Default is true.
     */
    bool GetLocked() const;

    /**
     * \brief Sets the visibility state of the label for rendering.
     * \param[in] visible If true, the label is rendered visibly.
     */
    void SetVisible(bool visible);

    /**
     * \brief Returns the visibility state of the label.
     * \return true if the label is visible, false otherwise. Default is true.
     */
    bool GetVisible() const;

    /**
     * \brief Sets the opacity of the label for rendering.
     * \param[in] opacity Opacity value in [0.0, 1.0]. Default is 0.6.
     */
    void SetOpacity(float opacity);

    /**
     * \brief Returns the opacity of the label.
     * \return Opacity value in [0.0, 1.0].
     */
    float GetOpacity() const;

    /**
     * \brief Sets the (class) name of the label.
     * \param[in] name The name string, e.g. "Liver" or "Tumor".
     */
    void SetName(const std::string &name);

    /**
     * \brief Returns the (class) name of the label.
     * \return The label name string.
     */
    std::string GetName() const;

    /**
     * \brief Sets the DICOM tracking ID of the label.
     * \param[in] trackingID The tracking ID string.
     */
    void SetTrackingID(const std::string& trackingID);

    /**
     * \brief Returns the DICOM tracking ID of the label.
     * \return The tracking ID string.
     */
    std::string GetTrackingID() const;

    /**
     * \brief Returns whether a tracking ID property is set on the label.
     *
     * Distinguishes "property absent" from "property present but empty".
     * GetTrackingID returns "" in both cases. Persistence readers set the
     * property only when the source carried a non-empty value, so a true
     * result indicates a real tracking identifier on the source.
     */
    bool HasTrackingID() const;

    /**
     * \brief Sets the DICOM tracking unique identifier (UID) of the label.
     * \param[in] trackingUID The tracking UID string.
     */
    void SetTrackingUID(const std::string& trackingUID);

    /**
     * \brief Returns the DICOM tracking unique identifier (UID) of the label.
     * \return The tracking UID string.
     */
    std::string GetTrackingUID() const;

    /**
     * \brief Returns whether a tracking UID property is set on the label.
     *
     * Distinguishes "property absent" from "property present but empty"
     * (see HasTrackingID).
     * GetTrackingUID returns "" in both cases. Persistence readers set the
     * property only when the source carried a non-empty value, so a true
     * result indicates a real tracking UID on the source.
     */
    bool HasTrackingUID() const;

    /**
     * \brief Sets a textual description for the label.
     * \param[in] description The description string.
     */
    void SetDescription(const std::string& description);

    /**
     * \brief Returns the textual description of the label.
     * \return The description string.
     */
    std::string GetDescription() const;

    /**
     * \brief Returns the center of mass of the label as image index coordinates.
     * \return The center of mass in index coordinates.
     * \sa GetCenterOfMassCoordinates, UpdateCenterOfMass
     */
    mitk::Point3D GetCenterOfMassIndex() const;

    /**
     * \brief Returns the center of mass of the label in world coordinates.
     * \return The center of mass in world (physical) coordinates.
     * \sa GetCenterOfMassIndex, UpdateCenterOfMass
     */
    mitk::Point3D GetCenterOfMassCoordinates() const;

    /**
     * \brief Resets (clears) the center of mass properties.
     */
    void ResetCenterOfMass();

    /**
     * \brief Returns the modification time of the last center of mass property update.
     * \return The modification timestamp.
     */
    itk::ModifiedTimeType GetCenterOfMassMTime() const;

    /**
     * \brief Updates the center of mass with new index and world coordinates.
     * \param[in] index The center of mass in image index coordinates.
     * \param[in] coordinates The center of mass in world coordinates.
     */
    void UpdateCenterOfMass(const mitk::Point3D& index, const mitk::Point3D& coordinates);

    /**
     * \brief Sets the display color of the label.
     * \param[in] color The RGB color to use for rendering this label.
     */
    void SetColor(const mitk::Color & color);

    /**
     * \brief Returns the display color of the label.
     * \return Reference to the RGB color. Default is white (1, 1, 1).
     */
    const mitk::Color &GetColor() const;

    /**
     * \brief Sets the pixel value that identifies this label in the segmentation image.
     * \param[in] pixelValue The pixel value for this label.
     */
    void SetValue(PixelType pixelValue);

    /**
     * \brief Returns the pixel value that identifies this label in the segmentation image.
     * \return The pixel value of the label.
     */
    PixelType GetValue() const;

    /**
     * \brief Enumeration describing how a label was created or refined.
     */
    enum class AlgorithmType
    {
      Undefined = 0,   /**< Undefined/unknown algorithm type. */
      MANUAL,          /**< Label created manually by user with simple tools (e.g. Add tool). */
      SEMIAUTOMATIC,   /**< Label created or refined with algorithmic assistance (e.g. nnInteractive). */
      AUTOMATIC        /**< Label created fully automatically by an algorithm (e.g. nnUNet). */
    };

    /**
     * \brief Sets the algorithm type that was used to create or modify this label.
     * \param[in] algoType The algorithm type.
     */
    void SetAlgorithmType(AlgorithmType algoType);

    /**
     * \brief Sets the algorithm type from a string representation.
     * \param[in] algoType String representation ("MANUAL", "SEMIAUTOMATIC", "AUTOMATIC", or "Undefined").
     */
    void SetAlgorithmTypeStr(const std::string& algoType);

    /**
     * \brief Returns the algorithm type used to create or modify this label.
     * \return The AlgorithmType enum value.
     */
    AlgorithmType GetAlgorithmType() const;

    /**
     * \brief Returns the algorithm type as a string.
     * \return String representation of the algorithm type.
     */
    std::string GetAlgorithmTypeStr() const;

    /**
     * \brief Sets the name of the algorithm used to create or modify this label.
     * \param[in] algoName The algorithm name string.
     */
    void SetAlgorithmName(const std::string& algoName);

    /**
     * \brief Returns the name of the algorithm used to create or modify this label.
     * \return The algorithm name string.
     */
    std::string GetAlgorithmName() const;

    /**
     * \brief Returns whether an algorithm name property is set on the label.
     *
     * Distinguishes "property absent" from "property present" (GetAlgorithmName returns the
     * "MITK Segmentation" fallback in the absent case, so it cannot make this distinction).
     * Persistence readers and AddToolUse only set the property when there is a real name to record,
     * so a false result means no algorithm name was ever recorded for this label.
     */
    bool HasAlgorithmName() const;

    /** \brief Records that a tool/operation contributed to this label, updating algorithm type and name.
     *
     * Type: the first contribution to a still-Undefined label defines its type; any type already
     * present (a genuine MANUAL loaded from a DICOM SEG, or a prior tool's type) is preserved, and a
     * later tool of a different type mixes the result to SEMIAUTOMATIC.
     * Name: for internally-created labels, algorithm_name starts with the "MITK Segmentation" prefix;
     * the first dedicated tool is appended after ": " and further tools after "|"
     * (e.g. "MITK Segmentation: nnUNet|Paint"). A name still equal to just the prefix means no
     * dedicated tool has been recorded yet.
     *
     * \remark The encoding reserves the separators "|" and ": ". To keep the provenance string
     *      parseable, those characters are sanitized to "#" inside algoName (a warning is logged when
     *      this happens) rather than rejected; an empty algoName is ignored. Tool names should use
     *      plain spaces (e.g. "Boolean Union", "Morphological Closing", "Interpolation").
     * \remark Idempotent for repeated identical use (type stable, name de-duplicated), so it is safe
     *      under the many small writes a single interaction (e.g. a paint drag) produces.
     * \note Externally-loaded provenance names that do not carry the MITK prefix are kept as-is; a later
     *      in-MITK tool simply appends to them and mixes the type as usual.
     */
    void AddToolUse(AlgorithmType algoType, const std::string& algoName);

    /** \brief Absorbs another label's recorded provenance (algorithm type and tool names) into this one.
     *
     * Intended for content-merging operations (e.g. merging labels): the target should reflect that it
     * now contains the source's algorithmically-derived content. The source's type is mixed into this
     * label's type with the same rule as AddToolUse (a differing defined type yields SEMIAUTOMATIC), and
     * each tool name recorded on the source is appended (de-duplicated) to this label's tool chain.
     *
     * \param[in] other The source label whose provenance is absorbed. Must not be null.
     * \remark A source with type but no recorded tool name (e.g. a vendor MANUAL label) contributes only
     *      its type. A source with an Undefined type contributes nothing.
     */
    void MergeToolUses(const Label* other);

    /**
         * \brief Sets an anatomic region code at the specified index.
         * \param code The DICOMCodeSequenceWithModifiers representing the anatomic region
         * \param index The index at which to store the code (default: 0)
         */
    void SetAnatomicRegion(const DICOMCodeSequenceWithModifiers& code, std::size_t index = 0);

    /**
     * \brief Gets an anatomic region code at the specified index.
     * \param index The index of the code to retrieve (default: 0)
     * \return The DICOMCodeSequenceWithModifiers at the specified index, or an empty code if not found
     */
    DICOMCodeSequenceWithModifiers GetAnatomicRegion(std::size_t index = 0) const;

    /**
     * \brief Gets the number of anatomic region codes stored.
     * \return The count of anatomic region codes
     */
    std::size_t GetAnatomicRegionCount() const;

    /**
     * \brief Removes an anatomic region code at the specified index.
     * \param index The index of the code to remove
     */
    void RemoveAnatomicRegion(std::size_t index);

    /**
     * \brief Sets a primary anatomic structure code at the specified index.
     * \param code The DICOMCodeSequenceWithModifiers representing the primary anatomic structure
     * \param index The index at which to store the code (default: 0)
     */
    void SetPrimaryAnatomicStructure(const DICOMCodeSequenceWithModifiers& code, std::size_t index = 0);

    /**
     * \brief Gets a primary anatomic structure code at the specified index.
     * \param index The index of the code to retrieve (default: 0)
     * \return The DICOMCodeSequenceWithModifiers at the specified index, or an empty code if not found
     */
    DICOMCodeSequenceWithModifiers GetPrimaryAnatomicStructure(std::size_t index = 0) const;

    /**
     * \brief Gets the number of primary anatomic structure codes stored.
     * \return The count of primary anatomic structure codes
     */
    std::size_t GetPrimaryAnatomicStructureCount() const;

    /**
     * \brief Removes a primary anatomic structure code at the specified index.
     * \param index The index of the code to remove
     */
    void RemovePrimaryAnatomicStructure(std::size_t index);

    /**
     * \brief Sets the segmented property category code.
     * \param code The DICOMCodeSequence representing the segmented property category
     */
    void SetSegmentedPropertyCategory(const DICOMCodeSequence& code);

    /**
     * \brief Gets the segmented property category code.
     * \return The DICOMCodeSequence representing the segmented property category
     */
    std::optional<DICOMCodeSequence> GetSegmentedPropertyCategory() const;

    /**
     * \brief Sets the segmented property type code with optional modifiers.
     * \param code The DICOMCodeSequenceWithModifiers representing the segmented property type
     */
    void SetSegmentedPropertyType(const DICOMCodeSequenceWithModifiers& code);

    /**
     * \brief Gets the segmented property type code with modifiers.
     * \return The DICOMCodeSequenceWithModifiers representing the segmented property type
     */
    std::optional<DICOMCodeSequenceWithModifiers> GetSegmentedPropertyType() const;

    /**
     * \brief Sets a property on this label.
     * \param[in] propertyKey The key of the property.
     * \param[in] property Pointer to the property to set.
     * \param[in] contextName Optional context name (default: empty).
     * \param[in] fallBackOnDefaultContext If true, fall back to the default context (default: false).
     */
    void SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    /**
     * \brief Returns a const pointer to the property with the given key.
     * \param[in] propertyKey The key of the property.
     * \param[in] contextName Optional context name (default: empty).
     * \param[in] fallBackOnDefaultContext If true, fall back to the default context (default: true).
     * \return Const smart pointer to the property, or nullptr if not found.
     */
    BaseProperty::ConstPointer GetConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) const override;

    /**
     * \brief Returns a non-const pointer to the property with the given key.
     * \param[in] propertyKey The key of the property.
     * \param[in] contextName Optional context name (default: empty).
     * \param[in] fallBackOnDefaultContext If true, fall back to the default context (default: true).
     * \return Pointer to the property, or nullptr if not found.
     */
    BaseProperty* GetNonConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) override;

    using itk::Object::Modified;
    /** \brief Marks this label as modified. */
    void Modified() { Superclass::Modified(); }

    /** \brief Function updates the property values of a label provided as templates.
    If a property exists in the destination label only the value will be updated (therefor observers and pointers
    stay valid). If the property does not exist, it will be cloned and added. No properties will be removed.
    \param templateLabel Label instance that provides the new values for properties
    \param updateLabelValue Indicate if also the label value should be updated.
    */
    void Update(const Label* templateLabel, bool updateLabelValue = false);

    /**
     * \brief Default constructor. Creates a label with default properties.
     *
     * Defaults: locked=true, visible=true, opacity=0.6, color=white,
     * name="Unknown label name", value=UNLABELED_VALUE.
     */
    Label();

    /**
     * \brief Constructor that creates a label with a specified pixel value and name.
     * \param[in] value The pixel value for this label.
     * \param[in] name The name for this label.
     */
    Label(PixelType value, const std::string& name);

    /** \brief Destructor. */
    ~Label() override;

  protected:
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;
    using PropertyList::GetProperty;

    Label(const Label &other);

    mitkCloneMacro(Label);

    void SetCenterOfMassIndex(const mitk::Point3D& center);
    void SetCenterOfMassCoordinates(const mitk::Point3D& center);

  private:
    PixelType m_Value;

    /**
     * \brief Helper function to set a DICOMCode as properties.
     * \param basePath The base property key path
     * \param code The DICOMCode to store
     * \param withModifiers If true, also store modifiers (code must be DICOMCodeSequenceWithModifiers)
     */
    void SetDICOMCodeSequenceAsProperties(const PropertyKeyPath& basePath,
      const DICOMCodeSequence& code,
      bool withModifiers);

    /**
     * \brief Helper function to get a DICOMCodeSequence from properties.
     * \param basePath The base property key path
     * \return The DICOMCodeSequence retrieved from properties
     */
    std::optional<DICOMCodeSequence> GetDICOMCodeSequenceFromProperties(const PropertyKeyPath& basePath) const;

    /**
     * \brief Helper function to get a DICOMCodeSequenceWithModifiers from properties.
     * \param basePath The base property key path
     * \return The DICOMCodeSequenceWithModifiers retrieved from properties including modifiers
     */
    std::optional<DICOMCodeSequenceWithModifiers> GetDICOMCodeSequenceWithModifiersFromProperties(const PropertyKeyPath& basePath) const;

    /**
     * \brief Helper function to remove DICOMCodeSequence properties.
     * \param basePath The base property key path
     */
    void RemoveDICOMCodeSequenceProperties(const PropertyKeyPath& basePath);
  };

  /** \brief Type alias for a vector of mutable Label smart pointers. */
  using LabelVector = std::vector<Label::Pointer>;

  /** \brief Type alias for a vector of const Label smart pointers. */
  using ConstLabelVector = std::vector<Label::ConstPointer>;

  /**
  * \brief Equal A function comparing two labels for being equal in data
  *
  * \ingroup MITKTestingAPI
  *
  * Following aspects are tested for equality:
  *  - Lebel equality via Equal-PropetyList
  *
  * \param rightHandSide An image to be compared
  * \param leftHandSide An image to be compared
  * \param eps Tolarence for comparison. You can use mitk::eps in most cases.
  * \param verbose Flag indicating if the user wants detailed console output or not.
  * \return true, if all subsequent comparisons are true, false otherwise
  */
  MITKMULTILABEL_EXPORT bool Equal(const mitk::Label &leftHandSide,
                                   const mitk::Label &rightHandSide,
                                   ScalarType eps,
                                   bool verbose);

} // namespace mitk

#endif
