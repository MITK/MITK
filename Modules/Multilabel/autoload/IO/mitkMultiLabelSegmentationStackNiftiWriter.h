/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationStackNiftiWriter_h
#define mitkMultiLabelSegmentationStackNiftiWriter_h

#include <mitkMultiLabelSegmentationStackWriterBase.h>

namespace mitk
{
  /**
   * \brief Write a MultiLabelSegmentation as stacks of NIfTI files with a mitklabel.json metadata file.
   *
   * mitk::Identifiable UID is supported and will be serialized.
   *
   * \ingroup Process
   */
  class MultiLabelSegmentationStackNiftiWriter : public MultiLabelSegmentationStackWriterBase
  {
  public:
    /** \brief Default constructor. Registers writer for the NIfTI stack MIME type. */
    MultiLabelSegmentationStackNiftiWriter();
    ~MultiLabelSegmentationStackNiftiWriter() = default;

  protected:
    /** \brief Return the ITK NIfTI ImageIO instance used for writing individual stack images. */
    itk::ImageIOBase::Pointer GetITKIO() const override;

    /** \brief Return the file extension for NIfTI stack images (".nii.gz"). */
    std::string GetStackImageExtension() const override;
  private:
    MultiLabelSegmentationStackNiftiWriter *Clone() const override;
  };
} // end of namespace mitk

#endif
