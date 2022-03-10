/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_matchpoint_visualizer_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Mitk
#include <mitkStatusBar.h>
#include <mitkProperties.h>
#include <mitkColorProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataProperty.h>
#include <mitkNodePredicateFunction.h>
#include "mitkRegVisDirectionProperty.h"
#include "mitkRegVisStyleProperty.h"
#include "mitkRegVisColorStyleProperty.h"
#include "mitkRegVisPropertyTags.h"
#include "mitkRegVisHelper.h"
#include "mitkMatchPointPropertyTags.h"
#include "mitkRegistrationHelper.h"

// Qmitk
#include "QmitkMatchPointRegistrationVisualizer.h"

// Qt
#include <QMessageBox>
#include <QErrorMessage>

const std::string QmitkMatchPointRegistrationVisualizer::VIEW_ID =
"org.mitk.views.matchpoint.visualizer";

QmitkMatchPointRegistrationVisualizer::QmitkMatchPointRegistrationVisualizer()
    : m_Parent(nullptr), m_internalUpdateGuard(false), m_spSelectedFOVRefNode(nullptr),
    m_spSelectedRegNode(nullptr)
{
}

void QmitkMatchPointRegistrationVisualizer::SetFocus()
{
}

void QmitkMatchPointRegistrationVisualizer::CreateConnections()
{
    connect(m_Controls->registrationNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationVisualizer::OnNodeSelectionChanged);
    connect(m_Controls->fovReferenceNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkMatchPointRegistrationVisualizer::OnNodeSelectionChanged);

    connect(m_Controls->m_pbStyleGrid, SIGNAL(clicked()), this, SLOT(OnStyleButtonPushed()));
    connect(m_Controls->m_pbStyleGlyph, SIGNAL(clicked()), this, SLOT(OnStyleButtonPushed()));
    connect(m_Controls->m_pbStylePoints, SIGNAL(clicked()), this, SLOT(OnStyleButtonPushed()));

    connect(m_Controls->m_comboDirection, SIGNAL(currentIndexChanged(int)), this,
        SLOT(OnDirectionChanged(int)));
    connect(m_Controls->m_pbUpdateViz, SIGNAL(clicked()), this, SLOT(OnUpdateBtnPushed()));

    connect(m_Controls->radioColorUni, SIGNAL(toggled(bool)), m_Controls->btnUniColor,
        SLOT(setEnabled(bool)));
    connect(m_Controls->radioColorVecMag, SIGNAL(toggled(bool)), m_Controls->groupColorCoding,
        SLOT(setEnabled(bool)));

    connect(m_Controls->m_pbStyleGrid, SIGNAL(toggled(bool)), m_Controls->tabGrid,
        SLOT(setEnabled(bool)));

    connect(m_Controls->cbVevMagInterlolate, SIGNAL(toggled(bool)), this,
        SLOT(OnColorInterpolationChecked(bool)));

    connect(m_Controls->m_checkUseRefSize, SIGNAL(clicked()), this, SLOT(TransferFOVRefGeometry()));
    connect(m_Controls->m_checkUseRefSpacing, SIGNAL(clicked()), this, SLOT(TransferFOVRefGeometry()));
    connect(m_Controls->m_checkUseRefOrigin, SIGNAL(clicked()), this, SLOT(TransferFOVRefGeometry()));
    connect(m_Controls->m_checkUseRefOrientation, SIGNAL(clicked()), this, SLOT(TransferFOVRefGeometry()));
}

void QmitkMatchPointRegistrationVisualizer::Error(QString msg)
{
    mitk::StatusBar::GetInstance()->DisplayErrorText(msg.toLatin1());
    MITK_ERROR << msg.toStdString().c_str();
}

