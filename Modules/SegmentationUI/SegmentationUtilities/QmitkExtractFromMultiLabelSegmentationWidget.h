/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkExtractFromMultiLabelSegmentationWidget_h
#define QmitkExtractFromMultiLabelSegmentationWidget_h

#include <MitkSegmentationUIExports.h>
#include <QmitkNodeSelectionDialog.h>
#include <QmitkAbstractNodeSelectionWidget.h>

#include "itkSmartPointer.h"

#include <QWidget>

namespace Ui
{
  class QmitkExtractFromMultiLabelSegmentationWidgetControls;
}

namespace mitk
{
  class DataNode;
  class DataStorage;
  class Surface;
  class Image;
  class LabelSetImage;
}

/*!
  \brief QmitkExtractFromMultiLabelSegmentationWidget

  Widget that offers the GUI and logic to extract different images (class maps, instance maps
  and instance masks) from a multi label segmentation.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkExtractFromMultiLabelSegmentationWidget : public QWidget
{
  Q_OBJECT

public:

  /** @brief Default constructor, including creation of GUI elements and signals/slots connections. */
  explicit QmitkExtractFromMultiLabelSegmentationWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);

  /** @brief Default destructor. */
  ~QmitkExtractFromMultiLabelSegmentationWidget() override;

private slots:

  /** @brief This slot is called if the selection in the workbench is changed. */
  void OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/);

  /** @brief This slot is called if user activates the button to convert a surface into a binary image. */
  void OnExtractPressed();

private:
  bool m_InternalEvent = false;
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

  void ConfigureWidgets();

  void StoreToDataStorage(mitk::Image* image, const std::string& name, mitk::DataNode* parent);

  Ui::QmitkExtractFromMultiLabelSegmentationWidgetControls* m_Controls;
};

#endif
