/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationStackReader_h
#define mitkMultiLabelSegmentationStackReader_h

#include <mitkAbstractFileReader.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
  * Reads a MultiLabelSegmentation from a mitklabel.json file 
  * (including the associated image files).
  * mitk::Identifiable UID is supported and will be serialized.
  * @ingroup Process
  */
  class MultiLabelSegmentationStackReader : public mitk::AbstractFileReader
  {
  public:
    typedef mitk::MultiLabelSegmentation InputType;

    MultiLabelSegmentationStackReader();
    ~MultiLabelSegmentationStackReader() = default;

    ConfidenceLevel GetConfidenceLevel() const override;

  protected:

    std::vector<itk::SmartPointer<BaseData>> DoRead() override;


  private:
    MultiLabelSegmentationStackReader* Clone() const override;
  };
} // end of namespace mitk

#endif