void QmitkMatchPointRegistrationVisualizer::CreateQtPartControl(QWidget* parent)
{
    m_Controls = new Ui::MatchPointRegVisControls;

    // create GUI widgets from the Qt Designer's .ui file
    m_Controls->setupUi(parent);

    m_Parent = parent;

    this->m_Controls->registrationNodeSelector->SetDataStorage(this->GetDataStorage());
    this->m_Controls->registrationNodeSelector->SetSelectionIsOptional(false);
    this->m_Controls->fovReferenceNodeSelector->SetDataStorage(this->GetDataStorage());
    this->m_Controls->fovReferenceNodeSelector->SetSelectionIsOptional(false);
    m_Controls->registrationNodeSelector->SetInvalidInfo("Select registration.");
    m_Controls->registrationNodeSelector->SetPopUpTitel("Select registration.");
    m_Controls->registrationNodeSelector->SetPopUpHint("Select the registration object whose registration visualization should be edited.");
    m_Controls->fovReferenceNodeSelector->SetInvalidInfo("Select a FOV reference image.");
    m_Controls->fovReferenceNodeSelector->SetPopUpTitel("Select a FOV reference image.");
    m_Controls->fovReferenceNodeSelector->SetPopUpHint("Select the the image that should be used to define the field of view (FOV) for the registration visualization. The visualization will use the image geometry (size, orientation, spacing...).");

    this->ConfigureNodePredicates();

    this->m_Controls->btnVecMagColorSmall->setDisplayColorName(false);
    this->m_Controls->btnVecMagColorMedium->setDisplayColorName(false);
    this->m_Controls->btnVecMagColorLarge->setDisplayColorName(false);
    this->m_Controls->btnVecMagColorNeg->setDisplayColorName(false);
    this->m_Controls->btnUniColor->setDisplayColorName(false);
    this->m_Controls->btnStartGridColor->setDisplayColorName(false);

    this->CreateConnections();

    this->m_Controls->radioColorUni->setChecked(false);
    this->m_Controls->radioColorVecMag->setChecked(true);

    this->CheckInputs();
    this->LoadStateFromNode();
    this->ConfigureVisualizationControls();

    //deactivate because currently not an implemented style
    this->m_Controls->m_pbStylePoints->setVisible(false);
}

void QmitkMatchPointRegistrationVisualizer::ConfigureNodePredicates()
{
  m_Controls->registrationNodeSelector->SetNodePredicate(mitk::MITKRegistrationHelper::RegNodePredicate());

  auto geometryCheck = [](const mitk::DataNode * node)
  {
    return node->GetData() && node->GetData()->GetGeometry();
  };
  mitk::NodePredicateFunction::Pointer hasGeometry = mitk::NodePredicateFunction::New(geometryCheck);

  auto nodePredicate = mitk::NodePredicateAnd::New(mitk::MITKRegistrationHelper::ImageNodePredicate().GetPointer(), hasGeometry.GetPointer());
  m_Controls->fovReferenceNodeSelector->SetNodePredicate(nodePredicate.GetPointer());
}

mitk::MAPRegistrationWrapper* QmitkMatchPointRegistrationVisualizer::GetCurrentRegistration()
{
    mitk::MAPRegistrationWrapper* result = nullptr;

    if (this->m_spSelectedRegNode.IsNotNull())
    {
        result = dynamic_cast<mitk::MAPRegistrationWrapper*>(this->m_spSelectedRegNode->GetData());
        assert(result);
    }

    return result;
}

mitk::DataNode::Pointer QmitkMatchPointRegistrationVisualizer::GetSelectedRegNode() const
{
    return m_Controls->registrationNodeSelector->GetSelectedNode();
}

mitk::DataNode::Pointer QmitkMatchPointRegistrationVisualizer::GetRefNodeOfReg(bool target) const
{
    mitk::DataNode::Pointer spResult = nullptr;

    if (this->m_spSelectedRegNode.IsNotNull() && m_spSelectedRegNode->GetData())
    {
        std::string nodeName;
        mitk::BaseProperty* uidProp;

        if (target)
        {
          uidProp = m_spSelectedRegNode->GetData()->GetProperty(mitk::Prop_RegAlgTargetData);
        }
        else
        {
          uidProp = m_spSelectedRegNode->GetData()->GetProperty(mitk::Prop_RegAlgMovingData);
        }

        if (uidProp)
        {
            //search for the target node
            mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(mitk::Prop_UID,
                uidProp);
            spResult = this->GetDataStorage()->GetNode(predicate);
        }
    }

    return spResult;
}

mitk::DataNode::Pointer QmitkMatchPointRegistrationVisualizer::GetSelectedDataNode()
{
  return m_Controls->fovReferenceNodeSelector->GetSelectedNode();
}

void QmitkMatchPointRegistrationVisualizer::CheckInputs()
{
  this->m_spSelectedRegNode = this->GetSelectedRegNode();

  this->InitRegNode();

  this->m_spSelectedFOVRefNode = this->GetSelectedDataNode();
}

