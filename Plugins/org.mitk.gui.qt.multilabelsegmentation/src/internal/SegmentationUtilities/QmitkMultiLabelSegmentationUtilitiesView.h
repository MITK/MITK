/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelSegmentationUtilitiesView_h
#define QmitkMultiLabelSegmentationUtilitiesView_h

#include <ui_QmitkMultiLabelSegmentationUtilitiesViewControls.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkAbstractView.h>

class QmitkBooleanOperationsWidget;
class QmitkSurfaceToImageWidget;
class QmitkImageMaskingWidget;
class QmitkMorphologicalOperationsWidget;
class QmitkMorphologicalOperationsWidget;
class QmitkConvertToMlWidget;

class QmitkMultiLabelSegmentationUtilitiesView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  QmitkMultiLabelSegmentationUtilitiesView();
  ~QmitkMultiLabelSegmentationUtilitiesView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

private:
  void AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text);

  QmitkBooleanOperationsWidget* m_BooleanOperationsWidget;

  QmitkMorphologicalOperationsWidget* m_MorphologicalOperationsWidget;

  QmitkSurfaceToImageWidget* m_SurfaceToImageWidget;

  QmitkImageMaskingWidget* m_ImageMaskingWidget;

  QmitkConvertToMlWidget* m_ConvertToMlWidget;

  Ui::QmitkMultiLabelSegmentationUtilitiesViewControls m_Controls;
};

#endif
