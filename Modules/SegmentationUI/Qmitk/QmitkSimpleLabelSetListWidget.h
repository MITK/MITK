/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSimpleLabelSetListWidget_h
#define QmitkSimpleLabelSetListWidget_h

#include "mitkLabel.h"
#include "mitkLabelSetImage.h"
#include <MitkSegmentationUIExports.h>
#include <QListWidget>

/**
  \brief Widget that offers a simple list that displays all labels (color and name) in the active
  layer of a LabelSetImage.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkSimpleLabelSetListWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkSimpleLabelSetListWidget(QWidget* parent = nullptr);
  ~QmitkSimpleLabelSetListWidget() override;

  using LabelVectorType = std::vector<mitk::Label::ConstPointer>;

  LabelVectorType SelectedLabels() const;
  const mitk::LabelSetImage* GetLabelSetImage() const;

signals:
  void SelectedLabelsChanged(const LabelVectorType& selectedLabels);
  void ActiveLayerChanged();

public slots :
  void SetLabelSetImage(const mitk::LabelSetImage* image);
  void SetSelectedLabels(const LabelVectorType& selectedLabels);

protected slots:

  void OnLabelSelectionChanged();

protected:
  void OnLayerChanged();
  void OnLabelChanged(mitk::LabelSetImage::LabelValueType lv);

  void OnLooseLabelSetConnection();
  void OnEstablishLabelSetConnection();

  void ResetList();

  mitk::LabelSetImage::ConstPointer m_LabelSetImage;
  QListWidget* m_LabelList;
  bool m_Emmiting;
};

#endif
