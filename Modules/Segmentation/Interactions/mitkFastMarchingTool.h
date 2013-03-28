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

#ifndef mitkFastMarchingTool_h_Included
#define mitkFastMarchingTool_h_Included

#include "mitkFeedbackContourTool.h"
#include "mitkLegacyAdaptors.h"
#include "SegmentationExports.h"


namespace mitk
{

/**
  \brief FastMarching
*/
class Segmentation_EXPORT FastMarchingTool : public FeedbackContourTool
{
  public:

    mitkClassMacro(FastMarchingTool, FeedbackContourTool);
    itkNewMacro(FastMarchingTool);


    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:

    FastMarchingTool(); // purposely hidden
    virtual ~FastMarchingTool();

    virtual void Activated();
    virtual void Deactivated();

    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);
    virtual bool OnAddPoint(Action*, const StateEvent*);
    virtual bool OnDelete(Action*, const StateEvent*);


    Image::Pointer m_ReferenceSlice;
    Image::Pointer m_WorkingSlice;

    ScalarType m_LowerThreshold;
    ScalarType m_UpperThreshold;
    ScalarType m_InitialLowerThreshold;
    ScalarType m_InitialUpperThreshold;

    Point2I m_LastScreenPosition;
    int m_ScreenYDifference;

  private:

    ScalarType m_MouseDistanceScaleFactor;

};

} // namespace

#endif


