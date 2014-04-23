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

#ifndef mitkBinaryThresholdULTool_h_Included
#define mitkBinaryThresholdULTool_h_Included

#include <MitkSegmentationExports.h>
#include "mitkSegTool3D.h"

#include <itkImage.h>

namespace us {
class ModuleResource;
}

namespace mitk
{

  /**
  \brief A tool for image thresholding using lower and upper thresholds.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
  */
  class MitkSegmentation_EXPORT BinaryThresholdULTool : public SegTool3D
  {
  public:

    Message3<mitk::ScalarType, mitk::ScalarType, bool> IntervalBordersChanged;
    Message2<mitk::ScalarType, mitk::ScalarType>  ThresholdingValuesChanged;

    mitkClassMacro(BinaryThresholdULTool, SegTool3D);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const;
    us::ModuleResource GetIconResource() const;
    virtual const char* GetName() const;

    virtual void Activated();

    void Run();

    void SetThresholdValues(mitk::ScalarType lower, mitk::ScalarType upper);

  protected:

    BinaryThresholdULTool(); // purposely hidden
    virtual ~BinaryThresholdULTool();

    template < typename TPixel, unsigned int VDimension >
    void InternalRun( itk::Image<TPixel, VDimension>* source );

    mitk::ScalarType m_SensibleMinimumThresholdValue;
    mitk::ScalarType m_SensibleMaximumThresholdValue;
    mitk::ScalarType m_CurrentLowerThresholdValue;
    mitk::ScalarType m_CurrentUpperThresholdValue;

    bool m_IsFloatImage;
  };

} // namespace

#endif

