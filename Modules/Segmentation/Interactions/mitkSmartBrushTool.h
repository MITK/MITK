#pragma once

#include <itkBinaryThresholdImageFilter.h>

#include <MitkSegmentationExports.h>

#include "mitkFeedbackContourTool.h"
#include "mitkDataNode.h"
#include "mitkPointSet.h"

#include "mitkSmartBrushStrokeFilter.h"

namespace mitk
{
  class StateMachineAction;
  class InteractionEvent;
  class InteractionPositionEvent;

  class MITKSEGMENTATION_EXPORT SmartBrushTool : public FeedbackContourTool {
  public:
    Message1<int> radiusChanged;
    Message1<int> sensitivityChanged;

    mitkClassMacro(SmartBrushTool, FeedbackContourTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetRadius(int value);
    void SetSensitivity(int value);

    void SetGeometrySlice(const itk::EventObject& geometrySliceEvent);
    void SetGeometry(const itk::EventObject&) {};

    virtual const char** GetXPM() const override;
    virtual us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;
    virtual const char* GetName() const override;

  protected:
    SmartBrushTool();
    virtual ~SmartBrushTool();

    typedef float InternalPixelType;
    typedef itk::Image<InternalPixelType, 3> InternalImageType;
    typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, 3> SegmentationType;

    InternalImageType::Pointer m_OriginalImage;

    void ConnectActionsAndFunctions() override;

    virtual void Activated() override;
    virtual void Deactivated() override;

    virtual void OnPrimaryButtonPressed(StateMachineAction*, InteractionEvent*);
    virtual void OnAltPrimaryButtonPressed(StateMachineAction*, InteractionEvent*);
    virtual void OnAltSecondaryButtonPressed(StateMachineAction*, InteractionEvent*);
    virtual void OnCtrlPrimaryButtonPressed(StateMachineAction*, InteractionEvent*);
    virtual void OnPrimaryButtonPressedMoved(StateMachineAction*, InteractionEvent*);
    virtual void OnMouseMoved(StateMachineAction*, InteractionEvent*);
    virtual void OnMouseReleased(StateMachineAction*, InteractionEvent*);

    virtual void MousePressed(mitk::InteractionEvent* interactionEvent);
    virtual void MouseMoved(mitk::InteractionEvent* interactionEvent, bool leftMouseButtonPressed);
    void MouseMovedImpl(const mitk::PlaneGeometry* planeGeometry, bool LeftMouseButtonPressed);

    void UpdateContour();

    mitk::Point2D upperLeft(mitk::Point2D p);
    void CheckIfCurrentSliceHasChanged(const mitk::PlaneGeometry* planeGeometry);

    void OnToolManagerImageModified(const itk::EventObject&);
    void OnToolManagerWorkingDataModified();

    enum Directions {
      ADD = 1,
      REMOVE = 0,
    } m_CurrentDirection;

    itk::Image<float, 3>::Pointer m_StrokeBuffer;

    SmartBrushStrokeFilter<InternalImageType>::Pointer m_SmartBrushStrokeFilter;
    itk::BinaryThresholdImageFilter<itk::Image<float, 3>, SegmentationType>::Pointer m_ThresholdFilter;

    int m_Radius;
    int m_Sensitivity;

    ContourModel::Pointer m_MasterContour;
    int m_LastContourRadius;

    int m_LastTimeStep;
    Point3D m_LastWorldCoordinates;
    Point3D m_LastPosition;
    Point3D m_ReferencePoint;

    Image::Pointer m_WorkingImage;
    DataNode::Pointer m_WorkingNode;
    int m_ImageObserverTag;
    Image::Pointer m_WorkingSlice;
    PlaneGeometry::Pointer m_CurrentPlane;

    std::vector<mitk::BaseRenderer*> m_Windows;

    DataNode::Pointer m_PointSetNode;
    PointSet::Pointer m_PointSet;
    bool m_IsReferencePointSet;
  };
}
