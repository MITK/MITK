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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
// Qmitk
#include "PointSetInteractionMultispectrum.h"
#include "QmitkPointListWidget.h"
#include "QmitkRenderWindow.h"

// Qt
#include <QMessageBox>
#include <QApplication>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <QColor>
#include <QString>

//mitk image
#include <mitkImage.h>

// MITK
#include "mitkITKImageImport.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkImageCast.h"
#include <mitkVector.h>
#include <mitkPoint.h>
#include <mitkImageStatisticsHolder.h>
// ITK
#include <itkConnectedThresholdImageFilter.h>

const std::string PointSetInteractionMultispectrum::VIEW_ID =
    "org.mitk.views.pointsetinteractionmultispectrum";

PointSetInteractionMultispectrum::PointSetInteractionMultispectrum() :
                    m_PointListWidget(NULL)
{

}

void PointSetInteractionMultispectrum::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void PointSetInteractionMultispectrum::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this,
      SLOT(DoImageProcessing()));

  //! [cpp-createqtpartcontrol]
  // create a QmitkPointListWidget and add it to the widget created from .ui file
  m_PointListWidget = new QmitkPointListWidget();
  m_Controls.verticalLayout->addWidget(m_PointListWidget, 1);

  // retrieve a possibly existing IRenderWindowPart
  if (mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart())
  {
    // let the point set widget know about the render window part (crosshair updates)
    RenderWindowPartActivated(renderWindowPart);
  }

  // create a new DataNode containing a PointSet with some interaction
  m_PointSet = mitk::PointSet::New();
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  pointSetNode->SetData(m_PointSet);
  pointSetNode->SetName("points for displaying reflectance.");
  pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));

  // add the pointset to the data storage (for rendering and access by other modules)
  GetDataStorage()->Add(pointSetNode);

  // tell the GUI widget about the point set
  m_PointListWidget->SetPointSetNode(pointSetNode);
  //! [cpp-createqtpartcontrol]

  m_Plot = new QwtPlot();
}


void PointSetInteractionMultispectrum::OnSelectionChanged(
    berry::IWorkbenchPart::Pointer /*source*/,
    const QList<mitk::DataNode::Pointer>& nodes) {
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes ){
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonPerformImageProcessing->setEnabled( false );
}


void PointSetInteractionMultispectrum::RenderWindowPartActivated(
    mitk::IRenderWindowPart* renderWindowPart)
{
  // let the point set widget know about the slice navigation controllers
  // in the active render window part (crosshair updates)
  foreach(QmitkRenderWindow* renderWindow, renderWindowPart->GetQmitkRenderWindows().values())
                {
    m_PointListWidget->AddSliceNavigationController(renderWindow->GetSliceNavigationController());
                }
}

void PointSetInteractionMultispectrum::RenderWindowPartDeactivated(
    mitk::IRenderWindowPart* renderWindowPart)
{
  foreach(QmitkRenderWindow* renderWindow, renderWindowPart->GetQmitkRenderWindows().values())
                {
    m_PointListWidget->RemoveSliceNavigationController(renderWindow->GetSliceNavigationController());
                }
}

void PointSetInteractionMultispectrum::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> DataManagerNodes =
      this->GetDataManagerSelection();
  if (DataManagerNodes.empty())
    return;

  mitk::DataNode* DataManagerNode = DataManagerNodes.front();

  if (!DataManagerNode)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template",
        "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode
  // a node itself is not very useful, we need its data item (the image).
  // notice that this is the 'BaseData' type.

  mitk::BaseData* data = DataManagerNode->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);
    if (image)
    {
      //! [cpp-doimageprocessing]
      // So we have an image. Let's see if the user has set some seed points already
      if (m_PointSet->GetSize() == 0)
      {
        // no points there. Not good for region growing
        QMessageBox::information( NULL,
            "Reflectance display functionality",
            "Please set some points inside the image first.\n"
            "(hold Shift key and click left mouse button inside the image.)");
        return;
      }

      // plot the reflectances //
      PlotReflectance(m_PointSet, DataManagerNodes);

      //! [cpp-doimageprocessing]
    }
  }
}


