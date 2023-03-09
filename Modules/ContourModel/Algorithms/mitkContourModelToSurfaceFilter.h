/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelToSurfaceFilter_h
#define mitkContourModelToSurfaceFilter_h

#include "mitkCommon.h"
#include <MitkContourModelExports.h>

#include "mitkContourModel.h"
#include "mitkContourModelSource.h"

#include <mitkSurfaceSource.h>

namespace mitk
{
  class MITKCONTOURMODEL_EXPORT ContourModelToSurfaceFilter : public SurfaceSource
  {
  public:
    /** Standard class typedefs. */
    mitkClassMacro(ContourModelToSurfaceFilter, SurfaceSource);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef mitk::Surface OutputType;

    typedef mitk::ContourModel InputType;

    void GenerateOutputInformation() override;

    /** Set/Get the image input of this process object.  */
    using Superclass::SetInput;
    virtual void SetInput(const InputType *input);
    virtual void SetInput(unsigned int idx, const InputType *input);
    const InputType *GetInput(void);
    const InputType *GetInput(unsigned int idx);

  protected:
    ContourModelToSurfaceFilter();
    ~ContourModelToSurfaceFilter() override;

    void GenerateData() override;
  };
}
#endif
