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

#ifndef QmitkSegmentationUtilities2View_h
#define QmitkSegmentationUtilities2View_h

#include <ui_QmitkSegmentationUtilities2ViewControls.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkAbstractView.h>

class QmitkBooleanOperationsWidget;
class QmitkImageMaskingWidget;
class QmitkMorphologicalOperationsWidget;

class QmitkSegmentationUtilities2View : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  QmitkSegmentationUtilities2View();
  ~QmitkSegmentationUtilities2View();

  void CreateQtPartControl(QWidget* parent);
  void SetFocus();

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart);
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart);

private:
  void AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text);

  Ui::QmitkSegmentationUtilities2ViewControls m_Controls;
  QmitkBooleanOperationsWidget* m_BooleanOperationsWidget;
  QmitkImageMaskingWidget* m_ImageMaskingWidget;
  QmitkMorphologicalOperationsWidget* m_MorphologicalOperationsWidget;
};

#endif
