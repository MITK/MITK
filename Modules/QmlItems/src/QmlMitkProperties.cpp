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

#include "QmlMitkProperties.h"

#include <mitkImage.h>
#include <mitkDataNode.h>
#include <QmlMitkRenderingManager.h>
#include <QmlMitkTransferFunctionItem.h>

#include <QQmlEngine>
#include <QQmlContext>

QmlMitkProperties* QmlMitkProperties::instance = nullptr;

QmlMitkProperties::QmlMitkProperties()
{
    instance = this;
}

QmlMitkProperties::~QmlMitkProperties()
{
}

void QmlMitkProperties::setRendering(bool state)
{
    this->m_rendering = state;
    if(this->m_node)
    {
        this->m_node->SetBoolProperty("volumerendering", state);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

        QmlMitkTransferFunctionItem::instance->OnUpdateCanvas();
    }

    emit this->renderingChanged();
}

void QmlMitkProperties::setLod(bool state)
{
    this->m_lod = state;
    if(this->m_node)
    {
        this->m_node->SetBoolProperty("volumerendering.uselod", state);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    emit this->lodChanged();
}

void QmlMitkProperties::setMode(int mode)
{
    if (!this->m_node)
        return;

    this->m_mode = mode;

    bool usegpu=mode==RM_GPU_COMPOSITE_SLICING;
    // Only with VTK 5.6 or above
#if ((VTK_MAJOR_VERSION > 5) || ((VTK_MAJOR_VERSION==5) && (VTK_MINOR_VERSION>=6) ))
    bool useray=(mode==RM_GPU_COMPOSITE_RAYCAST)||(mode==RM_GPU_MIP_RAYCAST);
#endif
    bool usemip=(mode==RM_GPU_MIP_RAYCAST)||(mode==RM_CPU_MIP_RAYCAST);

    this->m_node->SetBoolProperty("volumerendering.usegpu",usegpu);
    // Only with VTK 5.6 or above
#if ((VTK_MAJOR_VERSION > 5) || ((VTK_MAJOR_VERSION==5) && (VTK_MINOR_VERSION>=6) ))
    this->m_node->SetBoolProperty("volumerendering.useray",useray);
#endif
    this->m_node->SetBoolProperty("volumerendering.usemip",usemip);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    emit this->modeChanged();
}

bool QmlMitkProperties::getRendering()
{
    return this->m_rendering;
}

bool QmlMitkProperties::getLod()
{
    return this->m_lod;
}

int QmlMitkProperties::getMode()
{
    return this->m_mode;
}

void QmlMitkProperties::setEnabled(bool enabled)
{
    this->m_enabled = enabled;
    emit this->enabledChanged();
}

bool QmlMitkProperties::isEnabled()
{
    return this->m_enabled;
}

void QmlMitkProperties::notify(mitk::DataNode::Pointer node)
{
    this->m_node = node;

    if(!this->m_node)
    {
        this->setEnabled(false);
        emit this->sync();
        return;
    }

    mitk::Image::Pointer image = static_cast<mitk::Image*>(this->m_node->GetData());

    if(image->GetDimension() == 2)
        this->setEnabled(false);
    else
        this->setEnabled(true);

    bool use_volumerendering;
    bool use_lod;
    bool use_ray;
    bool use_mip;
    bool use_gpu;

    this->m_node->GetBoolProperty("volumerendering", use_volumerendering);
    this->m_node->GetBoolProperty("volumerendering.uselod", use_lod);
    this->m_node->GetBoolProperty("volumerendering.useray", use_ray);
    this->m_node->GetBoolProperty("volumerendering.usemip", use_mip);
    this->m_node->GetBoolProperty("volumerendering.usegpu", use_gpu);

    this->setRendering(use_volumerendering);
    this->setLod(use_lod);

    int mode = 0;

    if(use_ray)
    {
        if(use_mip)
            mode=RM_GPU_MIP_RAYCAST;
        else
            mode=RM_GPU_COMPOSITE_RAYCAST;
    }
    else if(use_gpu)
        mode=RM_GPU_COMPOSITE_SLICING;
    else
    {
        if(use_mip)
            mode=RM_CPU_MIP_RAYCAST;
        else
            mode=RM_CPU_COMPOSITE_RAYCAST;
    }

    this->setMode(mode);

    emit this->sync();
}

void QmlMitkProperties::create(QQmlEngine &engine)
{
    qmlRegisterType<QmlMitkProperties>("Mitk.Views", 1, 0, "Properties");

    QQmlComponent component(&engine, QUrl("qrc:/MitkProperties.qml"));
}
