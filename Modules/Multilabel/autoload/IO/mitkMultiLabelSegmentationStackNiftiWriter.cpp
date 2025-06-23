/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelSegmentationStackNiftiWriter.h"

#include "mitkMultilabelIOMimeTypes.h"

// itk
#include "itkNiftiImageIO.h"

namespace mitk
{
  MultiLabelSegmentationStackNiftiWriter::MultiLabelSegmentationStackNiftiWriter()
    : MultiLabelSegmentationStackWriterBase(MitkMultilabelIOMimeTypes::MULTILABEL_NIFTISTACK_MIMETYPE(), "MITK ML Segmentation NIFTI Stack")
  {
    AbstractFileWriter::SetRanking(5);
    this->RegisterService();
  }

  MultiLabelSegmentationStackNiftiWriter* MultiLabelSegmentationStackNiftiWriter::Clone() const { return new MultiLabelSegmentationStackNiftiWriter(*this); }

  itk::ImageIOBase::Pointer MultiLabelSegmentationStackNiftiWriter::GetITKIO() const
  {
    return itk::NiftiImageIO::New();
  }

  std::string MultiLabelSegmentationStackNiftiWriter::GetStackImageExtension() const
  {
    return ".nii.gz";
  }

} // namespace
