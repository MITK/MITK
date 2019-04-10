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
  ~QmitkMultiLabelSegmentationUtilitiesView();

  void CreateQtPartControl(QWidget* parent);
  void SetFocus();

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);

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
