/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMorphologicalOperationsWidget_h
#define QmitkMorphologicalOperationsWidget_h

#include <MitkSegmentationUIExports.h>

#include <mitkMorphologicalOperations.h>
#include <mitkRenderingManager.h>

#include <QmitkAbstractNodeSelectionWidget.h>

#include <QWidget>

namespace Ui
{
  class QmitkMorphologicalOperationsWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
}

/** \brief GUI class for morphological segmentation tools.
 */
class MITKSEGMENTATIONUI_EXPORT QmitkMorphologicalOperationsWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkMorphologicalOperationsWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);
  ~QmitkMorphologicalOperationsWidget() override;

public slots:
  void OnClosingButtonClicked();
  void OnOpeningButtonClicked();
  void OnDilatationButtonClicked();
  void OnErosionButtonClicked();
  void OnFillHolesButtonClicked();
  void OnSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList nodes);
  void OnRadioButtonsClicked();

protected:
  void ConfigureButtons();

  using MorphFunctionType = void(mitk::Image::Pointer& image, int factor,
    mitk::MorphologicalOperations::StructuralElementType structuralElement);
  void Processing(std::function<MorphFunctionType> morphFunction, const std::string& opsName) const;

private:
  mitk::Image::Pointer GetSelectedLabelMask() const;
  void SaveResultLabelMask(const mitk::Image* resultMask, const std::string& labelName) const;

  mitk::MorphologicalOperations::StructuralElementType CreateStructerElement_UI() const;

  Ui::QmitkMorphologicalOperationsWidgetControls* m_Controls;
};

#endif
