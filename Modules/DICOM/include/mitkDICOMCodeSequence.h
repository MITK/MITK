/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMCodeSequence_h
#define mitkDICOMCodeSequence_h

#include <MitkDICOMExports.h>

#include <string>

namespace mitk
{
  /**
   * @brief Represents a DICOM Code Sequence with value, scheme, and meaning.
   *
   * This class provides a simplified representation of a DICOM Code Sequence,
   * storing only the essential elements: Code Value, Coding Scheme Designator,
   * and Code Meaning. It abstracts the underlying DICOM library implementation.
   * The code value might stem from one of the following DICOM tags:
   * - Code Value (0008,0100)
   * - Long Code Value (0008,0119)
   * - or URN Code Value (0008,0120)
   * Which tag to use/correlate to depends on the content (length, or if an URN prefix exists).
   * This is e.g. handled by DCMTK.
   */
  class MITKDICOM_EXPORT DICOMCodeSequence
  {
  public:
    DICOMCodeSequence() = default;

    /**
     * @brief Constructs a DICOMCodeSequence with the specified attributes.
     * @param value The Code Value (0008,0100), Long Code Value (0008,0119), or URN Code Value (0008,0120)
     * @param scheme The Coding Scheme Designator (0008,0102)
     * @param meaning The Code Meaning (0008,0104)
     */
    DICOMCodeSequence(const std::string& value, const std::string& scheme, const std::string& meaning);

    virtual ~DICOMCodeSequence() = default;

    DICOMCodeSequence(const DICOMCodeSequence&) = default;
    DICOMCodeSequence& operator=(const DICOMCodeSequence&) = default;
    DICOMCodeSequence(DICOMCodeSequence&&) = default;
    DICOMCodeSequence& operator=(DICOMCodeSequence&&) = default;

    /**
     * @brief Gets the Code Value.
     */
    const std::string& GetValue() const;

    /**
     * @brief Sets the Code Value.
     */
    void SetValue(const std::string& value);

    /**
     * @brief Gets the Coding Scheme Designator.
     */
    const std::string& GetScheme() const;

    /**
     * @brief Sets the Coding Scheme Designator.
     */
    void SetScheme(const std::string& scheme);

    /**
     * @brief Gets the Code Meaning.
     */
    const std::string& GetMeaning() const;

    /**
     * @brief Sets the Code Meaning.
     */
    void SetMeaning(const std::string& meaning);

    /**
     * @brief Checks if the code is empty (all fields empty).
     * @return true if all fields are empty, false otherwise
     */
    bool IsEmpty() const;

    /**
     * @brief Equality comparison operator.
     * @param other The DICOMCodeSequence to compare with
     * @return true if value, scheme, and meaning are equal
     */
    bool operator==(const DICOMCodeSequence& other) const;

    /**
     * @brief Inequality comparison operator.
     * @param other The DICOMCodeSequence to compare with
     * @return true if any field differs
     */
    bool operator!=(const DICOMCodeSequence& other) const;

  protected:
    std::string m_Value;
    std::string m_Scheme;
    std::string m_Meaning;
  };

} // namespace mitk

#endif
