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

#ifndef _MITK_DiffusionImageNrrdWriterService__H_
#define _MITK_DiffusionImageNrrdWriterService__H_

#include <mitkAbstractFileWriter.h>
#include <mitkDiffusionPropertyHelper.h>

namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */
class DiffusionImageNrrdWriterService : public mitk::AbstractFileWriter
{
public:

    DiffusionImageNrrdWriterService();
    virtual ~DiffusionImageNrrdWriterService();

    using AbstractFileWriter::Write;
    virtual void Write() override;

    virtual ConfidenceLevel GetConfidenceLevel() const override;

    typedef mitk::DiffusionPropertyHelper::ImageType                        VectorImageType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
    typedef mitk::DiffusionPropertyHelper::MeasurementFrameType             MeasurementFrameType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;

  protected:

    DiffusionImageNrrdWriterService(const DiffusionImageNrrdWriterService& other);
    virtual mitk::DiffusionImageNrrdWriterService* Clone() const override;


};


} // end of namespace mitk

#endif
