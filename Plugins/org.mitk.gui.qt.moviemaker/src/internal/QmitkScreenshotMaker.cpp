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
#include "QmitkStdMultiWidget.h"

#include "mitkVtkPropRenderer.h"
#include "mitkGlobalInteraction.h"

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

    :
      QmitkFunctionality(), m_Controls(NULL),
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

void QmitkScreenshotMaker::Activated()
{
    QmitkFunctionality::Activated();
}

void QmitkScreenshotMaker::Deactivated()
{
    QmitkFunctionality::Deactivated();
}

void QmitkScreenshotMaker::GenerateScreenshot()
{
    if (m_LastFile.size()==0)
        m_LastFile = QDir::currentPath()+"/screenshot.png";
    QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", m_LastFile, "PNG file (*.png);;JPEG file (*.jpg)");
    if (fileName.size()>0)
        m_LastFile = fileName;

    vtkRenderer* renderer = mitk::GlobalInteraction::GetInstance()->GetFocus()->GetVtkRenderer();
    if (renderer == NULL)
        return;
    this->TakeScreenshot(renderer, 1, fileName);
}

void QmitkScreenshotMaker::GenerateMultiplanarScreenshots()
{
    if (m_LastPath.size()==0)
        m_LastPath = QDir::currentPath();
    QString filePath = QFileDialog::getExistingDirectory(NULL, "Save screenshots to...", m_LastPath);
    if (filePath.size()>0)
        m_LastPath = filePath;

    if( filePath.isEmpty() )
    {
        return;
    }

    //emit StartBlockControls();

    mitk::DataNode* n;

    QString fileName = "/axial.png";
    int c = 1;
    while (QFile::exists(filePath+fileName))
    {
        fileName = QString("/axial_");
        fileName += QString::number(c);
        fileName += ".png";
        c++;
    }
    vtkRenderer* renderer = m_MultiWidget->mitkWidget1->GetRenderer()->GetVtkRenderer();
    if (renderer != NULL)
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
    renderer = m_MultiWidget->mitkWidget2->GetRenderer()->GetVtkRenderer();
    if (renderer != NULL)
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
    renderer = m_MultiWidget->mitkWidget3->GetRenderer()->GetVtkRenderer();
    if (renderer != NULL)
        this->TakeScreenshot(renderer, 1, filePath+fileName);

    n = this->m_MultiWidget->GetWidgetPlane1();
    if(n)
    {
        n->SetProperty( "color", mitk::ColorProperty::New( 1,0,0 ) );
    }

    n = this->m_MultiWidget->GetWidgetPlane2();
    if(n)
    {
        n->SetProperty( "color", mitk::ColorProperty::New( 0,1,0 ) );
    }

    n = this->m_MultiWidget->GetWidgetPlane3();
    if(n)
    {
        n->SetProperty( "color", mitk::ColorProperty::New( 0,0,1 ) );
    }
}

void QmitkScreenshotMaker::Generate3DHighresScreenshot()
{
    if (m_LastFile.size()==0)
        m_LastFile = QDir::currentPath()+"/3D_screenshot.png";
    QString fileName = QFileDialog::getSaveFileName(NULL, "Save screenshot to...", m_LastFile, "PNG file (*.png);;JPEG file (*.jpg)");
    if (fileName.size()>0)
        m_LastFile = fileName;
    GenerateHR3DAtlasScreenshots(fileName);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkScreenshotMaker::GenerateMultiplanar3DHighresScreenshot()
{
    if (m_LastPath.size()==0)
        m_LastPath = QDir::currentPath();
    QString filePath = QFileDialog::getExistingDirectory( NULL, "Save screenshots to...", m_LastPath);
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

void QmitkScreenshotMaker::GenerateHR3DAtlasScreenshots(QString fileName)
{
    // only works correctly for 3D RenderWindow
    vtkRenderer* renderer = m_MultiWidget->mitkWidget4->GetRenderer()->GetVtkRenderer();
    if (renderer == NULL)
        return;
    this->TakeScreenshot(renderer, this->m_Controls->m_MagFactor->text().toFloat(), fileName);
}

vtkCamera* QmitkScreenshotMaker::GetCam()
{
    mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow());
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
                cam->Register( NULL );
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

void QmitkScreenshotMaker::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    if(nodes.size())
        m_SelectedNode = nodes[0];
}

void QmitkScreenshotMaker::CreateQtPartControl(QWidget *parent)
{
    if (!m_Controls)
    {
        m_Controls = new Ui::QmitkScreenshotMakerControls;
        m_Controls->setupUi(parent);

        // Initialize "Selected Window" combo box
        const mitk::RenderingManager::RenderWindowVector rwv =
                mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

    }

    this->CreateConnections();

}

void QmitkScreenshotMaker::StdMultiWidgetAvailable(QmitkStdMultiWidget&  stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
    m_Parent->setEnabled(true);
}

void QmitkScreenshotMaker::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
    m_Parent->setEnabled(false);
}

void QmitkScreenshotMaker::TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName)
{
    if ((renderer == NULL) ||(magnificationFactor < 1) || fileName.isEmpty())
        return;

    bool doubleBuffering( renderer->GetRenderWindow()->GetDoubleBuffer() );
    renderer->GetRenderWindow()->DoubleBufferOff();

    vtkImageWriter* fileWriter;

    QFileInfo fi(fileName);
    QString suffix = fi.suffix();
    if (suffix.compare("jpg", Qt::CaseInsensitive) == 0)
    {
        vtkJPEGWriter* w = vtkJPEGWriter::New();
        w->SetQuality(100);
        w->ProgressiveOff();
        fileWriter = w;

    }
    else  // default is png
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

    m_MultiWidget->DisableColoredRectangles();
    m_MultiWidget->DisableDepartmentLogo();
    m_MultiWidget->DisableGradientBackground();

    m_MultiWidget->mitkWidget1->ActivateMenuWidget( false );
    m_MultiWidget->mitkWidget2->ActivateMenuWidget( false );
    m_MultiWidget->mitkWidget3->ActivateMenuWidget( false );
    m_MultiWidget->mitkWidget4->ActivateMenuWidget( false );

    fileWriter->Write();
    fileWriter->Delete();

    m_MultiWidget->mitkWidget1->ActivateMenuWidget( true, m_MultiWidget );
    m_MultiWidget->mitkWidget2->ActivateMenuWidget( true, m_MultiWidget );
    m_MultiWidget->mitkWidget3->ActivateMenuWidget( true, m_MultiWidget );
    m_MultiWidget->mitkWidget4->ActivateMenuWidget( true, m_MultiWidget );

    m_MultiWidget->EnableColoredRectangles();
    m_MultiWidget->EnableDepartmentLogo();
    m_MultiWidget->EnableGradientBackground();
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
