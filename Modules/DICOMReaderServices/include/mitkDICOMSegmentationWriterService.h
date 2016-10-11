/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef _MITK_DICOM_SEGMENTATION_WRITER_SERVICE__H_
#define _MITK_DICOM_SEGMENTATION_WRITER_SERVICE__H_

#include <mitkAbstractFileWriter.h>

namespace mitk
{
//TODO: Docu
/**
 * @internal
 *
 * @brief 
 *
 * @ingroup IO
 */
class DICOMSegmentationWriterService : public AbstractFileWriter
{
public:

  DICOMSegmentationWriterService();
  virtual ~DICOMSegmentationWriterService();

  using AbstractFileWriter::Write;
  virtual void Write() override;

  virtual ConfidenceLevel GetConfidenceLevel() const override;

private:

  DICOMSegmentationWriterService(const DICOMSegmentationWriterService& other);

  virtual mitk::DICOMSegmentationWriterService* Clone() const override;

};

}

#endif
