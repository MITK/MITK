/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMedSAMTool_h
#define mitkMedSAMTool_h

#include "mitkSegmentAnythingTool.h"
#include "mitkSegWithPreviewTool.h"
#include "mitkPointSet.h"
#include "mitkProcessExecutor.h"
#include "mitkSegmentAnythingPythonService.h"
#include <MitkSegmentationExports.h>
#include <itkImage.h>
#include <mitkLevelWindow.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
  \brief Medical Segment Anything Model interactive 2D tool class.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT MedSAMTool : public SegmentAnythingTool
  {
  public:
    mitkClassMacro(MedSAMTool, SegmentAnythingTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;

    //void Activated() override;
    //bool HasPicks() const;
    //void ConnectActionsAndFunctions() override;
    std::stringstream GetPointsAsCSVString(const mitk::BaseGeometry *);

  protected:
    MedSAMTool() = default;
    ~MedSAMTool() = default;
  };
} // namespace

#endif
