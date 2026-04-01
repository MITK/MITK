/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultilabelIOMimeTypes_h
#define mitkMultilabelIOMimeTypes_h

#include <mitkCustomMimeType.h>
#include <MitkMultilabelIOExports.h>

namespace mitk
{
  /** \brief Provides custom MIME types for multi-label segmentation IO. */
  namespace MitkMultilabelIOMimeTypes
  {
    /** \brief MIME type for legacy LabelSetImage NRRD files. */
    class MITKMULTILABELIO_EXPORT LegacyLabelSetMimeType : public CustomMimeType
    {
    public:
      LegacyLabelSetMimeType();

      bool AppliesTo(const std::string& path) const override;
      LegacyLabelSetMimeType* Clone() const override;
    };

    /** \brief Return an instance of the legacy LabelSet MIME type. */
    MITKMULTILABELIO_EXPORT LegacyLabelSetMimeType LEGACYLABELSET_MIMETYPE();

    /** \brief Return the name string for the legacy LabelSet MIME type. */
    MITKMULTILABELIO_EXPORT std::string LEGACYLABELSET_MIMETYPE_NAME();

    /** \brief MIME type for MultiLabelSegmentation NRRD files with embedded label metadata. */
    class MITKMULTILABELIO_EXPORT MultiLabelSegmentationMimeType : public CustomMimeType
    {
    public:
      MultiLabelSegmentationMimeType();

      bool AppliesTo(const std::string& path) const override;
      MultiLabelSegmentationMimeType* Clone() const override;
    };

    /** \brief Return an instance of the MultiLabelSegmentation MIME type. */
    MITKMULTILABELIO_EXPORT MultiLabelSegmentationMimeType MULTILABEL_SEGMENTATION_MIMETYPE();

    /** \brief Return the name string for the MultiLabelSegmentation MIME type. */
    MITKMULTILABELIO_EXPORT std::string MULTILABEL_SEGMENTATION_MIMETYPE_NAME();

    /** \brief MIME type for multi-label meta JSON files (mitklabel.json). */
    class MITKMULTILABELIO_EXPORT MultiLabelMetaMimeType : public CustomMimeType
    {
    public:
      MultiLabelMetaMimeType();
      bool AppliesTo(const std::string& path) const override;
      MultiLabelMetaMimeType* Clone() const override;
    };

    /** \brief Return an instance of the MultiLabelMeta MIME type. */
    MITKMULTILABELIO_EXPORT MultiLabelMetaMimeType MULTILABELMETA_MIMETYPE();

    /** \brief Return the name string for the MultiLabelMeta MIME type. */
    MITKMULTILABELIO_EXPORT std::string MULTILABELMETA_MIMETYPE_NAME();

    /** \brief MIME type for multi-label NIfTI stack directories. */
    class MITKMULTILABELIO_EXPORT MultiLabelNiftiStackMimeType : public CustomMimeType
    {
    public:
      MultiLabelNiftiStackMimeType();

      bool AppliesTo(const std::string& path) const override;
      MultiLabelNiftiStackMimeType* Clone() const override;
    };

    /** \brief Return an instance of the MultiLabel NIfTI stack MIME type. */
    MITKMULTILABELIO_EXPORT MultiLabelNiftiStackMimeType MULTILABEL_NIFTISTACK_MIMETYPE();

    /** \brief Return the name string for the MultiLabel NIfTI stack MIME type. */
    MITKMULTILABELIO_EXPORT std::string MULTILABEL_NIFTISTACK_MIMETYPE_NAME();

    /** \brief Return all registered multi-label IO MIME types. Caller takes ownership. */
    MITKMULTILABELIO_EXPORT std::vector<CustomMimeType*> Get();

  }
}

#endif