void QmitkMatchPointRegistrationVisualizer::ConfigureVisualizationControls()
{
    if (!m_internalUpdateGuard)
    {
        m_internalUpdateGuard = true;
        m_Controls->groupViz->setVisible(this->m_spSelectedRegNode.IsNotNull());

        m_Controls->m_pbUpdateViz->setEnabled(this->m_spSelectedRegNode.IsNotNull());
        m_Controls->m_boxSettings->setEnabled(this->m_spSelectedRegNode.IsNotNull());
        m_Controls->m_boxStyles->setEnabled(this->m_spSelectedRegNode.IsNotNull());

        this->ActualizeRegInfo(this->GetCurrentRegistration());

        this->m_Controls->m_checkUseRefSize->setEnabled(this->m_spSelectedRegNode.IsNotNull()
            && this->m_spSelectedFOVRefNode.IsNotNull());
        this->m_Controls->m_checkUseRefOrigin->setEnabled(this->m_spSelectedRegNode.IsNotNull()
            && this->m_spSelectedFOVRefNode.IsNotNull());
        this->m_Controls->m_checkUseRefSpacing->setEnabled(this->m_spSelectedRegNode.IsNotNull()
            && this->m_spSelectedFOVRefNode.IsNotNull());

        m_internalUpdateGuard = false;
    }
}

void QmitkMatchPointRegistrationVisualizer::StoreStateInNode()
{
    if (this->m_spSelectedRegNode.IsNotNull())
    {
        //general
        this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisDirection,
            mitk::RegVisDirectionProperty::New(this->m_Controls->m_comboDirection->currentIndex()));

        this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisGrid,
            this->m_Controls->m_pbStyleGrid->isChecked());
        this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisGlyph,
            this->m_Controls->m_pbStyleGlyph->isChecked());
        this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisPoints,
            this->m_Controls->m_pbStylePoints->isChecked());

        //Visualization
        if (this->m_Controls->radioColorUni->isChecked())
        {
            this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisColorStyle,
                mitk::RegVisColorStyleProperty::New(0));
        }
        else
        {
            this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisColorStyle,
                mitk::RegVisColorStyleProperty::New(1));
        }

        float tmpColor[3];

        tmpColor[0] = this->m_Controls->btnUniColor->color().redF();
        tmpColor[1] = this->m_Controls->btnUniColor->color().greenF();
        tmpColor[2] = this->m_Controls->btnUniColor->color().blueF();
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorUni,
            mitk::ColorProperty::New(tmpColor), nullptr, true);

        tmpColor[0] = this->m_Controls->btnVecMagColorNeg->color().redF();
        tmpColor[1] = this->m_Controls->btnVecMagColorNeg->color().greenF();
        tmpColor[2] = this->m_Controls->btnVecMagColorNeg->color().blueF();
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor1Value,
            mitk::ColorProperty::New(tmpColor), nullptr, true);

        tmpColor[0] = this->m_Controls->btnVecMagColorSmall->color().redF();
        tmpColor[1] = this->m_Controls->btnVecMagColorSmall->color().greenF();
        tmpColor[2] = this->m_Controls->btnVecMagColorSmall->color().blueF();
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Value,
            mitk::ColorProperty::New(tmpColor), nullptr, true);
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Magnitude,
            mitk::DoubleProperty::New(this->m_Controls->sbVecMagSmall->value()), nullptr, true);

        tmpColor[0] = this->m_Controls->btnVecMagColorMedium->color().redF();
        tmpColor[1] = this->m_Controls->btnVecMagColorMedium->color().greenF();
        tmpColor[2] = this->m_Controls->btnVecMagColorMedium->color().blueF();
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Value,
            mitk::ColorProperty::New(tmpColor), nullptr, true);
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Magnitude,
            mitk::DoubleProperty::New(this->m_Controls->sbVecMagMedium->value()), nullptr, true);

        tmpColor[0] = this->m_Controls->btnVecMagColorLarge->color().redF();
        tmpColor[1] = this->m_Controls->btnVecMagColorLarge->color().greenF();
        tmpColor[2] = this->m_Controls->btnVecMagColorLarge->color().blueF();
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Value,
            mitk::ColorProperty::New(tmpColor), nullptr, true);
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Magnitude,
            mitk::DoubleProperty::New(this->m_Controls->sbVecMagLarge->value()), nullptr, true);

        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorInterpolate,
            mitk::BoolProperty::New(this->m_Controls->cbVevMagInterlolate->isChecked()), nullptr, true);

        //Grid Settings
        this->m_spSelectedRegNode->SetIntProperty(mitk::nodeProp_RegVisGridFrequence,
            this->m_Controls->m_sbGridFrequency->value());
        this->m_spSelectedRegNode->SetBoolProperty(mitk::nodeProp_RegVisGridShowStart,
            this->m_Controls->m_groupShowStartGrid->isChecked());
        tmpColor[0] = this->m_Controls->btnStartGridColor->color().redF();
        tmpColor[1] = this->m_Controls->btnStartGridColor->color().greenF();
        tmpColor[2] = this->m_Controls->btnStartGridColor->color().blueF();
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridStartColor,
            mitk::ColorProperty::New(tmpColor), nullptr, true);

        //FOV
        mitk::Vector3D value;
        value[0] = this->m_Controls->m_sbFOVSizeX->value();
        value[1] = this->m_Controls->m_sbFOVSizeY->value();
        value[2] = this->m_Controls->m_sbFOVSizeZ->value();
        this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVSize,
            mitk::Vector3DProperty::New(value));

        value[0] = this->m_Controls->m_sbGridSpX->value();
        value[1] = this->m_Controls->m_sbGridSpY->value();
        value[2] = this->m_Controls->m_sbGridSpZ->value();
        this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVSpacing,
            mitk::Vector3DProperty::New(value));

        mitk::Point3D origin;
        origin[0] = this->m_Controls->m_sbFOVOriginX->value();
        origin[1] = this->m_Controls->m_sbFOVOriginY->value();
        origin[2] = this->m_Controls->m_sbFOVOriginZ->value();
        this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrigin,
            mitk::Point3dProperty::New(origin));

        mitk::Vector3D orientationRow1;
        mitk::Vector3D orientationRow2;
        mitk::Vector3D orientationRow3;
        orientationRow1.SetVnlVector(m_FOVRefOrientation.GetVnlMatrix().get_row(0).as_ref());
        orientationRow2.SetVnlVector(m_FOVRefOrientation.GetVnlMatrix().get_row(1).as_ref());
        orientationRow3.SetVnlVector(m_FOVRefOrientation.GetVnlMatrix().get_row(2).as_ref());
        this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrientation1,
            mitk::Vector3DProperty::New(orientationRow1));
        this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrientation2,
            mitk::Vector3DProperty::New(orientationRow2));
        this->m_spSelectedRegNode->SetProperty(mitk::nodeProp_RegVisFOVOrientation3,
            mitk::Vector3DProperty::New(orientationRow3));

    }
}

