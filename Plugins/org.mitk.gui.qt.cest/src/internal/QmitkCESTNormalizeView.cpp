/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCESTNormalizeView.h"

#include <QMessageBox>

#include "mitkWorkbenchUtil.h"

#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateDataProperty.h"
#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBoxWithSelectNone.h"

#include <mitkImage.h>
#include "mitkCESTImageNormalizationFilter.h"
#include "mitkCESTPropertyHelper.h"
#include "mitkCESTImageDetectionHelper.h"

const std::string QmitkCESTNormalizeView::VIEW_ID = "org.mitk.views.cest.normalization";

void QmitkCESTNormalizeView::SetFocus()
{
  m_Controls.btnNormalize->setFocus();
}

void QmitkCESTNormalizeView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  m_Controls.btnNormalize->setEnabled(false);

  m_Controls.comboCESTImage->SetPredicate(this->m_IsCESTImagePredicate);
  m_Controls.comboCESTImage->SetDataStorage(this->GetDataStorage());

  connect(m_Controls.btnNormalize, SIGNAL(clicked()), this, SLOT(OnNormalizeButtonClicked()));
  connect(m_Controls.comboCESTImage, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(UpdateGUIControls()));

  UpdateGUIControls();
}

void QmitkCESTNormalizeView::UpdateGUIControls()
{
    m_Controls.btnNormalize->setEnabled(m_Controls.comboCESTImage->GetSelectedNode().IsNotNull());
}

void QmitkCESTNormalizeView::OnNormalizeButtonClicked()
{
    auto selectedImageNode = m_Controls.comboCESTImage->GetSelectedNode();
    if (!selectedImageNode)
    {
      MITK_ERROR << "Invalid system state. CEST selection is invalid. Selected node is null_ptr.";
      return;
    }

    auto selectedImage = dynamic_cast<mitk::Image*>(selectedImageNode->GetData());
    if (!selectedImageNode)
    {
      MITK_ERROR << "Invalid system state. CEST selection is invalid. Selected node is not an image.";
      return;
    }

    std::string offsetsStr = "";
    bool hasOffsets = selectedImage->GetPropertyList()->GetStringProperty(mitk::CEST_PROPERTY_NAME_OFFSETS().c_str(), offsetsStr);
    if (!hasOffsets)
    {
      QMessageBox::information(nullptr, "CEST normalization", "Selected image was missing CEST offset information.");
      return;
    }

    if (!mitk::IsNotNormalizedCESTImage(selectedImage))
    {
      QMessageBox::information(nullptr, "CEST normalization", "Selected image already seems to be normalized.");
      return;
    }

    if (selectedImage->GetDimension() == 4)
    {
      auto normalizationFilter = mitk::CESTImageNormalizationFilter::New();
      normalizationFilter->SetInput(selectedImage);
      normalizationFilter->Update();

      auto resultImage = normalizationFilter->GetOutput();

      mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
      dataNode->SetData(resultImage);

      std::string normalizedName = selectedImageNode->GetName() + "_normalized";
      dataNode->SetName(normalizedName);

      this->GetDataStorage()->Add(dataNode);
    }
}

QmitkCESTNormalizeView::QmitkCESTNormalizeView()
{
  auto isImage = mitk::NodePredicateDataType::New("Image");

  this->m_IsCESTImagePredicate = mitk::NodePredicateAnd::New(isImage, mitk::CreateAnyCESTImageNodePredicate()).GetPointer();
}
