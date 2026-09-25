/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMFilesHelper_h
#define mitkDICOMFilesHelper_h

#include <optional>
#include <string>
#include <vector>

#include <MitkDICOMExports.h>

namespace mitk {

/** \brief A list of file paths used throughout the DICOM module. */
typedef std::vector<std::string> DICOMFilePathList;

/**
 * \brief Search a directory for DICOM files.
 *
 * Searches for all files in the directory of the passed file path.
 * All files will be checked if they are DICOM files. All DICOM files will be added to the result and
 * returned.
 *
 * \param[in] filePath A file path whose parent directory will be searched for DICOM files.
 *                     If a directory path is passed, that directory is searched directly.
 * \return A list of absolute paths to all DICOM files found in the directory.
 * \note The helper does no sorting of any kind.
 * \sa FilterForDICOMFiles, FilterDICOMFilesForSameSeries
 */
DICOMFilePathList MITKDICOM_EXPORT GetDICOMFilesInSameDirectory(const std::string& filePath);

/**
 * \brief Filter a file list to contain only valid DICOM files.
 *
 * All passed files will be checked if they are DICOM files.
 * All DICOM files will be added to the result and returned.
 *
 * \param[in] fileList The list of file paths to filter.
 * \return A list of paths that are valid DICOM files.
 * \note The helper does no sorting of any kind.
 * \sa GetDICOMFilesInSameDirectory
 */
DICOMFilePathList MITKDICOM_EXPORT FilterForDICOMFiles(const DICOMFilePathList& fileList);

/**
 * \brief Filter DICOM files to return only those in the same series as a reference file.
 *
 * Returns all DICOM files passed with fileList that have the same Series Instance UID
 * as the passed refFilePath.
 *
 * \param[in] refFilePath Path to the reference DICOM file whose Series Instance UID is used for filtering.
 * \param[in] fileList The list of candidate DICOM file paths.
 * \return A list of DICOM file paths that share the same Series Instance UID as refFilePath.
 * \pre refFilePath must point to a valid DICOM file.
 * \sa GetDICOMFilesInSameDirectory, FilterForDICOMFiles
 */
DICOMFilePathList MITKDICOM_EXPORT FilterDICOMFilesForSameSeries(const std::string& refFilePath, const DICOMFilePathList& fileList);

/**
 * \brief The entry of listedFiles that refers to the same file as file.
 *
 * An entry that equals file as a path is taken first. Otherwise the
 * filesystem decides, but only for entries whose file name equals the one
 * of file ignoring (ASCII) case, so resolving a differently cased name costs
 * about one lookup instead of one per listed file. A short (8.3) name is
 * therefore not resolved.
 *
 * \param[in] file The path to resolve, spelled as the caller got it.
 * \param[in] listedFiles The directory listing to resolve it against.
 * \return The listed entry, or nothing if no entry refers to file.
 */
std::optional<std::string> MITKDICOM_EXPORT FindListedFile(const std::string& file, const DICOMFilePathList& listedFiles);
}

#endif
