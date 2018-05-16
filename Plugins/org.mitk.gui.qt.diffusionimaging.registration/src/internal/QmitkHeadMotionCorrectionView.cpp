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

//misc
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkHeadMotionCorrectionView.h"

// MITK
#include <mitkDiffusionPropertyHelper.h>
#include <mitkNodePredicateDataType.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QTimer>

#include <mitkDWIHeadMotionCorrectionFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkHeadMotionCorrectionView::VIEW_ID = "org.mitk.views.headmotioncorrectionview";

QmitkHeadMotionCorrectionView::QmitkHeadMotionCorrectionView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_DiffusionImage( nullptr )
{

}

// Destructor
QmitkHeadMotionCorrectionView::~QmitkHeadMotionCorrectionView()
{
}

void QmitkHeadMotionCorrectionView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkHeadMotionCorrectionViewControls;
    m_Controls->setupUi( parent );
    connect( m_Controls->m_ImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    connect( m_Controls->m_RegistrationStartButton, SIGNAL(clicked()), this, SLOT(StartRegistration()) );
    this->m_Parent = parent;

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
    m_Controls->m_ImageBox->SetPredicate(isImagePredicate);
    UpdateGUI();
  }
}

void QmitkHeadMotionCorrectionView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& )
{

}

void QmitkHeadMotionCorrectionView::UpdateGUI()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    m_Controls->m_RegistrationStartButton->setEnabled(true);
    m_Controls->m_RegistrationStartButton->setToolTip("Start Registration");
  }
  else
  {
    m_Controls->m_RegistrationStartButton->setEnabled(false);
    m_Controls->m_RegistrationStartButton->setToolTip("No Diffusion image selected.");
  }
}

void QmitkHeadMotionCorrectionView::SetFocus()
{
  UpdateGUI();
  m_Controls->m_RegistrationStartButton->setFocus();
}

void QmitkHeadMotionCorrectionView::StartRegistration()
{
  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  mitk::Image::Pointer inImage = dynamic_cast<mitk::Image*>(node->GetData());

  if (!mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(inImage))
  {
    QMessageBox::warning(nullptr, "Image not processed", "Input is not a diffusion-weighted image!");
    return;
  }

  DWIHeadMotionCorrectionFilterType::Pointer registerer = DWIHeadMotionCorrectionFilterType::New();
  registerer->SetInput(inImage);
  registerer->Update();
  mitk::Image::Pointer image = registerer->GetCorrectedImage();
  mitk::DataNode::Pointer corrected_node = mitk::DataNode::New();
  corrected_node->SetData( image );
  QString name(node->GetName().c_str());
  name += "_Corrected";
  corrected_node->SetName(name.toStdString());
  GetDataStorage()->Add(corrected_node, node);
}
