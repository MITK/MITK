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

//Qmitk headers
#include "QmitkMlbstTrainingDataWidget.h"

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateIsDWI.h>
#include <mitkFiberBundle.h>

const std::string QmitkMlbstTrainingDataWidget::VIEW_ID = "org.mitk.views.DotModelParametersWidget";

QmitkMlbstTrainingDataWidget::QmitkMlbstTrainingDataWidget( QWidget * parent, Qt::WindowFlags )
    : QWidget(parent)
{
    m_Controls = nullptr;
    this->CreateQtPartControl(this);
}

QmitkMlbstTrainingDataWidget::~QmitkMlbstTrainingDataWidget()
{
}

void QmitkMlbstTrainingDataWidget::CreateQtPartControl(QWidget *parent)
{
    if (!m_Controls)
    {
        // create GUI widgets
        m_Controls = new Ui::QmitkMlbstTrainingDataWidgetControls;
        m_Controls->setupUi(parent);

        mitk::NodePredicateIsDWI::Pointer isDiffusionImage = mitk::NodePredicateIsDWI::New();

        mitk::TNodePredicateDataType<mitk::FiberBundle>::Pointer isFib = mitk::TNodePredicateDataType<mitk::FiberBundle>::New();

        m_Controls->image->SetPredicate(isDiffusionImage);
        m_Controls->fibers->SetPredicate(isFib);

        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
        mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
        mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
        finalPredicate = mitk::NodePredicateAnd::New(finalPredicate, isBinaryPredicate);

        m_Controls->mask->SetPredicate(finalPredicate);
        m_Controls->whiteMatter->SetPredicate(finalPredicate);
    }
}
