#pragma once

#include "mitkFeedbackContourTool.h"
#include "mitkLegacyAdaptors.h"
#include <MitkSegmentationExports.h>

struct mitkIpPicDescriptor;

namespace us {
class ModuleResource;
}

namespace mitk
{

class MITKSEGMENTATION_EXPORT ClassicRegionGrowingTool : public FeedbackContourTool
{
public:

  mitkClassMacro(ClassicRegionGrowingTool, FeedbackContourTool);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual const char** GetXPM() const override;
  virtual us::ModuleResource GetCursorIconResource() const override;
  us::ModuleResource GetIconResource() const override;

  virtual const char* GetName() const override;

protected:

  ClassicRegionGrowingTool(); // purposely hidden
  virtual ~ClassicRegionGrowingTool();

  void ConnectActionsAndFunctions() override;

  virtual void Activated() override;
  virtual void Deactivated() override;

  virtual void OnMousePressed(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool OnMousePressedInside(StateMachineAction*, InteractionEvent* interactionEvent, mitkIpPicDescriptor* workingPicSlice, int initialWorkingOffset);
  virtual void OnMousePressedOutside(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual void OnMouseMoved(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual void OnMouseReleased(StateMachineAction*, InteractionEvent* interactionEvent);

  mitkIpPicDescriptor* PerformRegionGrowingAndUpdateContour(int timestep = 0);

  Image::Pointer m_ReferenceSlice;
  Image::Pointer m_WorkingSlice;

  ScalarType m_LowerThreshold;
  ScalarType m_UpperThreshold;
  ScalarType m_InitialLowerThreshold;
  ScalarType m_InitialUpperThreshold;

  Point2I m_LastScreenPosition;
  int m_ScreenYDifference;

private:

  mitkIpPicDescriptor* SmoothIPPicBinaryImage(mitkIpPicDescriptor* image, int &contourOfs, mitkIpPicDescriptor* dest = NULL);
  void SmoothIPPicBinaryImageHelperForRows(mitkIpPicDescriptor* source, mitkIpPicDescriptor* dest, int &contourOfs, int* maskOffsets, int maskSize, int startOffset, int endOffset);

  mitkIpPicDescriptor* m_OriginalPicSlice;
  int m_SeedPointMemoryOffset;

  ScalarType m_VisibleWindow;
  ScalarType m_DefaultWindow;
  ScalarType m_MouseDistanceScaleFactor;

  int m_PaintingPixelValue;
  int m_LastWorkingSeed;

  bool m_FillFeedbackContour;
};

} // namespace
