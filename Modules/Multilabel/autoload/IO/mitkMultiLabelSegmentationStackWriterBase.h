/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationStackWriterBase_h
#define mitkMultiLabelSegmentationStackWriterBase_h

#include <mitkAbstractFileWriter.h>
#include <mitkLabelSetImage.h>

#include <itkImageIOBase.h>

#include <MitkMultilabelIOExports.h>

namespace mitk
{
  /**
   * \brief Base class for writers that export MultiLabelSegmentation to a stack of image files with a metadata file.
   *
   * Subclasses must provide the ITK ImageIO instance and file extension
   * for the image format used in the stack.
   *
   * \ingroup Process
   */
  class MITKMULTILABELIO_EXPORT MultiLabelSegmentationStackWriterBase : public mitk::AbstractFileWriter
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;

    /**
     * \brief Construct the writer base with a specific MIME type and description.
     * \param mimeType The custom MIME type this writer handles.
     * \param description Human-readable description.
     */
    MultiLabelSegmentationStackWriterBase(const CustomMimeType& mimeType,
      const std::string& description);

    /** \brief Write the MultiLabelSegmentation as a stack of image files plus metadata. */
    void Write() override;

    /** \brief Return the confidence level for writing the given data. */
    ConfidenceLevel GetConfidenceLevel() const override;

  protected:
    /** \brief Return the ITK ImageIO instance used for writing individual stack images. */
    virtual itk::ImageIOBase::Pointer GetITKIO() const = 0;

    /** \brief Return the file extension for individual stack image files (e.g. ".nii.gz"). */
    virtual std::string GetStackImageExtension() const = 0;

  private:
    std::vector<std::string> m_DefaultMetaDataKeys;
  };
} // end of namespace mitk

#endif
