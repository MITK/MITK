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


#include "QmitkScreenshotMaker.h"
//#include "QmitkMovieMakerControls.h"
#include "QmitkStepperAdapter.h"

#include "mitkVtkPropRenderer.h"
#include <QmitkRenderWindow.h>

#include <iostream>

#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <qaction.h>
#include <qfiledialog.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcolor.h>
#include <qcolordialog.h>

#include "qapplication.h"

#include "vtkImageWriter.h"
#include "vtkJPEGWriter.h"
#include "vtkPNGWriter.h"
#include "vtkRenderLargeImage.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTestUtilities.h"

#include <vtkActor.h>
#include "vtkMitkRenderProp.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkRenderWindowInteractor.h"
#include <qradiobutton.h>

#include "mitkSliceNavigationController.h"
#include "mitkPlanarFigure.h"

QmitkScreenshotMaker::QmitkScreenshotMaker(QObject *parent, const char * /*name*/)
  : QmitkAbstractView(),
    m_Controls(nullptr),
    m_SelectedNode(0),
    m_BackgroundColor(QColor(0,0,0))
{
  parentWidget = parent;
}

QmitkScreenshotMaker::~QmitkScreenshotMaker()
{
}

void QmitkScreenshotMaker::CreateConnections()
{
    if (m_Controls)
    {
        connect((QObject*) m_Controls->m_AllViews, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateMultiplanar3DHighresScreenshot()));
        connect((QObject*) m_Controls->m_View1, SIGNAL(clicked()), (QObject*) this, SLOT(View1()));
        connect((QObject*) m_Controls->m_View2, SIGNAL(clicked()), (QObject*) this, SLOT(View2()));
        connect((QObject*) m_Controls->m_View3, SIGNAL(clicked()), (QObject*) this, SLOT(View3()));
        connect((QObject*) m_Controls->m_Shot, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateMultiplanarScreenshots()));
        connect((QObject*) m_Controls->m_BackgroundColor, SIGNAL(clicked()), (QObject*) this, SLOT(SelectBackgroundColor()));
        connect((QObject*) m_Controls->btnScreenshot, SIGNAL(clicked()), this, SLOT(GenerateScreenshot()));
        connect((QObject*) m_Controls->m_HRScreenshot, SIGNAL(clicked()), this, SLOT(Generate3DHighresScreenshot()));

        QString styleSheet = "background-color:rgb(0,0,0)";
        m_Controls->m_BackgroundColor->setStyleSheet(styleSheet);
    }
}

void QmitkScreenshotMaker::GenerateScreenshot()
{
    if (m_LastFile.size()==0)
        m_LastFile = QDir::currentPath()+"/screenshot.png";

    QString filter;
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save screenshot to...", m_LastFile, m_PNGExtension + ";;" + m_JPGExtension, &filter);

    if (fileName.size()>0)
        m_LastFile = fileName;

    auto renderWindowPart = this->GetRenderWindowPart(OPEN);
    mitk::BaseRenderer* renderer = renderWindowPart->GetActiveQmitkRenderWindow()->GetRenderer();

    if (renderer == nullptr)
    {
       renderer = renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer();
       if (renderer == nullptr)
          return;
    }
    this->TakeScreenshot(renderer->GetVtkRenderer(), 1, fileName, filter);
}

