/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationStackNNiftiWriter_h
#define mitkMultiLabelSegmentationStackNNiftiWriter_h

#include <mitkMultiLabelSegmentationStackWriterBase.h>

namespace mitk
{
  /**
  * Writes a MultiLabelSegmentation as stacks of nifti files includin meta
  * information as mitklabel.json file.
  * mitk::Identifiable UID is supported and will be serialized.
  * @ingroup Process
  */
  class MultiLabelSegmentationStackNiftiWriter : public MultiLabelSegmentationStackWriterBase
  {
  public:
    MultiLabelSegmentationStackNiftiWriter();
    ~MultiLabelSegmentationStackNiftiWriter() = default;

  protected:
    itk::ImageIOBase::Pointer GetITKIO() const override;
    std::string GetStackImageExtension() const override;
  private:
    MultiLabelSegmentationStackNiftiWriter *Clone() const override;
  };
} // end of namespace mitk

#endif