void QmitkMatchPointRegistrationVisualizer::LoadStateFromNode()
{
    if (this->m_spSelectedRegNode.IsNotNull())
    {
        mitk::RegVisDirectionProperty* directionProp = nullptr;

        if (this->m_spSelectedRegNode->GetProperty(directionProp, mitk::nodeProp_RegVisDirection))
        {
            this->m_Controls->m_comboDirection->setCurrentIndex(directionProp->GetValueAsId());
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisDirection) + QStringLiteral(" has not the assumed type."));
        }

        bool styleActive = false;

        if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisGrid, styleActive))
        {
            this->m_Controls->m_pbStyleGrid->setChecked(styleActive);
            this->m_Controls->tabGrid->setEnabled(styleActive);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisGrid) + QStringLiteral(" has not the assumed type."));
        }

        if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisGlyph, styleActive))
        {
            this->m_Controls->m_pbStyleGlyph->setChecked(styleActive);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisGlyph) + QStringLiteral(" has not the assumed type."));
        }

        if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisPoints, styleActive))
        {
            this->m_Controls->m_pbStylePoints->setChecked(styleActive);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisPoints) + QStringLiteral(" has not the assumed type."));
        }

        ///////////////////////////////////////////////////////
        //visualization
        mitk::RegVisColorStyleProperty* colorStyleProp = nullptr;

        if (this->m_spSelectedRegNode->GetProperty(colorStyleProp, mitk::nodeProp_RegVisColorStyle))
        {
            this->m_Controls->radioColorUni->setChecked(colorStyleProp->GetValueAsId() == 0);
            this->m_Controls->radioColorVecMag->setChecked(colorStyleProp->GetValueAsId() == 1);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisColorStyle) + QStringLiteral(" has not the assumed type."));
        }

        QColor tmpColor;
        float colorUni[3] = { 0.0, 0.0, 0.0 };
        this->m_spSelectedRegNode->GetColor(colorUni, nullptr, mitk::nodeProp_RegVisColorUni);
        tmpColor.setRgbF(colorUni[0], colorUni[1], colorUni[2]);
        this->m_Controls->btnUniColor->setColor(tmpColor);

        float color1[3] = { 0.0, 0.0, 0.0 };
        this->m_spSelectedRegNode->GetColor(color1, nullptr, mitk::nodeProp_RegVisColor1Value);
        tmpColor.setRgbF(color1[0], color1[1], color1[2]);
        this->m_Controls->btnVecMagColorNeg->setColor(tmpColor);

        float color2[3] = { 0.25, 0.25, 0.25 };
        this->m_spSelectedRegNode->GetColor(color2, nullptr, mitk::nodeProp_RegVisColor2Value);
        tmpColor.setRgbF(color2[0], color2[1], color2[2]);
        this->m_Controls->btnVecMagColorSmall->setColor(tmpColor);

        float color3[3] = { 0.5, 0.5, 0.5 };
        this->m_spSelectedRegNode->GetColor(color3, nullptr, mitk::nodeProp_RegVisColor3Value);
        tmpColor.setRgbF(color3[0], color3[1], color3[2]);
        this->m_Controls->btnVecMagColorMedium->setColor(tmpColor);

        float color4[3] = { 1.0, 1.0, 1.0 };
        this->m_spSelectedRegNode->GetColor(color4, nullptr, mitk::nodeProp_RegVisColor4Value);
        tmpColor.setRgbF(color4[0], color4[1], color4[2]);
        this->m_Controls->btnVecMagColorLarge->setColor(tmpColor);

        double mag2 = 0;
        this->m_spSelectedRegNode->GetPropertyValue(mitk::nodeProp_RegVisColor2Magnitude, mag2);
        double mag3 = 0;
        this->m_spSelectedRegNode->GetPropertyValue(mitk::nodeProp_RegVisColor3Magnitude, mag3);
        double mag4 = 0;
        this->m_spSelectedRegNode->GetPropertyValue(mitk::nodeProp_RegVisColor4Magnitude, mag4);

        bool interpolate = true;
        this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisColorInterpolate, interpolate);

        this->m_Controls->sbVecMagSmall->setValue(mag2);
        this->m_Controls->sbVecMagMedium->setValue(mag3);
        this->m_Controls->sbVecMagLarge->setValue(mag4);

        this->m_Controls->cbVevMagInterlolate->setChecked(interpolate);

        ///////////////////////////////////////////////////////
        //Grid general
        bool showStart = false;

        if (this->m_spSelectedRegNode->GetBoolProperty(mitk::nodeProp_RegVisGridShowStart, showStart))
        {
            this->m_Controls->m_groupShowStartGrid->setChecked(showStart);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisGridShowStart) + QStringLiteral(" is not correctly defined."));
        }

        int gridFrequ = 5;

        if (this->m_spSelectedRegNode->GetIntProperty(mitk::nodeProp_RegVisGridFrequence, gridFrequ))
        {
            this->m_Controls->m_sbGridFrequency->setValue(gridFrequ);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisGridFrequence) + QStringLiteral(" is not correctly defined."));
        }

        float colorStart[3] = { 0.0, 0.0, 0.0 };
        this->m_spSelectedRegNode->GetColor(colorStart, nullptr, mitk::nodeProp_RegVisGridStartColor);
        tmpColor.setRgbF(colorStart[0], colorStart[1], colorStart[2]);
        this->m_Controls->btnStartGridColor->setColor(tmpColor);

        ///////////////////////////////////////////////////////
        //FOV
        mitk::Vector3DProperty* valueProp = nullptr;

        if (this->m_spSelectedRegNode->GetProperty(valueProp, mitk::nodeProp_RegVisFOVSize))
        {
            this->m_Controls->m_sbFOVSizeX->setValue(valueProp->GetValue()[0]);
            this->m_Controls->m_sbFOVSizeY->setValue(valueProp->GetValue()[1]);
            this->m_Controls->m_sbFOVSizeZ->setValue(valueProp->GetValue()[2]);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisFOVSize) + QStringLiteral(" is not correctly defined."));
        }

        if (this->m_spSelectedRegNode->GetProperty(valueProp, mitk::nodeProp_RegVisFOVSpacing))
        {
            this->m_Controls->m_sbGridSpX->setValue(valueProp->GetValue()[0]);
            this->m_Controls->m_sbGridSpY->setValue(valueProp->GetValue()[1]);
            this->m_Controls->m_sbGridSpZ->setValue(valueProp->GetValue()[2]);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisFOVSpacing) + QStringLiteral(" is not correctly defined."));
        }

        mitk::Point3dProperty* originProp = nullptr;

        if (this->m_spSelectedRegNode->GetProperty(originProp, mitk::nodeProp_RegVisFOVOrigin))
        {
            this->m_Controls->m_sbFOVOriginX->setValue(originProp->GetValue()[0]);
            this->m_Controls->m_sbFOVOriginY->setValue(originProp->GetValue()[1]);
            this->m_Controls->m_sbFOVOriginZ->setValue(originProp->GetValue()[2]);
        }
        else
        {
            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. Node property ") + QString(
                mitk::nodeProp_RegVisFOVOrigin) + QStringLiteral(" is not correctly defined."));
        }

        mitk::Vector3DProperty* orientationProp1;
        mitk::Vector3DProperty* orientationProp2;
        mitk::Vector3DProperty* orientationProp3;

        if (this->m_spSelectedRegNode->GetProperty(orientationProp1, mitk::nodeProp_RegVisFOVOrientation1) &&
            this->m_spSelectedRegNode->GetProperty(orientationProp2, mitk::nodeProp_RegVisFOVOrientation2) &&
            this->m_spSelectedRegNode->GetProperty(orientationProp3, mitk::nodeProp_RegVisFOVOrientation3))
        {
            this->m_Controls->m_sbFOVOriginX->setValue(originProp->GetValue()[0]);
            this->m_Controls->m_sbFOVOriginY->setValue(originProp->GetValue()[1]);
            this->m_Controls->m_sbFOVOriginZ->setValue(originProp->GetValue()[2]);
            m_FOVRefOrientation.GetVnlMatrix().set_row(0, orientationProp1->GetValue().GetVnlVector());
            m_FOVRefOrientation.GetVnlMatrix().set_row(1, orientationProp2->GetValue().GetVnlVector());
            m_FOVRefOrientation.GetVnlMatrix().set_row(2, orientationProp3->GetValue().GetVnlVector());
        }
        else
        {
            m_FOVRefOrientation.SetIdentity();

            this->Error(QStringLiteral("Cannot configure plugin controlls correctly. One of the node propertiesy ") +
                QString(mitk::nodeProp_RegVisFOVOrientation1) + QString(mitk::nodeProp_RegVisFOVOrientation2) +
                QString(mitk::nodeProp_RegVisFOVOrientation3) + QStringLiteral(" is not correctly defined."));
        }

        this->UpdateOrientationMatrixWidget();
    }
}

