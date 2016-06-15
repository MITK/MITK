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

#ifndef __QmlMitkProperties_h
#define __QmlMitkProperties_h

#include "MitkQmlItemsExports.h"

#include <QObject>
#include <QQuickItem>

#include <mitkDataNode.h>
#include <mitkWeakPointer.h>

enum RenderMode
{
    RM_CPU_COMPOSITE_RAYCAST = 0,
    RM_CPU_MIP_RAYCAST       = 1,
    RM_GPU_COMPOSITE_SLICING = 2,
    RM_GPU_COMPOSITE_RAYCAST = 3,
    RM_GPU_MIP_RAYCAST       = 4
};

class MITKQMLITEMS_EXPORT QmlMitkProperties : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(RenderMode)
    Q_PROPERTY(bool rendering READ getRendering WRITE setRendering NOTIFY renderingChanged);
    Q_PROPERTY(bool lod READ getLod WRITE setLod NOTIFY lodChanged);
    Q_PROPERTY(int mode READ getMode WRITE setMode NOTIFY modeChanged);
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged);

private:
    bool m_rendering;
    bool m_lod;
    int m_mode;
    bool m_enabled;
    mitk::WeakPointer<mitk::DataNode> m_node;

public:
    static QmlMitkProperties* instance;

    void setRendering(bool state);
    void setLod(bool state);
    void setMode(int mode);
    void setEnabled(bool enabled);

    bool isEnabled();
    bool getRendering();
    bool getLod();
    int getMode();

    void notify(mitk::DataNode::Pointer node);

    QmlMitkProperties();
    ~QmlMitkProperties();

    static void create(QQmlEngine &engine);

signals:
    void renderingChanged();
    void lodChanged();
    void modeChanged();
    void enabledChanged();
    void sync();
};

#endif
