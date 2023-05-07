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
#include <QmitkSegmentationUtilityWidget.h>

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
class MITKSEGMENTATIONUI_EXPORT QmitkMorphologicalOperationsWidget : public QmitkSegmentationUtilityWidget
{
  Q_OBJECT

public:
  explicit QmitkMorphologicalOperationsWidget(mitk::DataStorage* dataStorage,
                                              mitk::SliceNavigationController* timeNavigationController,
                                              QWidget* parent = nullptr);
  ~QmitkMorphologicalOperationsWidget() override;

public slots:
  void OnClosingButtonClicked();
  void OnOpeningButtonClicked();
  void OnDilatationButtonClicked();
  void OnErosionButtonClicked();
  void OnFillHolesButtonClicked();
  void OnSelectionChanged(unsigned int index, const mitk::DataNode* selection);
  void OnRadioButtonsClicked();

protected:
  void EnableButtons(bool enable);

private:
  Ui::QmitkMorphologicalOperationsWidgetControls* m_Controls;
  mitk::MorphologicalOperations::StructuralElementType CreateStructerElement_UI();
};

#endif
