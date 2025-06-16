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
  * Base class for writes that export MultiLabelSegmentation to a
  * stack of image file (per group or per label) including a meta
  * data file.
  * @ingroup Process
  */
  class MITKMULTILABELIO_EXPORT MultiLabelSegmentationStackWriterBase : public mitk::AbstractFileWriter
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;

    MultiLabelSegmentationStackWriterBase(const CustomMimeType& mimeType,
      const std::string& description);

    void Write() override;
    ConfidenceLevel GetConfidenceLevel() const override;

  protected:
    virtual itk::ImageIOBase::Pointer GetITKIO() const = 0;

  private:
    std::vector<std::string> m_DefaultMetaDataKeys;
  };
} // end of namespace mitk

#endif
