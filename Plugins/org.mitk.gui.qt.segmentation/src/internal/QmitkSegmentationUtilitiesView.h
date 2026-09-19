/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentationUtilitiesView_h
#define QmitkSegmentationUtilitiesView_h

#include <mitkIRenderWindowPartListener.h>
#include <mitkDataNode.h>
#include <QmitkAbstractView.h>

#include <QList>
#include <memory>

namespace Ui
{
  class QmitkSegmentationUtilitiesViewControls;
}

class QmitkBooleanOperationsWidget;
class QmitkImageMaskingWidget;
class QmitkMorphologicalOperationsWidget;
class QmitkConvertToMultiLabelSegmentationWidget;
class QmitkExtractFromMultiLabelSegmentationWidget;

class QmitkSegmentationUtilitiesView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  QmitkSegmentationUtilitiesView();
  ~QmitkSegmentationUtilitiesView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

private:
  void AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text);

  /** \brief Selects the newly created utility result node(s) in the Data Manager. */
  void OnNewResultsReady(const QList<mitk::DataNode::Pointer>& nodes);

  std::unique_ptr<Ui::QmitkSegmentationUtilitiesViewControls> m_Controls;
  QmitkBooleanOperationsWidget* m_BooleanOperationsWidget;
  QmitkImageMaskingWidget* m_ImageMaskingWidget;
  QmitkMorphologicalOperationsWidget* m_MorphologicalOperationsWidget;
  QmitkConvertToMultiLabelSegmentationWidget* m_ConvertToSegWidget;
  QmitkExtractFromMultiLabelSegmentationWidget* m_ExtractFromSegWidget;
};

#endif
