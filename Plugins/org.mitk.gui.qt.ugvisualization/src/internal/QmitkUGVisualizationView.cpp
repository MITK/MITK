/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkUGVisualizationView.h"
#include "QmitkStdMultiWidget.h"

#include <mitkUnstructuredGrid.h>
#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>
#include <mitkVtkScalarModeProperty.h>
#include <mitkPropertyObserver.h>

#include <QmitkUGCombinedRepresentationPropertyWidget.h>
#include <QmitkBoolPropertyWidget.h>

#include <QWidgetAction>


class UGVisVolumeObserver : public mitk::PropertyView
{
public:

  UGVisVolumeObserver(mitk::BoolProperty* property, QmitkUGVisualizationView* view)
    : PropertyView(property), m_View(view), m_BoolProperty(property)
  {
  }

protected:

  virtual void PropertyChanged()
  {
    m_View->m_VolumeMode = m_BoolProperty->GetValue();
    m_View->UpdateEnablement();
  }

  virtual void PropertyRemoved()
  {
    m_View->m_VolumeMode = false;
    m_Property = 0;
    m_BoolProperty = 0;
  }

  QmitkUGVisualizationView* m_View;
  mitk::BoolProperty* m_BoolProperty;

};


const std::string QmitkUGVisualizationView::VIEW_ID = "org.mitk.views.ugvisualization";


QmitkUGVisualizationView::QmitkUGVisualizationView()
: QmitkFunctionality(), m_MultiWidget(0), m_Outline2DAction(0), m_Outline2DWidget(0),
  m_LODAction(0), m_ScalarVisibilityAction(0), m_ScalarVisibilityWidget(0),
  m_FirstVolumeRepId(-1), m_ShowTFGeneratorWidget(true), m_ShowScalarOpacityWidget(false),
  m_ShowColorWidget(true), m_ShowGradientOpacityWidget(false), m_ShowTFGeneratorAction(0),
  m_ShowScalarOpacityAction(0), m_ShowColorAction(0), m_ShowGradientOpacityAction(0),
  m_VolumeModeObserver(0)
{
}

QmitkUGVisualizationView::QmitkUGVisualizationView(const QmitkUGVisualizationView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
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

  m_Controls.m_TransferFunctionGeneratorWidget->SetThresholdTabEnabled(m_VolumeMode);
  m_Controls.m_TransferFunctionGeneratorWidget->SetBellTabEnabled(m_VolumeMode);
  m_Controls.m_TransferFunctionWidget->SetScalarOpacityFunctionEnabled(m_VolumeMode);
  m_Controls.m_TransferFunctionWidget->SetGradientOpacityFunctionEnabled(m_VolumeMode);
}

void QmitkUGVisualizationView::UpdateGUI()
{
  bool enable = false;

  mitk::DataNode* node = 0;
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
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

    mitk::BoolProperty* outlineProp = 0;
    node->GetProperty(outlineProp, "outline polygons");
    m_Outline2DWidget->SetProperty(outlineProp);

    mitk::BoolProperty* scalarVisProp = 0;
    node->GetProperty(scalarVisProp, "scalar visibility");
    m_ScalarVisibilityWidget->SetProperty(scalarVisProp);

    mitk::VtkScalarModeProperty* scalarProp = 0;
    if (node->GetProperty(scalarProp, "scalar mode"))
    {
      m_Controls.m_ScalarModeComboBox->SetProperty(scalarProp);
    }

    mitk::GridRepresentationProperty* gridRepProp = 0;
    mitk::GridVolumeMapperProperty* gridVolumeProp = 0;
    mitk::BoolProperty* volumeProp = 0;
    node->GetProperty(gridRepProp, "grid representation");
    node->GetProperty(gridVolumeProp, "volumerendering.mapper");
    node->GetProperty(volumeProp, "volumerendering");
    m_Controls.m_RepresentationComboBox->SetProperty(gridRepProp, gridVolumeProp, volumeProp);

    if (m_VolumeModeObserver)
    {
      delete m_VolumeModeObserver;
      m_VolumeModeObserver = 0;
    }

    if (volumeProp)
    {
      m_VolumeModeObserver = new UGVisVolumeObserver(volumeProp, this);
    }
  }

  UpdateEnablement();

}


void QmitkUGVisualizationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkUGVisualizationView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = 0;
}


void QmitkUGVisualizationView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{ 
  UpdateGUI();
}




