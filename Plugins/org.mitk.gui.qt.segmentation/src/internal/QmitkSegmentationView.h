/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSEGMENTATIONVIEW_H
#define QMITKSEGMENTATIONVIEW_H

#include "ui_QmitkSegmentationViewControls.h"

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include <berryIBerryPreferences.h>

#include <mitkSegmentationInteractor.h>

// mitk annotation
#include <mitkLogoAnnotation.h>

/**
* @brief The segmentation view provides a set of tool to use different segmentation algorithms.
*        It provides two selection widgets to load an image node and a segmentation node
*        on which to perform the segmentation. Creating new segmentation nodes is also possible.
*        The available segmentation tools are grouped into "2D"- and "3D"-tools.
*
*        Most segmentation tools / algorithms need some kind of user interaction, where the
*        user is asked to draw something in the image display or set some seed points / start values.
*        The tools also often provide additional propeties so that a user can modify the
*        algorithm's behavior.
*
*        This class additionally provides options to work with different layers (create new layers,
*        switch between layers).
*        Moreover, a multilabel widget displays all the existing labels of a multilabel segmentation
*        for the currently active layer.
*        The multilabel widget allows to control the labels by creatin new one, removing existing ones,
*        showing / hiding single labels, merging labels, (re-)naming them etc.
*
*        Additionally the view provides an option to create "2D"- and "3D"-interpolations between
*        neighboring segmentation masks on unsegmented slices.
*        Interpolation for multilabel segmentations is currently not implemented.
*/
class QmitkSegmentationView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkSegmentationView();
  ~QmitkSegmentationView() override;

private Q_SLOTS:

  // reaction to the selection of a new reference image in the selection widget
  void OnReferenceSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  // reaction to the selection of a new segmentation image in the selection widget
  void OnSegmentationSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

  // reaction to the shortcut ("CTRL+H") for toggling the visibility of the working node
  void OnVisibilityShortcutActivated();

  // reaction to the shortcut ("CTRL+L") for iterating over all labels
  void OnLabelToggleShortcutActivated();

  // reaction to the button "New segmentation"
  void OnNewSegmentation();

  void OnManualTool2DSelected(int id);

  void OnShowMarkerNodes(bool);

  void OnLayersChanged();

  void OnShowLabelTable(bool);

  void OnGoToLabel(const mitk::Point3D &pos);

  void OnLabelSetWidgetReset();

private:

  void CreateQtPartControl(QWidget* parent) override;

  void SetFocus() override {}
  /**
  * @brief Add an observer for the mitk::Tool::ToolEnteredRenderWindow-event
  *        to the currently active tool.
  *
  * The active tool is retrieved from the tool manager.
  * If the event is not already observerd by the active tool,
  * an observer for this event is added, using a 'ReceptorMemberCommand'.
  * This command uses 'ValidateRendererGeometry' as a callback function,
  * and will call this function, if the event is invoked.
  */
  void ActiveToolChanged();
  /**
  * @brief Test whether the geometry of the reference image
  *        fits the world geometry of the respective renderer.
  *
  * The respective renderer is retrieved from the given event, which
  * needs to be a 'ToolEnteredRenderWindow'. This event provides not only
  * the sending base renderer but also a bool that indicates whether
  * the mouse cursor entered or left the base renderer.
  * This information is used to compare the renderer's world geometry
  * with the oriented time geometry of the current reference image.
  * If the geometries align, the renderer is not blocked anymore and the
  * view's warning message is removed.
  * If the geometries do not align, 'BlockRenderer' is called and a
  * warning message is added to the top of the plugin view.
  *
  * @param event   The observed mitk::Tool::ToolEnteredRenderWindow-event.
  */
  void ValidateRendererGeometry(const itk::EventObject& event);
  /**
  * @brief Block a base renderer for segmentation.
  *
  * This function will add an overlay image to the given base renderer
  * to indicate that the renderer should not be used for interactive
  * image segmentation.
  *
  * @param baseRenderer   The base renderer to "block"
  * @param blocked        If true, the overlay image will be added.
  *                       If false, the overlay image will be removed.
  */
  void BlockRenderer(mitk::BaseRenderer* baseRenderer, bool blocked);

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  void OnPreferencesChanged(const berry::IBerryPreferences* prefs) override;

  void NodeAdded(const mitk::DataNode* node) override;

  void NodeRemoved(const mitk::DataNode* node) override;

  void EstablishLabelSetConnection();

  void LooseLabelSetConnection();

  void OnAnySelectionChanged();

  // make sure all images / segmentations look according to the user preference settings
  void ApplyDisplayOptions();

  // decorates a DataNode according to the user preference settings
  void ApplyDisplayOptions(mitk::DataNode* node);

  // If a contourmarker is selected, the plane in the related widget will be reoriented according to the marker`s geometry
  void OnContourMarkerSelected(const mitk::DataNode* node);

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes) override;

  void ResetMouseCursor();

  void SetMouseCursor(const us::ModuleResource&, int hotspotX, int hotspotY);

  void UpdateGUI();

  void ValidateSelectionInput();

  void UpdateWarningLabel(QString text);

  std::string GetDefaultLabelSetPreset() const;

  QWidget* m_Parent;

  Ui::QmitkSegmentationViewControls* m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart;

  mitk::ToolManager* m_ToolManager;

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;

  mitk::SegmentationInteractor::Pointer m_SegmentationInteractor;

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;
  NodeTagMapType m_WorkingDataObserverTags;
  NodeTagMapType m_ReferenceDataObserverTags;
  unsigned int m_RenderingManagerObserverTag;

  mitk::NodePredicateAnd::Pointer m_ReferencePredicate;
  mitk::NodePredicateAnd::Pointer m_SegmentationPredicate;

  mitk::LogoAnnotation::Pointer m_RendererAnnotation;

  bool m_DrawOutline;
  bool m_SelectionMode;
  bool m_MouseCursorSet;

  QString m_LabelSetPresetPreference;
  bool m_DefaultLabelNaming;
};

#endif // QMITKSEGMENTATIONVIEW_H
