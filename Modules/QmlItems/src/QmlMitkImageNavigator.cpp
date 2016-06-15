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

#include "QmlMitkImageNavigator.h"

#include <QmlMitkStdMultiItem.h>
#include <QmitkStepperAdapter.h>

QmlMitkImageNavigator* QmlMitkImageNavigator::instance = nullptr;

QmlMitkImageNavigator::QmlMitkImageNavigator()
: m_AxialStepper(0)
, m_SagittalStepper(0)
, m_FrontalStepper(0)
, m_TimeStepper(0)
{
    instance = this;
}

QmlMitkImageNavigator::~QmlMitkImageNavigator()
{
}

void QmlMitkImageNavigator::setNavigatorAxial(QmlMitkSliderNavigatorItem *item)
{
    this->m_NavigatorAxial = item;
}

void QmlMitkImageNavigator::setNavigatorSagittal(QmlMitkSliderNavigatorItem *item)
{
    this->m_NavigatorSagittal = item;
}

void QmlMitkImageNavigator::setNavigatorCoronal(QmlMitkSliderNavigatorItem *item)
{
    this->m_NavigatorCoronal = item;
}

void QmlMitkImageNavigator::setNavigatorTime(QmlMitkSliderNavigatorItem *item)
{
    this->m_NavigatorTime = item;
}

QmlMitkSliderNavigatorItem* QmlMitkImageNavigator::getNavigatorAxial()
{
    return this->m_NavigatorAxial;
}

QmlMitkSliderNavigatorItem* QmlMitkImageNavigator::getNavigatorSagittal()
{
    return this->m_NavigatorSagittal;
}

QmlMitkSliderNavigatorItem* QmlMitkImageNavigator::getNavigatorCoronal()
{
    return this->m_NavigatorCoronal;
}

QmlMitkSliderNavigatorItem* QmlMitkImageNavigator::getNavigatorTime()
{
    return this->m_NavigatorTime;
}

void QmlMitkImageNavigator::setWorldCoordinateX(double coordinate)
{
    this->m_WorldCoordinateX = coordinate;
    this->OnMillimetreCoordinateValueChanged();
}

void QmlMitkImageNavigator::setWorldCoordinateY(double coordinate)
{
    this->m_WorldCoordinateY = coordinate;
    this->OnMillimetreCoordinateValueChanged();
}

void QmlMitkImageNavigator::setWorldCoordinateZ(double coordinate)
{
    this->m_WorldCoordinateZ = coordinate;
    this->OnMillimetreCoordinateValueChanged();
}

void QmlMitkImageNavigator::setWorldCoordinateXMin(double coordinate)
{
    this->m_WorldCoordinateXMin = coordinate;
}

void QmlMitkImageNavigator::setWorldCoordinateYMin(double coordinate)
{
    this->m_WorldCoordinateYMin = coordinate;
}

void QmlMitkImageNavigator::setWorldCoordinateZMin(double coordinate)
{
    this->m_WorldCoordinateZMin = coordinate;
}

void QmlMitkImageNavigator::setWorldCoordinateXMax(double coordinate)
{
    this->m_WorldCoordinateXMax = coordinate;
}

void QmlMitkImageNavigator::setWorldCoordinateYMax(double coordinate)
{
    this->m_WorldCoordinateYMax = coordinate;
}

void QmlMitkImageNavigator::setWorldCoordinateZMax(double coordinate)
{
    this->m_WorldCoordinateZMax = coordinate;
}

double QmlMitkImageNavigator::getWorldCoordinateX()
{
    return this->m_WorldCoordinateX;
}

double QmlMitkImageNavigator::getWorldCoordinateY()
{
    return this->m_WorldCoordinateY;
}

double QmlMitkImageNavigator::getWorldCoordinateZ()
{
    return this->m_WorldCoordinateZ;
}

double QmlMitkImageNavigator::getWorldCoordinateXMin()
{
    return this->m_WorldCoordinateXMin;
}

double QmlMitkImageNavigator::getWorldCoordinateYMin()
{
    return this->m_WorldCoordinateYMin;
}

double QmlMitkImageNavigator::getWorldCoordinateZMin()
{
    return this->m_WorldCoordinateZMin;
}

double QmlMitkImageNavigator::getWorldCoordinateXMax()
{
    return this->m_WorldCoordinateXMax;
}

double QmlMitkImageNavigator::getWorldCoordinateYMax()
{
    return this->m_WorldCoordinateYMax;
}

double QmlMitkImageNavigator::getWorldCoordinateZMax()
{
    return this->m_WorldCoordinateZMax;
}

