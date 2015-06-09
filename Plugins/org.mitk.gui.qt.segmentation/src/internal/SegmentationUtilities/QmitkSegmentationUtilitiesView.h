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

#ifndef QmitkSegmentationUtilitiesView_h
#define QmitkSegmentationUtilitiesView_h

#include <ui_QmitkSegmentationUtilitiesViewControls.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkAbstractView.h>

class QmitkBooleanOperationsWidget;
class QmitkContourModelToImageWidget;
class QmitkImageMaskingWidget;
class QmitkMorphologicalOperationsWidget;
class QmitkSurfaceToImageWidget;
class QmitkCTKWidgetsWidget;

class QmitkSegmentationUtilitiesView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  QmitkSegmentationUtilitiesView();
  ~QmitkSegmentationUtilitiesView();

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

private:
  void AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text);

  Ui::QmitkSegmentationUtilitiesViewControls m_Controls;
  QmitkBooleanOperationsWidget* m_BooleanOperationsWidget;
  QmitkContourModelToImageWidget* m_ContourModelToImageWidget;
  QmitkImageMaskingWidget* m_ImageMaskingWidget;
  QmitkMorphologicalOperationsWidget* m_MorphologicalOperationsWidget;
  QmitkSurfaceToImageWidget* m_SurfaceToImageWidget;
  QmitkCTKWidgetsWidget* m_CTKWidgetsWidget;
};

#endif