void QmitkMatchPointRegistrationVisualizer::CheckAndSetDefaultFOVRef()
{
    //check if node has a default reference node.
    mitk::DataNode::Pointer defaultRef = this->GetRefNodeOfReg(
        this->m_Controls->m_comboDirection->currentIndex() ==
        1); //direction value 1 = show inverse mapping -> we need the target image used for the registration.

    //if there is a default node and no m_spSelectedFOVRefNode is set -> set default node and transfer values
    if (defaultRef.IsNotNull() && this->m_spSelectedFOVRefNode.IsNull())
    {
      //there is a default ref and no ref lock -> select default ref and transfer its values
      this->m_spSelectedFOVRefNode = defaultRef;
      QmitkSingleNodeSelectionWidget::NodeList selection({ defaultRef });
      this->m_Controls->fovReferenceNodeSelector->SetCurrentSelection(selection);
      this->m_Controls->m_checkUseRefSize->setChecked(true);
      this->m_Controls->m_checkUseRefOrigin->setChecked(true);
      this->m_Controls->m_checkUseRefSpacing->setChecked(true);
      this->m_Controls->m_checkUseRefOrientation->setChecked(true);
    }

    if (this->m_spSelectedFOVRefNode.IsNotNull())
    {
      //auto transfere values
      this->TransferFOVRefGeometry();
    }
}