void QmlMitkImageNavigator::initialize()
{
    if(!QmlMitkStdMultiItem::instance)
        return;

    QmlMitkRenderWindowItem* renderWindow = QmlMitkStdMultiItem::instance->getViewerAxial();

    if (renderWindow)
    {
        if (m_AxialStepper) m_AxialStepper->deleteLater();
        m_AxialStepper = new QmitkStepperAdapter(this->m_NavigatorAxial,
                                                 renderWindow->GetSliceNavigationController()->GetSlice(),
                                                 "sliceNavigatorAxialFromSimpleExample");

        connect(m_AxialStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
    }
    else
    {
        this->m_NavigatorAxial->setEnabled(false);
    }

    renderWindow = QmlMitkStdMultiItem::instance->getViewerSagittal();
    if (renderWindow)
    {
        if (m_SagittalStepper) m_SagittalStepper->deleteLater();
        m_SagittalStepper = new QmitkStepperAdapter(this->m_NavigatorSagittal,
                                                    renderWindow->GetSliceNavigationController()->GetSlice(),
                                                    "sliceNavigatorSagittalFromSimpleExample");
        connect(m_SagittalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
    }
    else
    {
        this->m_NavigatorSagittal->setEnabled(false);
    }

    renderWindow = QmlMitkStdMultiItem::instance->getViewerCoronal();
    if (renderWindow)
    {
        if (m_FrontalStepper) m_FrontalStepper->deleteLater();
        m_FrontalStepper = new QmitkStepperAdapter(this->m_NavigatorCoronal,
                                                   renderWindow->GetSliceNavigationController()->GetSlice(),
                                                   "sliceNavigatorFrontalFromSimpleExample");
        connect(m_FrontalStepper, SIGNAL(Refetch()), this, SLOT(OnRefetch()));
    }
    else
    {
        this->m_NavigatorCoronal->setEnabled(false);
    }

    mitk::SliceNavigationController* timeController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
    if (timeController)
    {
        if (m_TimeStepper) m_TimeStepper->deleteLater();
        m_TimeStepper = new QmitkStepperAdapter(this->m_NavigatorTime,
                                                timeController->GetTime(),
                                                "sliceNavigatorTimeFromSimpleExample");
    }
    else
    {
        this->m_NavigatorTime->setEnabled(false);
    }
}

int QmlMitkImageNavigator::GetClosestAxisIndex(mitk::Vector3D normal)
{
    // cos(theta) = normal . axis
    // cos(theta) = (a, b, c) . (d, e, f)
    // cos(theta) = (a, b, c) . (1, 0, 0) = a
    // cos(theta) = (a, b, c) . (0, 1, 0) = b
    // cos(theta) = (a, b, c) . (0, 0, 1) = c
    double absCosThetaWithAxis[3];

    for (int i = 0; i < 3; i++)
    {
        absCosThetaWithAxis[i] = fabs(normal[i]);
    }
    int largestIndex = 0;
    double largestValue = absCosThetaWithAxis[0];
    for (int i = 1; i < 3; i++)
    {
        if (absCosThetaWithAxis[i] > largestValue)
        {
            largestValue = absCosThetaWithAxis[i];
            largestIndex = i;
        }
    }
    return largestIndex;
}

void QmlMitkImageNavigator::SetStepSizes()
{
    this->SetStepSize(0);
    this->SetStepSize(1);
    this->SetStepSize(2);
}

void QmlMitkImageNavigator::SetStepSize(int axis)
{
    mitk::BaseGeometry::ConstPointer geometry = QmlMitkStdMultiItem::instance->getViewerAxial()->GetSliceNavigationController()->GetInputWorldGeometry3D();

    if (geometry.IsNotNull())
    {
        mitk::Point3D crossPositionInIndexCoordinates;
        mitk::Point3D crossPositionInIndexCoordinatesPlus1;
        mitk::Point3D crossPositionInMillimetresPlus1;
        mitk::Vector3D transformedAxisDirection;

        mitk::Point3D crossPositionInMillimetres = QmlMitkStdMultiItem::instance->getCrossPosition();
        geometry->WorldToIndex(crossPositionInMillimetres, crossPositionInIndexCoordinates);

        crossPositionInIndexCoordinatesPlus1 = crossPositionInIndexCoordinates;
        crossPositionInIndexCoordinatesPlus1[axis] += 1;

        geometry->IndexToWorld(crossPositionInIndexCoordinatesPlus1, crossPositionInMillimetresPlus1);

        transformedAxisDirection = crossPositionInMillimetresPlus1 - crossPositionInMillimetres;

        int closestAxisInMillimetreSpace = this->GetClosestAxisIndex(transformedAxisDirection);
        double stepSize = transformedAxisDirection.GetNorm();
        this->SetStepSize(closestAxisInMillimetreSpace, stepSize);
    }
}

void QmlMitkImageNavigator::SetStepSize(int axis, double stepSize)
{
    if (axis == 0)
    {
        //m_Controls.m_XWorldCoordinateSpinBox->setSingleStep(stepSize);
    }
    else if (axis == 1)
    {
        //m_Controls.m_YWorldCoordinateSpinBox->setSingleStep(stepSize);
    }
    else if (axis == 2)
    {
        //m_Controls.m_ZWorldCoordinateSpinBox->setSingleStep(stepSize);
    }
}


void QmlMitkImageNavigator::OnMillimetreCoordinateValueChanged()
{
    mitk::TimeGeometry::ConstPointer geometry = QmlMitkStdMultiItem::instance->getViewerAxial()->GetSliceNavigationController()->GetInputWorldTimeGeometry();

    if (geometry.IsNotNull())
    {
        mitk::Point3D positionInWorldCoordinates;
        positionInWorldCoordinates[0] = this->m_WorldCoordinateX;
        positionInWorldCoordinates[1] = this->m_WorldCoordinateY;
        positionInWorldCoordinates[2] = this->m_WorldCoordinateZ;

        QmlMitkStdMultiItem::instance->moveCrossToPosition(positionInWorldCoordinates);
    }
}


void QmlMitkImageNavigator::OnRefetch()
{
    mitk::BaseGeometry::ConstPointer geometry = QmlMitkStdMultiItem::instance->getViewerAxial()->GetSliceNavigationController()->GetInputWorldGeometry3D();
    mitk::TimeGeometry::ConstPointer timeGeometry = QmlMitkStdMultiItem::instance->getViewerAxial()->GetSliceNavigationController()->GetInputWorldTimeGeometry();

    if (geometry.IsNull() && timeGeometry.IsNotNull())
    {
        mitk::TimeStepType timeStep = QmlMitkStdMultiItem::instance->getViewerAxial()->GetSliceNavigationController()->GetTime()->GetPos();
        geometry = timeGeometry->GetGeometryForTimeStep(timeStep);
    }

    if (geometry.IsNotNull())
    {
        mitk::BoundingBox::BoundsArrayType bounds = geometry->GetBounds();

        mitk::Point3D cornerPoint1InIndexCoordinates;
        cornerPoint1InIndexCoordinates[0] = bounds[0];
        cornerPoint1InIndexCoordinates[1] = bounds[2];
        cornerPoint1InIndexCoordinates[2] = bounds[4];

        mitk::Point3D cornerPoint2InIndexCoordinates;
        cornerPoint2InIndexCoordinates[0] = bounds[1];
        cornerPoint2InIndexCoordinates[1] = bounds[3];
        cornerPoint2InIndexCoordinates[2] = bounds[5];

        if (!geometry->GetImageGeometry())
        {
            cornerPoint1InIndexCoordinates[0] += 0.5;
            cornerPoint1InIndexCoordinates[1] += 0.5;
            cornerPoint1InIndexCoordinates[2] += 0.5;
            cornerPoint2InIndexCoordinates[0] -= 0.5;
            cornerPoint2InIndexCoordinates[1] -= 0.5;
            cornerPoint2InIndexCoordinates[2] -= 0.5;
        }

        mitk::Point3D crossPositionInWorldCoordinates = QmlMitkStdMultiItem::instance->getCrossPosition();

        mitk::Point3D cornerPoint1InWorldCoordinates;
        mitk::Point3D cornerPoint2InWorldCoordinates;

        geometry->IndexToWorld(cornerPoint1InIndexCoordinates, cornerPoint1InWorldCoordinates);
        geometry->IndexToWorld(cornerPoint2InIndexCoordinates, cornerPoint2InWorldCoordinates);

        this->m_WorldCoordinateXMin = std::min(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]);
        this->m_WorldCoordinateYMin = std::min(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]);
        this->m_WorldCoordinateZMin = std::min(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]);

        this->m_WorldCoordinateXMax = std::max(cornerPoint1InWorldCoordinates[0], cornerPoint2InWorldCoordinates[0]);
        this->m_WorldCoordinateYMax = std::max(cornerPoint1InWorldCoordinates[1], cornerPoint2InWorldCoordinates[1]);
        this->m_WorldCoordinateZMax = std::max(cornerPoint1InWorldCoordinates[2], cornerPoint2InWorldCoordinates[2]);

        this->m_WorldCoordinateX = crossPositionInWorldCoordinates[0];
        this->m_WorldCoordinateY = crossPositionInWorldCoordinates[1];
        this->m_WorldCoordinateZ = crossPositionInWorldCoordinates[2];

        emit this->sync();
    }
}

void QmlMitkImageNavigator::create(QQmlEngine &engine)
{
    QmlMitkSliderNavigatorItem::create();

    qmlRegisterType<QmlMitkImageNavigator>("Mitk.Views", 1, 0, "ImageNavigator");
    QQmlComponent component(&engine, QUrl("qrc:/MitkImageNavigator.qml"));
}
