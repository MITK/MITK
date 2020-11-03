/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkUGVisualizationView.h"

#include <mitkUnstructuredGrid.h>
#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>
#include <mitkVtkScalarModeProperty.h>
#include <mitkPropertyObserver.h>

#include <QmitkUGCombinedRepresentationPropertyWidget.h>
#include <QmitkBoolPropertyWidget.h>

#include <QWidgetAction>

#include <QMenu>


class UGVisVolumeObserver : public mitk::PropertyView
{
public:

  UGVisVolumeObserver(mitk::BoolProperty* property, QmitkUGVisualizationView* view)
    : PropertyView(property), m_View(view), m_BoolProperty(property)
  {
  }

protected:

  void PropertyChanged() override
  {
    m_View->m_VolumeMode = m_BoolProperty->GetValue();
    m_View->UpdateEnablement();
  }

  void PropertyRemoved() override
  {
    m_View->m_VolumeMode = false;
    m_Property = nullptr;
    m_BoolProperty = nullptr;
  }

  QmitkUGVisualizationView* m_View;
  mitk::BoolProperty* m_BoolProperty;

};


const std::string QmitkUGVisualizationView::VIEW_ID = "org.mitk.views.ugvisualization";


QmitkUGVisualizationView::QmitkUGVisualizationView()
: QmitkAbstractView(), m_Outline2DAction(nullptr), m_Outline2DWidget(nullptr),
  m_LODAction(nullptr), m_ScalarVisibilityAction(nullptr), m_ScalarVisibilityWidget(nullptr),
  m_FirstVolumeRepId(-1), m_ShowTFGeneratorWidget(true), m_ShowScalarOpacityWidget(false),
  m_ShowColorWidget(true), m_ShowGradientOpacityWidget(false), m_ShowTFGeneratorAction(nullptr),
  m_ShowScalarOpacityAction(nullptr), m_ShowColorAction(nullptr), m_ShowGradientOpacityAction(nullptr),
  m_VolumeModeObserver(nullptr)
{
}

QmitkUGVisualizationView::~QmitkUGVisualizationView()
{
  delete m_VolumeModeObserver;
}


void QmitkUGVisualizationView::CreateQtPartControl( QWidget *parent )
{
  m_Controls.setupUi( parent );

  m_Outline2DWidget = new QmitkBoolPropertyWidget("Outline 2D polygons", parent);
  m_Outline2DAction = new QWidgetAction(this);
  m_Outline2DAction->setDefaultWidget(m_Outline2DWidget);

  m_LODAction = new QAction("Enable LOD (Level Of Detail)", this);
  m_LODAction->setCheckable(true);

  m_ScalarVisibilityWidget = new QmitkBoolPropertyWidget("Visualize scalars", parent);
  m_ScalarVisibilityAction = new QWidgetAction(this);
  m_ScalarVisibilityAction->setDefaultWidget(m_ScalarVisibilityWidget);

  m_ShowColorAction = new QAction("Show color transfer function", this);
  m_ShowColorAction->setCheckable(true);
  m_ShowColorAction->setChecked(m_ShowColorWidget);
  m_ShowGradientOpacityAction = new QAction("Show gradient opacity function", this);
  m_ShowGradientOpacityAction->setCheckable(true);
  m_ShowGradientOpacityAction->setChecked(m_ShowGradientOpacityWidget);
  m_ShowScalarOpacityAction = new QAction("Show scalar opacity function", this);
  m_ShowScalarOpacityAction->setCheckable(true);
  m_ShowScalarOpacityAction->setChecked(m_ShowScalarOpacityWidget);
  m_ShowTFGeneratorAction = new QAction("Show transfer function generator", this);
  m_ShowTFGeneratorAction->setCheckable(true);
  m_ShowTFGeneratorAction->setChecked(m_ShowTFGeneratorWidget);

  QMenu* menu = new QMenu(parent);
  menu->addAction(m_ScalarVisibilityAction);
  menu->addAction(m_Outline2DAction);
  //menu->addAction(m_LODAction);
  menu->addSeparator();
  menu->addAction(m_ShowTFGeneratorAction);
  menu->addAction(m_ShowScalarOpacityAction);
  menu->addAction(m_ShowColorAction);
  menu->addAction(m_ShowGradientOpacityAction);

  m_Controls.m_OptionsButton->setMenu(menu);

  m_Controls.m_TransferFunctionWidget->SetScalarLabel("Scalar value");

//  const mitk::EnumerationProperty::EnumStringsContainerType& scalarStrings = scalarProp->GetEnumStrings();

//  for (mitk::EnumerationProperty::EnumStringsContainerType::const_iterator it = scalarStrings.begin();
//    it != scalarStrings.end(); ++it)
//  {
//    MITK_INFO << "ADding: " << it->first;
//    m_Controls.m_ScalarModeComboBox->addItem(QString::fromStdString(it->first), it->second);
//  }

  this->UpdateGUI();

  CreateConnections();
}