void QmitkScreenshotMaker::GenerateMultiplanarScreenshots()
{
    if (m_LastPath.size()==0)
        m_LastPath = QDir::currentPath();
    QString filePath = QFileDialog::getExistingDirectory(nullptr, "Save screenshots to...", m_LastPath);
    if (filePath.size()>0)
        m_LastPath = filePath;

    if( filePath.isEmpty() )
    {
        return;
    }

    //emit StartBlockControls();
    auto renderWindowPart = this->GetRenderWindowPart(OPEN);
    renderWindowPart->EnableDecorations(false, QStringList{mitk::IRenderWindowPart::DECORATION_CORNER_ANNOTATION});

    QString fileName = "/axial.png";
    int c = 1;
    while (QFile::exists(filePath+fileName))
    {
        fileName = QString("/axial_");
        fileName += QString::number(c);
        fileName += ".png";
        c++;
    }
    vtkRenderer* renderer = renderWindowPart->GetQmitkRenderWindow("axial")->GetRenderer()->GetVtkRenderer();
    if (renderer != nullptr)
        this->TakeScreenshot(renderer, 1, filePath+fileName);

    fileName = "/sagittal.png";
    c = 1;
    while (QFile::exists(filePath+fileName))
    {
        fileName = QString("/sagittal_");
        fileName += QString::number(c);
        fileName += ".png";
        c++;
    }
    renderer = renderWindowPart->GetQmitkRenderWindow("sagittal")->GetRenderer()->GetVtkRenderer();
    if (renderer != nullptr)
        this->TakeScreenshot(renderer, 1, filePath+fileName);

    fileName = "/coronal.png";
    c = 1;
    while (QFile::exists(filePath+fileName))
    {
        fileName = QString("/coronal_");
        fileName += QString::number(c);
        fileName += ".png";
        c++;
    }
    renderer = renderWindowPart->GetQmitkRenderWindow("coronal")->GetRenderer()->GetVtkRenderer();
    if (renderer != nullptr)
        this->TakeScreenshot(renderer, 1, filePath+fileName);

    /// TODO I do not find a simple way of doing this through the render window part API,
    /// however, I am also not convinced that this code is needed at all. The colour
    /// of the crosshair planes is never set to any colour other than these.
    /// I suggest a new 'mitk::DataNode* mitk::ILinkedRendererPart::GetSlicingPlane(const std::string& name) const'
    /// function to introduce that could return the individual ("axial", "sagittal" or
    /// "coronal" crosshair planes.

//    mitk::DataNode* n = renderWindowPart->GetSlicingPlane("axial");
//    if (n)
//    {
//        n->SetProperty( "color", mitk::ColorProperty::New( 1,0,0 ) );
//    }
//
//    n = renderWindowPart->GetSlicingPlane("sagittal");
//    if (n)
//    {
//        n->SetProperty( "color", mitk::ColorProperty::New( 0,1,0 ) );
//    }
//
//    n = renderWindowPart->GetSlicingPlane("coronal");
//    if (n)
//    {
//        n->SetProperty( "color", mitk::ColorProperty::New( 0,0,1 ) );
//    }

    renderWindowPart->EnableDecorations(true, QStringList{mitk::IRenderWindowPart::DECORATION_CORNER_ANNOTATION});
}

