/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOMimeTypes_h
#define mitkIOMimeTypes_h

#include <mitkCustomMimeType.h>

#include <string>

namespace mitk
{
  /**
   * \ingroup IO
   *
   * \brief Collection of predefined MITK mime-types and factory methods.
   *
   * This utility class provides static factory methods for creating
   * CustomMimeType objects for all file formats supported by MITK Core
   * (VTK, ITK, DICOM, NRRD, NIfTI, STL, OBJ, PLY, etc.). It also
   * defines the corresponding mime-type name strings and category constants.
   *
   * Use Get() to obtain the full set of default mime-types for bulk
   * registration, or call individual factory methods such as NRRD_MIMETYPE()
   * to obtain a specific mime-type.
   *
   * This class cannot be instantiated.
   *
   * \sa CustomMimeType
   * \sa MimeType
   * \sa IMimeTypeProvider
   */
  class MITKCORE_EXPORT IOMimeTypes
  {
  public:

    /**
     * \brief Base mime-type for all DICOM image formats.
     *
     * This mime-type class uses GDCM to inspect file contents and verify
     * DICOM compatibility. It can be subclassed to create more specific
     * DICOM mime-types. DICOM-RT modalities (RTSTRUCT, RTDOSE, RTPLAN)
     * are explicitly excluded.
     *
     * \sa DicomMimeType
     */
    class MITKCORE_EXPORT BaseDicomMimeType : public CustomMimeType
    {
    public:
      /**
       * \brief Construct with a specific mime-type name.
       * \param[in] name The unique mime-type name.
       */
      BaseDicomMimeType(const std::string &name);

      BaseDicomMimeType(const BaseDicomMimeType& other) = default;

      /**
       * \brief Check whether the given path points to a readable DICOM file.
       * \param[in] path File or directory path to inspect.
       * \return \c true if the path contains valid DICOM data (excluding RT modalities).
       */
      bool AppliesTo(const std::string& path) const override;

      /**
       * \brief Clone this mime-type.
       * \return A heap-allocated copy. Caller takes ownership.
       */
      BaseDicomMimeType* Clone() const override;
    };

    /**
     * \brief The default DICOM mime-type for standard (non-RT) DICOM images.
     *
     * \sa BaseDicomMimeType
     */
    class MITKCORE_EXPORT DicomMimeType : public BaseDicomMimeType
    {
    public:
      /** \brief Construct the default DICOM mime-type. */
      DicomMimeType();

      /**
       * \brief Clone this mime-type.
       * \return A heap-allocated copy. Caller takes ownership.
       */
      DicomMimeType *Clone() const override;
    };

    /**
     * \brief Get all predefined MITK Core mime-types.
     * \return A vector of heap-allocated CustomMimeType pointers. The caller
     *         takes ownership of each pointer.
     *
     * The returned mime-types are ordered by descending rank.
     */
    static std::vector<CustomMimeType *> Get();

    /** \brief Get the default base name prefix for MITK mime-types ("application/vnd.mitk").
     *  \return The base name string. */
    static std::string DEFAULT_BASE_NAME();

    /** \brief Get the "Images" category string.
     *  \return The category string "Images". */
    static std::string CATEGORY_IMAGES();

    /** \brief Get the "Surfaces" category string.
     *  \return The category string "Surfaces". */
    static std::string CATEGORY_SURFACES();

    // ------------------------------ VTK formats ----------------------------------

    /** \brief Create a CustomMimeType for VTK XML image files (.vti).
     *  \return A CustomMimeType for VTK images. */
    static CustomMimeType VTK_IMAGE_MIMETYPE();

    /** \brief Create a CustomMimeType for VTK legacy image files (.vtk).
     *  \return A CustomMimeType for VTK legacy images. */
    static CustomMimeType VTK_IMAGE_LEGACY_MIMETYPE();

    /** \brief Create a CustomMimeType for VTK parallel image files (.pvti).
     *  \return A CustomMimeType for VTK parallel images. */
    static CustomMimeType VTK_PARALLEL_IMAGE_MIMETYPE();

    /** \brief Create a CustomMimeType for VTK XML polydata files (.vtp).
     *  \return A CustomMimeType for VTK polydata surfaces. */
    static CustomMimeType VTK_POLYDATA_MIMETYPE();

    /** \brief Create a CustomMimeType for VTK legacy polydata files (.vtk).
     *  \return A CustomMimeType for VTK legacy polydata surfaces. */
    static CustomMimeType VTK_POLYDATA_LEGACY_MIMETYPE();

    /** \brief Create a CustomMimeType for VTK parallel polydata files (.pvtp).
     *  \return A CustomMimeType for VTK parallel polydata surfaces. */
    static CustomMimeType VTK_PARALLEL_POLYDATA_MIMETYPE();