void QmitkMatchPointRegistrationVisualizer::OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> /*nodes*/)
{
  this->CheckInputs();
  this->LoadStateFromNode();
  this->CheckAndSetDefaultFOVRef();
  this->ConfigureVisualizationControls();
}

void QmitkMatchPointRegistrationVisualizer::ActualizeRegInfo(mitk::MAPRegistrationWrapper*
    currentReg)
{
    std::stringstream descriptionString;

    m_Controls->m_teRegInfo->clear();

    if (currentReg)
    {
        descriptionString << "Moving dimension: " << currentReg->GetMovingDimensions() << "<br/>";
        descriptionString << "Target dimension: " << currentReg->GetTargetDimensions() << "<br/>";
        descriptionString << "Limited moving representation: " <<
            currentReg->HasLimitedMovingRepresentation() << "<br/>";
        descriptionString << "Limited target representation: " <<
            currentReg->HasLimitedTargetRepresentation() << "<br/>";

        mitk::MAPRegistrationWrapper::TagMapType tagMap = currentReg->GetTags();

        descriptionString << "<br/><b>Tags:</b><br/>";

        for (mitk::MAPRegistrationWrapper::TagMapType::const_iterator pos = tagMap.begin();
            pos != tagMap.end(); ++pos)
        {
            descriptionString << pos->first << " : " << pos->second << "<br/>";
        }
    }
    else
    {
        descriptionString << "<font color='red'>no registration selected!</font>";
    }

    m_Controls->m_teRegInfo->insertHtml(QString::fromStdString(descriptionString.str()));

}