void QmitkScreenshotMaker::Generate3DHighresScreenshot()
{
    if (m_LastFile.size()==0)
        m_LastFile = QDir::currentPath()+"/3D_screenshot.png";

    QString filter;
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save screenshot to...", m_LastFile, m_PNGExtension + ";;" + m_JPGExtension, &filter);

    if (fileName.size()>0)
        m_LastFile = fileName;
    GenerateHR3DAtlasScreenshots(fileName, filter);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::GenerateMultiplanar3DHighresScreenshot()
{
    if (m_LastPath.size()==0)
        m_LastPath = QDir::currentPath();
    QString filePath = QFileDialog::getExistingDirectory( nullptr, "Save screenshots to...", m_LastPath);
    if (filePath.size()>0)
        m_LastPath = filePath;

    if( filePath.isEmpty() )
    {
        return;
    }

    QString fileName = "/3D_View1.png";
    int c = 1;
    while (QFile::exists(filePath+fileName))
    {
        fileName = QString("/3D_View1_");
        fileName += QString::number(c);
        fileName += ".png";
        c++;
    }
    GetCam()->Azimuth( -7.5 );
    GetCam()->Roll(-4);
    GenerateHR3DAtlasScreenshots(filePath+fileName);
    GetCam()->Roll(4);

    fileName = "/3D_View2.png";
    c = 1;
    while (QFile::exists(filePath+fileName))
    {
        fileName = QString("/3D_View2_");
        fileName += QString::number(c);
        fileName += ".png";
        c++;
    }
    GetCam()->Azimuth( 90 );
    GetCam()->Elevation( 4 );
    GenerateHR3DAtlasScreenshots(filePath+fileName);

    fileName = "/3D_View3.png";
    c = 1;
    while (QFile::exists(filePath+fileName))
    {
        fileName = QString("/3D_View3_");
        fileName += QString::number(c);
        fileName += ".png";
        c++;
    }
    GetCam()->Elevation( 90 );
    GetCam()->Roll( -2.5 );
    GenerateHR3DAtlasScreenshots(filePath+fileName);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::GenerateHR3DAtlasScreenshots(QString fileName, QString filter)
{
    // only works correctly for 3D RenderWindow
    this->GetRenderWindowPart()->EnableDecorations(false, QStringList{mitk::IRenderWindowPart::DECORATION_CORNER_ANNOTATION});
    vtkRenderer* renderer = this->GetRenderWindowPart()->GetQmitkRenderWindow("3d")->GetRenderer()->GetVtkRenderer();
    if (renderer == nullptr)
        return;
    this->TakeScreenshot(renderer, this->m_Controls->m_MagFactor->text().toFloat(), fileName, filter);
    this->GetRenderWindowPart()->EnableDecorations(true, QStringList{mitk::IRenderWindowPart::DECORATION_CORNER_ANNOTATION});
}

vtkCamera* QmitkScreenshotMaker::GetCam()
{
    mitk::BaseRenderer* renderer = this->GetRenderWindowPart(OPEN)->GetQmitkRenderWindow("3d")->GetRenderer();
    vtkCamera* cam = 0;
    const mitk::VtkPropRenderer *propRenderer = dynamic_cast<const mitk::VtkPropRenderer * >( renderer );
    if (propRenderer)
    {
        // get vtk renderer
        vtkRenderer* vtkrenderer = propRenderer->GetVtkRenderer();
        if (vtkrenderer)
        {
            // get vtk camera
            vtkCamera* vtkcam = vtkrenderer->GetActiveCamera();
            if (vtkcam)
            {
                // vtk smart pointer handling
                cam = vtkcam;
                cam->Register( nullptr );
            }
        }
    }
    return cam;
}

void QmitkScreenshotMaker::View1()
{
    GetCam()->Elevation( 45 );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::View2()
{
    GetCam()->Azimuth(45);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::View3()
{
    GetCam()->Roll(45);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
    if(nodes.size())
        m_SelectedNode = nodes[0];
}

void QmitkScreenshotMaker::CreateQtPartControl(QWidget *parent)
{
    if (!m_Controls)
    {
        m_Parent = parent;
        m_Controls = new Ui::QmitkScreenshotMakerControls;
        m_Controls->setupUi(parent);

        // Initialize "Selected Window" combo box
        const mitk::RenderingManager::RenderWindowVector rwv =
                mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

    }

    this->CreateConnections();

}

void QmitkScreenshotMaker::SetFocus()
{
  m_Controls->btnScreenshot->setFocus();
}

void QmitkScreenshotMaker::RenderWindowPartActivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
    m_Parent->setEnabled(true);
}

void QmitkScreenshotMaker::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
    m_Parent->setEnabled(false);
}

void QmitkScreenshotMaker::TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName, QString filter)
{
    if ((renderer == nullptr) ||(magnificationFactor < 1) || fileName.isEmpty())
        return;

    bool doubleBuffering( renderer->GetRenderWindow()->GetDoubleBuffer() );
    renderer->GetRenderWindow()->DoubleBufferOff();

    vtkImageWriter* fileWriter = nullptr;

    QFileInfo fi(fileName);
    QString suffix = fi.suffix().toLower();

    if (suffix.isEmpty() || (suffix != "png" && suffix != "jpg" && suffix != "jpeg"))
    {
      if (filter == m_PNGExtension)
      {
        suffix = "png";
      }
      else if (filter == m_JPGExtension)
      {
        suffix = "jpg";
      }
      fileName += "." + suffix;
    }

    if (suffix.compare("jpg", Qt::CaseInsensitive) == 0 || suffix.compare("jpeg", Qt::CaseInsensitive) == 0)
    {
      vtkJPEGWriter* w = vtkJPEGWriter::New();
      w->SetQuality(100);
      w->ProgressiveOff();
      fileWriter = w;
    }
    else //default is png
    {
      fileWriter = vtkPNGWriter::New();
    }

    vtkRenderLargeImage* magnifier = vtkRenderLargeImage::New();
    magnifier->SetInput(renderer);
    magnifier->SetMagnification(magnificationFactor);
    //magnifier->Update();
    fileWriter->SetInputConnection(magnifier->GetOutputPort());
    fileWriter->SetFileName(fileName.toLatin1());

    // vtkRenderLargeImage has problems with different layers, therefore we have to
    // temporarily deactivate all other layers.
    // we set the background to white, because it is nicer than black...
    double oldBackground[3];
    renderer->GetBackground(oldBackground);


    //  QColor color = QColorDialog::getColor();
    double bgcolor[] = {m_BackgroundColor.red()/255.0, m_BackgroundColor.green()/255.0, m_BackgroundColor.blue()/255.0};
    renderer->SetBackground(bgcolor);

    mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();

    renderWindowPart->EnableDecorations(false);

    fileWriter->Write();
    fileWriter->Delete();

    renderWindowPart->EnableDecorations(true);

    renderer->SetBackground(oldBackground);

    renderer->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
}

void QmitkScreenshotMaker::SelectBackgroundColor()
{
    m_BackgroundColor = QColorDialog::getColor();

    m_Controls->m_BackgroundColor->setAutoFillBackground(true);


    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(m_BackgroundColor.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(m_BackgroundColor.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(m_BackgroundColor.blue()));
    styleSheet.append(")");
    m_Controls->m_BackgroundColor->setStyleSheet(styleSheet);
}