    /** \brief Create a CustomMimeType for STL surface files (.stl).
     *  \return A CustomMimeType for stereolithography surfaces. */
    static CustomMimeType STEREOLITHOGRAPHY_MIMETYPE();

    /** \brief Create a CustomMimeType for Wavefront OBJ surface files (.obj).
     *  \return A CustomMimeType for Wavefront OBJ surfaces. */
    static CustomMimeType WAVEFRONT_OBJ_MIMETYPE();

    /** \brief Create a CustomMimeType for Stanford PLY surface files (.ply).
     *  \return A CustomMimeType for Stanford PLY surfaces. */
    static CustomMimeType STANFORD_PLY_MIMETYPE();

    /** \brief Get the mime-type name for STL files. \return "application/vnd.mitk.stl" */
    static std::string STEREOLITHOGRAPHY_NAME();
    /** \brief Get the mime-type name for VTK XML images. \return "application/vnd.mitk.vtk.image" */
    static std::string VTK_IMAGE_NAME();
    /** \brief Get the mime-type name for VTK legacy images. \return "application/vnd.mitk.vtk.image.legacy" */
    static std::string VTK_IMAGE_LEGACY_NAME();
    /** \brief Get the mime-type name for VTK parallel images. \return "application/vnd.mitk.vtk.parallel.image" */
    static std::string VTK_PARALLEL_IMAGE_NAME();
    /** \brief Get the mime-type name for VTK XML polydata. \return "application/vnd.mitk.vtk.polydata" */
    static std::string VTK_POLYDATA_NAME();
    /** \brief Get the mime-type name for VTK legacy polydata. \return "application/vnd.mitk.vtk.polydata.legacy" */
    static std::string VTK_POLYDATA_LEGACY_NAME();
    /** \brief Get the mime-type name for VTK parallel polydata. \return "application/vnd.mitk.vtk.parallel.polydata" */
    static std::string VTK_PARALLEL_POLYDATA_NAME();
    /** \brief Get the mime-type name for Wavefront OBJ. \return "application/vnd.mitk.obj" */
    static std::string WAVEFRONT_OBJ_NAME();
    /** \brief Get the mime-type name for Stanford PLY. \return "application/vnd.mitk.ply" */
    static std::string STANFORD_PLY_NAME();

    // ------------------------- Image formats (ITK based) --------------------------

    /** \brief Create a CustomMimeType for NRRD image files (.nrrd, .nhdr).
     *  \return A CustomMimeType for NRRD images. */
    static CustomMimeType NRRD_MIMETYPE();

    /** \brief Create a CustomMimeType for NIfTI image files (.nii, .nii.gz, .hdr, etc.).
     *  \return A CustomMimeType for NIfTI images. */
    static CustomMimeType NIFTI_MIMETYPE();

    /** \brief Create a CustomMimeType for raw image files (.raw).
     *  \return A CustomMimeType for raw image data. */
    static CustomMimeType RAW_MIMETYPE();

    /** \brief Create a DicomMimeType for standard DICOM image files.
     *  \return A DicomMimeType instance. */
    static DicomMimeType DICOM_MIMETYPE();

    /** \brief Get the mime-type name for NRRD. \return "application/vnd.mitk.image.nrrd" */
    static std::string NRRD_MIMETYPE_NAME();
    /** \brief Get the mime-type name for NIfTI. \return "application/vnd.mitk.image.nifti" */
    static std::string NIFTI_MIMETYPE_NAME();
    /** \brief Get the mime-type name for raw images. \return "application/vnd.mitk.image.raw" */
    static std::string RAW_MIMETYPE_NAME();
    /** \brief Get the mime-type name for DICOM. \return "application/vnd.mitk.image.dicom" */
    static std::string DICOM_MIMETYPE_NAME();

    // ------------------------------ MITK formats ----------------------------------

    /** \brief Create a CustomMimeType for MITK point set files (.mps).
     *  \return A CustomMimeType for MITK point sets. */
    static CustomMimeType POINTSET_MIMETYPE();

    /** \brief Create a CustomMimeType for MITK geometry data files (.mitkgeometry).
     *  \return A CustomMimeType for MITK geometry data. */
    static CustomMimeType GEOMETRY_DATA_MIMETYPE();

    /** \brief Get the mime-type name for point sets. \return "application/vnd.mitk.pointset" */
    static std::string POINTSET_MIMETYPE_NAME();

  private:
    // purposely not implemented
    IOMimeTypes();
    IOMimeTypes(const IOMimeTypes &);
  };
}

#endif