void QmitkMatchPointRegistrationVisualizer::OnDirectionChanged(int)
{
    this->CheckAndSetDefaultFOVRef();
    this->ConfigureVisualizationControls();
}

void QmitkMatchPointRegistrationVisualizer::OnUpdateBtnPushed()
{
    this->StoreStateInNode();

    mitk::Geometry3D::Pointer gridDesc;
    unsigned int gridFrequ = 5;

    mitk::GetGridGeometryFromNode(this->m_spSelectedRegNode, gridDesc, gridFrequ);

    this->GetCurrentRegistration()->SetGeometry(gridDesc);
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void QmitkMatchPointRegistrationVisualizer::OnStyleButtonPushed()
{

}

void QmitkMatchPointRegistrationVisualizer::OnColorInterpolationChecked(bool checked)
{
    if (checked)
    {
        this->m_Controls->labelVecMagSmall->setText(QStringLiteral("="));
        this->m_Controls->labelVecMagMedium->setText(QStringLiteral("="));
        this->m_Controls->labelVecMagLarge->setText(QStringLiteral("="));
    }
    else
    {
        this->m_Controls->labelVecMagSmall->setText(QStringLiteral(">"));
        this->m_Controls->labelVecMagMedium->setText(QStringLiteral(">"));
        this->m_Controls->labelVecMagLarge->setText(QStringLiteral(">"));
    }
}

mitk::ScalarType QmitkMatchPointRegistrationVisualizer::GetSaveSpacing(mitk::ScalarType gridRes,
    mitk::ScalarType spacing, unsigned int maxGridRes) const
{
    mitk::ScalarType newSpacing = spacing;
    mitk::ScalarType scaling = gridRes / maxGridRes;

    if (scaling > 1.0)
    {
        newSpacing = spacing * scaling;
    }

    return newSpacing;
}

void QmitkMatchPointRegistrationVisualizer::TransferFOVRefGeometry()
{
    if (this->m_spSelectedFOVRefNode.IsNotNull())
    {
        assert(this->m_spSelectedFOVRefNode->GetData());
        assert(this->m_spSelectedFOVRefNode->GetData()->GetGeometry());

        mitk::BaseGeometry* gridRef = this->m_spSelectedFOVRefNode->GetData()->GetGeometry();

        mitk::Vector3D spacing = gridRef->GetSpacing();
        mitk::Point3D origin = gridRef->GetOrigin();
        mitk::Geometry3D::BoundsArrayType bounds = gridRef->GetBounds();
        mitk::AffineTransform3D::ConstPointer fovTransform = gridRef->GetIndexToWorldTransform();


        if (this->m_Controls->m_checkUseRefSize->isChecked())
        {
            this->m_Controls->m_sbFOVSizeX->setValue((bounds[1] - bounds[0])*spacing[0]);
            this->m_Controls->m_sbFOVSizeY->setValue((bounds[3] - bounds[2])*spacing[1]);
            this->m_Controls->m_sbFOVSizeZ->setValue((bounds[5] - bounds[4])*spacing[2]);
        }

        if (this->m_Controls->m_checkUseRefSpacing->isChecked())
        {

            this->m_Controls->m_sbGridSpX->setValue(GetSaveSpacing((bounds[1] - bounds[0]), spacing[0], 20));
            this->m_Controls->m_sbGridSpY->setValue(GetSaveSpacing((bounds[3] - bounds[2]), spacing[1], 20));
            this->m_Controls->m_sbGridSpZ->setValue(GetSaveSpacing((bounds[5] - bounds[4]), spacing[2], 20));

        }

        if (this->m_Controls->m_checkUseRefOrigin->isChecked())
        {
            this->m_Controls->m_sbFOVOriginX->setValue(origin[0]);
            this->m_Controls->m_sbFOVOriginY->setValue(origin[1]);
            this->m_Controls->m_sbFOVOriginZ->setValue(origin[2]);
        }

        if (this->m_Controls->m_checkUseRefOrientation->isChecked())
        {
            this->m_FOVRefOrientation = fovTransform->GetMatrix();
            this->UpdateOrientationMatrixWidget();
        }

    }
}

void QmitkMatchPointRegistrationVisualizer::UpdateOrientationMatrixWidget()
{
    for (unsigned int r = 0; r < 3; ++r)
    {
        for (unsigned int c = 0; c < 3; ++c)
        {
            this->m_Controls->m_tableOrientation->item(r,
                c)->setText(QString::number(this->m_FOVRefOrientation.GetVnlMatrix().get(r, c)));
        }
    }
}

void QmitkMatchPointRegistrationVisualizer::InitRegNode()
{
    if (this->m_spSelectedRegNode.IsNotNull())
    {
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGrid, mitk::BoolProperty::New(true));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGlyph, mitk::BoolProperty::New(false));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisPoints, mitk::BoolProperty::New(false));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisDirection,
            mitk::RegVisDirectionProperty::New());
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorStyle,
            mitk::RegVisColorStyleProperty::New(1));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorUni, mitk::ColorProperty::New(0,
            0.5, 0));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridFrequence,
            mitk::IntProperty::New(3));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridShowStart,
            mitk::BoolProperty::New(false));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisGridStartColor,
            mitk::ColorProperty::New(0.5, 0, 0));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVSize,
            mitk::Vector3DProperty::New(mitk::Vector3D(100.0)));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVSpacing,
            mitk::Vector3DProperty::New(mitk::Vector3D(5.0)));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor1Value, mitk::ColorProperty::New(0,
            0, 0.5));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Value, mitk::ColorProperty::New(0,
            0.7, 0));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor2Magnitude,
            mitk::DoubleProperty::New(1));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Value, mitk::ColorProperty::New(1,
            1, 0));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor3Magnitude,
            mitk::DoubleProperty::New(5));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Value, mitk::ColorProperty::New(1,
            0, 0));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColor4Magnitude,
            mitk::DoubleProperty::New(15));
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisColorInterpolate,
            mitk::BoolProperty::New(true));

        mitk::Point3D origin;
        origin.Fill(0.0);
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrigin,
            mitk::Point3dProperty::New(mitk::Point3D(origin)));

        mitk::Vector3D vec(0.0);
        vec.SetElement(0, 1.0);
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrientation1,
            mitk::Vector3DProperty::New(vec));
        vec.Fill(0.0);
        vec.SetElement(1, 1.0);
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrientation2,
            mitk::Vector3DProperty::New(vec));
        vec.Fill(0.0);
        vec.SetElement(2, 1.0);
        this->m_spSelectedRegNode->AddProperty(mitk::nodeProp_RegVisFOVOrientation3,
            mitk::Vector3DProperty::New(vec));

    }
}
