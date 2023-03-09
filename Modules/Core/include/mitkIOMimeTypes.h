/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOMimeTypes_h
#define mitkIOMimeTypes_h

#include "mitkCustomMimeType.h"

#include <string>

namespace mitk
{
  /**
   * @ingroup IO
   * @brief The IOMimeTypes class
   */
  class MITKCORE_EXPORT IOMimeTypes
  {
  public:

    /** Base mime types for all kind of DICOM images, that can be reused
    by more specific mime types based on DICOM images.*/
    class MITKCORE_EXPORT BaseDicomMimeType : public CustomMimeType
    {
    public:
      BaseDicomMimeType(const std::string &name);
      BaseDicomMimeType(const BaseDicomMimeType& other) = default;
      bool AppliesTo(const std::string& path) const override;
      BaseDicomMimeType* Clone() const override;
    };

    class MITKCORE_EXPORT DicomMimeType : public BaseDicomMimeType
    {
    public:
      DicomMimeType();
      DicomMimeType *Clone() const override;
    };

    static std::vector<CustomMimeType *> Get();

    static std::string DEFAULT_BASE_NAME(); // application/vnd.mitk

    static std::string CATEGORY_IMAGES();   // Images
    static std::string CATEGORY_SURFACES(); // Surfaces

    // ------------------------------ VTK formats ----------------------------------

    static CustomMimeType VTK_IMAGE_MIMETYPE();             // (mitk::Image) vti
    static CustomMimeType VTK_IMAGE_LEGACY_MIMETYPE();      // (mitk::Image) vtk
    static CustomMimeType VTK_PARALLEL_IMAGE_MIMETYPE();    // (mitk::Image) pvti
    static CustomMimeType VTK_POLYDATA_MIMETYPE();          // (mitk::Surface) vtp, vtk
    static CustomMimeType VTK_POLYDATA_LEGACY_MIMETYPE();   // (mitk::Surface) vtk
    static CustomMimeType VTK_PARALLEL_POLYDATA_MIMETYPE(); // (mitk::Surface) pvtp
    static CustomMimeType STEREOLITHOGRAPHY_MIMETYPE();     // (mitk::Surface) stl
    static CustomMimeType WAVEFRONT_OBJ_MIMETYPE();         // (mitk::Surface) obj
    static CustomMimeType STANFORD_PLY_MIMETYPE();          // (mitk::Surface) ply

    static std::string STEREOLITHOGRAPHY_NAME();     // DEFAULT_BASE_NAME.stl
    static std::string VTK_IMAGE_NAME();             // DEFAULT_BASE_NAME.vtk.image
    static std::string VTK_IMAGE_LEGACY_NAME();      // DEFAULT_BASE_NAME.vtk.image.legacy
    static std::string VTK_PARALLEL_IMAGE_NAME();    // DEFAULT_BASE_NAME.vtk.parallel.image
    static std::string VTK_POLYDATA_NAME();          // DEFAULT_BASE_NAME.vtk.polydata
    static std::string VTK_POLYDATA_LEGACY_NAME();   // DEFAULT_BASE_NAME.vtk.polydata.legacy
    static std::string VTK_PARALLEL_POLYDATA_NAME(); // DEFAULT_BASE_NAME.vtk.parallel.polydata
    static std::string WAVEFRONT_OBJ_NAME();         // DEFAULT_BASE_NAME.obj
    static std::string STANFORD_PLY_NAME();          // DEFAULT_BASE_NAME.ply

    // ------------------------- Image formats (ITK based) --------------------------

    static CustomMimeType NRRD_MIMETYPE(); // nrrd, nhdr
    static CustomMimeType NIFTI_MIMETYPE();
    static CustomMimeType RAW_MIMETYPE(); // raw
    static DicomMimeType DICOM_MIMETYPE();

    static std::string NRRD_MIMETYPE_NAME(); // DEFAULT_BASE_NAME.nrrd
    static std::string NIFTI_MIMETYPE_NAME();
    static std::string RAW_MIMETYPE_NAME(); // DEFAULT_BASE_NAME.raw
    static std::string DICOM_MIMETYPE_NAME();

    // ------------------------------ MITK formats ----------------------------------

    static CustomMimeType POINTSET_MIMETYPE();      // mps
    static CustomMimeType GEOMETRY_DATA_MIMETYPE(); // .mitkgeometry

    static std::string POINTSET_MIMETYPE_NAME(); // DEFAULT_BASE_NAME.pointset

  private:
    // purposely not implemented
    IOMimeTypes();
    IOMimeTypes(const IOMimeTypes &);
  };
}

#endif