void QmitkUGVisualizationView::CreateConnections()
{
  connect(m_Controls.m_ScalarModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateRenderWindow()));
  connect(m_Controls.m_RepresentationComboBox, SIGNAL(activated(int)), this, SLOT(UpdateRenderWindow()));
  connect(m_Outline2DWidget, SIGNAL(toggled(bool)), this, SLOT(UpdateRenderWindow()));
  connect(m_ScalarVisibilityWidget, SIGNAL(toggled(bool)), this, SLOT(UpdateRenderWindow()));
  connect(m_Controls.m_TransferFunctionGeneratorWidget, SIGNAL(SignalUpdateCanvas()), m_Controls.m_TransferFunctionWidget, SLOT(OnUpdateCanvas()));

  connect(m_ShowColorAction, SIGNAL(triggered(bool)), this, SLOT(ShowColorWidget(bool)));
  connect(m_ShowGradientOpacityAction, SIGNAL(triggered(bool)), this, SLOT(ShowGradientOpacityWidget(bool)));
  connect(m_ShowScalarOpacityAction, SIGNAL(triggered(bool)), this, SLOT(ShowScalarOpacityWidget(bool)));
  connect(m_ShowTFGeneratorAction, SIGNAL(triggered(bool)), this, SLOT(ShowTFGeneratorWidget(bool)));
}

void QmitkUGVisualizationView::SetFocus()
{
  m_Controls.m_OptionsButton->setFocus();
}

void QmitkUGVisualizationView::UpdateRenderWindow()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkUGVisualizationView::ShowTFGeneratorWidget(bool show)
{
  m_ShowTFGeneratorWidget = show;
  UpdateEnablement();
}

void QmitkUGVisualizationView::ShowScalarOpacityWidget(bool show)
{
  m_ShowScalarOpacityWidget = show;
  UpdateEnablement();
}

void QmitkUGVisualizationView::ShowColorWidget(bool show)
{
  m_ShowColorWidget = show;
  UpdateEnablement();
}

void QmitkUGVisualizationView::ShowGradientOpacityWidget(bool show)
{
  m_ShowGradientOpacityWidget = show;
  UpdateEnablement();
}

void QmitkUGVisualizationView::UpdateEnablement()
{
  m_Controls.m_TransferFunctionGeneratorWidget->setVisible(m_ShowTFGeneratorWidget);
  m_Controls.m_TransferFunctionWidget->ShowScalarOpacityFunction(m_ShowScalarOpacityWidget);
  m_Controls.m_TransferFunctionWidget->ShowColorFunction(m_ShowColorWidget);
  m_Controls.m_TransferFunctionWidget->ShowGradientOpacityFunction(m_ShowGradientOpacityWidget);

  m_Controls.m_TransferFunctionGeneratorWidget->SetThresholdTabEnabled(m_ScalarVisibilityWidget->isChecked());
  m_Controls.m_TransferFunctionGeneratorWidget->SetBellTabEnabled(m_ScalarVisibilityWidget->isChecked());
  m_Controls.m_TransferFunctionWidget->SetScalarOpacityFunctionEnabled(m_ScalarVisibilityWidget->isChecked());
  m_Controls.m_TransferFunctionWidget->SetGradientOpacityFunctionEnabled(m_VolumeMode);
}

void QmitkUGVisualizationView::UpdateGUI()
{
  bool enable = false;

  mitk::DataNode* node = nullptr;
  auto nodes = this->GetDataManagerSelection();
  if (!nodes.empty())
  {
    node = nodes.front();

    if (node)
    {
      // here we have a valid mitk::DataNode

      // a node itself is not very useful, we need its data item
      mitk::BaseData* data = node->GetData();
      if (data)
      {
        // test if this data item is an unstructured grid
        enable = dynamic_cast<mitk::UnstructuredGrid*>( data );
      }
    }
  }

  m_Controls.m_SelectedLabel->setVisible(enable);
  m_Controls.m_ErrorLabel->setVisible(!enable);
  m_Controls.m_ContainerWidget->setEnabled(enable);
  m_Controls.m_OptionsButton->setEnabled(enable);

  if (enable)
  {
    m_VolumeMode = false;
    node->GetBoolProperty("volumerendering", m_VolumeMode);

    m_Controls.m_SelectedLabel->setText(QString("Selected UG: ") + node->GetName().c_str());

    m_Controls.m_TransferFunctionGeneratorWidget->SetDataNode(node);
    m_Controls.m_TransferFunctionWidget->SetDataNode(node);

    mitk::BoolProperty* outlineProp = nullptr;
    node->GetProperty(outlineProp, "outline polygons");
    m_Outline2DWidget->SetProperty(outlineProp);

    mitk::BoolProperty* scalarVisProp = nullptr;
    node->GetProperty(scalarVisProp, "scalar visibility");
    m_ScalarVisibilityWidget->SetProperty(scalarVisProp);

    mitk::VtkScalarModeProperty* scalarProp = nullptr;
    if (node->GetProperty(scalarProp, "scalar mode"))
    {
      m_Controls.m_ScalarModeComboBox->SetProperty(scalarProp);
    }

    mitk::GridRepresentationProperty* gridRepProp = nullptr;
    mitk::GridVolumeMapperProperty* gridVolumeProp = nullptr;
    mitk::BoolProperty* volumeProp = nullptr;
    node->GetProperty(gridRepProp, "grid representation");
    node->GetProperty(gridVolumeProp, "volumerendering.mapper");
    node->GetProperty(volumeProp, "volumerendering");
    m_Controls.m_RepresentationComboBox->SetProperty(gridRepProp, gridVolumeProp, volumeProp);

    if (m_VolumeModeObserver)
    {
      delete m_VolumeModeObserver;
      m_VolumeModeObserver = nullptr;
    }

    if (volumeProp)
    {
      m_VolumeModeObserver = new UGVisVolumeObserver(volumeProp, this);
    }
  }

  UpdateEnablement();

}


void QmitkUGVisualizationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& /*nodes*/)
{
  UpdateGUI();
}