void PointSetInteractionMultispectrum::PlotReflectance(mitk::PointSet::Pointer m_PointSet, QList<mitk::DataNode::Pointer> dataManagerNodes)
{
  mitk::PointSet::PointsIterator PtIterator; // The point iterator in the PointSet, which gets accesss to each point.
  mitk::Point3D Point; // The extracted point. Notice that a point has three components (x, y, z).
  itk::Index<3> index; // The 3D index, which is converted from the world coordinate
  itk::Index<2> index2; // The truncated version of the 3D index, for which only the first two componets are extracted.
  itk::VectorImage<double, 2>::Pointer itkImage; // The itk vector image. This is used since the mitk::Image could not handle multi-channel images well.


  //    ////////////////  Qwt window configuration  /////////////////////////////////////////////
  delete m_Plot;
  m_Plot = new QwtPlot();                             // create a new plot               //
  m_Plot->setAxisAutoScale(QwtPlot::xBottom);         // automatical scale -x            //
  m_Plot->setAxisAutoScale(QwtPlot::yLeft);           // automatical scale -y            //
  m_Plot->setTitle("Multispectral Reflectance");     // set the plot title              //
  m_Plot->setCanvasBackground(Qt::white);             // set the background color        //
  m_Plot->insertLegend(new QwtLegend());              // set the legend                  //
  QwtPlotGrid* grid = new QwtPlotGrid();              // set the grid                    //
  grid->attach(m_Plot);                               // set the grid                    //
  m_Controls.verticalLayout->addWidget(m_Plot, 1);    // put the plot into the workbench //

  // the number of the plotted curve
  int curveIdx = 0;

  // iterate selected datanodes:
  for (auto node = dataManagerNodes.begin(); node != dataManagerNodes.end(); ++node)
  {
    mitk::BaseData* data = node->GetPointer()->GetData();

    // the current number of examined data point:
    int pointIdx = 0;

    if (data)
    {
      std::string curveNameString = "";

      node->GetPointer()->GetStringProperty("name", curveNameString);

      curveNameString += " point ";

      // test if this data item is an image or not (could also be a surface or something totally different)
      mitk::Image* image = dynamic_cast<mitk::Image*>(data);
      if (image)
      {
        // convert the MITK image to the ITK image //
        mitk::CastToItkImage(image, itkImage);


        ////////////////////////// main loop /////////////////////////////////
        int reflectanceIdx;
        itk::VariableLengthVector<double> reflectance;
        int channels;

        //  The loop goes through each point in the point set  //
        for (PtIterator = m_PointSet->Begin(0); PtIterator != m_PointSet->End(0); PtIterator++)
        {

          //  extract the reflectance on a given pixel  //
          Point = PtIterator.Value();
          image->GetGeometry(0)->WorldToIndex(Point, index);
          index2[0] = index[0];
          index2[1] = index[1];
          reflectance = itkImage->GetPixel(index2);
          channels = reflectance.GetNumberOfElements();

          //  plot the reflectance dynamically  //
          // create colors for each element in pointset.
          // "randomly" select colors for each added point by multiplying with prime.
          QColor curveColor((200 + (41 * curveIdx)) % 255, (89 * curveIdx) % 255,
              (37 * curveIdx) % 255);

          QString curveName(curveNameString.c_str());
          curveName.append(QString::number(pointIdx));

          QPolygonF qwtPoints;
          QwtPlotCurve* curve = new QwtPlotCurve();
          curve->setTitle(curveName);
          curve->setPen(curveColor, 4);
          curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
          QwtSymbol *symbol = new QwtSymbol(QwtSymbol::Ellipse,
              QBrush(Qt::yellow), QPen(Qt::black, 2), QSize(10, 10));

          for (reflectanceIdx = 0; reflectanceIdx < channels; reflectanceIdx++)
          {
            qwtPoints<< QPointF((double) reflectanceIdx,reflectance.GetElement(reflectanceIdx));
          }

          curve->setSamples(qwtPoints);
          curve->setSymbol(symbol);
          curve->attach(m_Plot);
          m_Plot->show();

          ++curveIdx;
          ++pointIdx;
        }
      }
    }
  }

}
