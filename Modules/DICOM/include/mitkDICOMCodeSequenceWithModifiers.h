/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMCodeSequenceWithModifiers_h
#define mitkDICOMCodeSequenceWithModifiers_h


#include "mitkDICOMCodeSequence.h"

#include <vector>

#include <MitkDICOMExports.h>

namespace mitk
{
  /**
   * @brief Represents a DICOM Code Sequence with optional modifiers.
   *
   * This class extends DICOMCodeSequence to include a sequence of modifier codes,
   * similar to DCMTK's CodeWithModifiers class. It provides a simplified
   * interface for storing a primary code along with its modifying codes.
   */
  class MITKDICOM_EXPORT DICOMCodeSequenceWithModifiers : public DICOMCodeSequence
  {
  public:
    using ModifierVector = std::vector<DICOMCodeSequence>;

    DICOMCodeSequenceWithModifiers() = default;

    /**
     * @brief Constructs a DICOMCodeSequenceWithModifiers with the specified attributes.
     * @param value The Code Value (0008,0100), Long Code Value (0008,0119), or URN Code Value (0008,0120)
     * @param scheme The Coding Scheme Designator (0008,0102)
     * @param meaning The Code Meaning (0008,0104)
     */
    DICOMCodeSequenceWithModifiers(const std::string& value, const std::string& scheme, const std::string& meaning);

    /**
     * @brief Constructs from a DICOMCodeSequence.
     * @param code The base DICOMCodeSequence
     */
    explicit DICOMCodeSequenceWithModifiers(const DICOMCodeSequence& code);

    ~DICOMCodeSequenceWithModifiers() override = default;

    DICOMCodeSequenceWithModifiers(const DICOMCodeSequenceWithModifiers&) = default;
    DICOMCodeSequenceWithModifiers& operator=(const DICOMCodeSequenceWithModifiers&) = default;
    DICOMCodeSequenceWithModifiers(DICOMCodeSequenceWithModifiers&&) = default;
    DICOMCodeSequenceWithModifiers& operator=(DICOMCodeSequenceWithModifiers&&) = default;

    /**
     * @brief Adds a modifier code.
     * @param modifier The modifier code to add
     */
    void AddModifier(const DICOMCodeSequence& modifier);

    /**
     * @brief Gets all modifier codes.
     * @return Const reference to the vector of modifiers
     */
    const ModifierVector& GetModifiers() const;

    /**
     * @brief Gets a modifier code at the specified index.
     * @param index The index of the modifier to retrieve
     * @return Const reference to the modifier at the given index
     * @pre index must be valid
     */
    const DICOMCodeSequence& GetModifier(std::size_t index) const;

    /**
     * @brief Gets a modifier code at the specified index.
     * @param index The index of the modifier to retrieve
     * @return Reference to the modifier at the given index
     * @pre index must be valid
     */
    DICOMCodeSequence& GetModifier(std::size_t index);

    /**
     * @brief Sets a modifier code at the specified index.
     * @param index The index where to set the modifier
     * @param modifier The modifier code to set
     * @pre index must be valid
     */
    void SetModifier(std::size_t index, const DICOMCodeSequence& modifier);

    /**
     * @brief Sets all modifier codes.
     * @param modifiers The vector of modifiers to set
     */
    void SetModifiers(const ModifierVector& modifiers);

    /**
     * @brief Clears all modifier codes.
     */
    void ClearModifiers();

    /**
     * @brief Gets the number of modifiers.
     * @return The count of modifier codes
     */
    std::size_t GetModifierCount() const;

    /**
     * @brief Checks if there are any modifiers.
     * @return true if at least one modifier exists, false otherwise
     */
    bool HasModifiers() const;

    /**
     * @brief Equality comparison operator.
     * @param other The DICOMCodeSequenceWithModifiers to compare with
     * @return true if base code and all modifiers are equal
     */
    bool operator==(const DICOMCodeSequenceWithModifiers& other) const;

    /**
     * @brief Inequality comparison operator.
     * @param other The DICOMCodeSequenceWithModifiers to compare with
     * @return true if base code or modifiers differ
     */
    bool operator!=(const DICOMCodeSequenceWithModifiers& other) const;

  private:
    ModifierVector m_Modifiers;
  };

} // namespace mitk

#endi
