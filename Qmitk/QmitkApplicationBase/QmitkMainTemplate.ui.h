/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "mitkGeometry2DDataVtkMapper3D.h"
#include "QmitkSelectableGLWidget.h"
#include "QLevelWindowWidget.h"
#include <vtkSTLReader.h>
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkVesselTreeFileReader.h>
#include <LevelWindow.h>
#include <DataTree.h>
#include <mitkFloatProperty.h>
#include <qregexp.h>
#include <vtkImageThreshold.h>
#include <vtkImageData.h>
#include <vtkPointData.h>

mitk::FloatProperty::Pointer opacityprop=NULL;

void QmitkMainTemplate::fileNew()
{

}

void QmitkMainTemplate::fileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.stl *.ves);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;stl files (*.stl);;ves files (*.ves)");

    if ( !fileName.isNull() )
    {
        fileOpen(fileName.ascii());
        mitkMultiWidget->updateMitkWidgets();
    }
}

void QmitkMainTemplate::fileOpen( const char * fileName )
{
    mitk::DataTreeNode::Pointer node=NULL;
    if(strstr(fileName, ".stl")!=0)
    {
        std::cout << "loading " << fileName << " as stl ... " << std::endl;

        vtkSTLReader *stlreader = vtkSTLReader::New();
        stlreader->SetFileName(fileName);
        stlreader->Update();

        if(stlreader->GetOutput()!=NULL)
        {
            mitk::SurfaceData::Pointer surface = mitk::SurfaceData::New();
            surface->SetVtkPolyData(stlreader->GetOutput());
            //surface->Register(); //FIXME: das weakpointer-Problem! ...

            mitk::DataTreeIterator* it=tree->inorderIterator();

            node=mitk::DataTreeNode::New();
            node->SetData(surface);
            it->add(node);

            //FIXME: das folgende sollte überflüssig sein. Problem ist aber, das es z.Z. noch keine Möglichkeit gibt festzustellen, wann der Baum zuletzt geändert wurde.
            initWidgets(it);

            delete it;
        }
    }
    else
        if((strstr(fileName, ".pic")!=0) || (strstr(fileName, ".seq")!=0))
        {
            ipPicDescriptor *header=ipPicGetHeader(const_cast<char *>(fileName), NULL);
            if(header!=NULL)
            {
                mitk::PicFileReader::Pointer reader;

                reader=mitk::PicFileReader::New();
                std::cout << "loading " << fileName << " as pic ... " << std::endl;
                //            reader->Register(); //FIXME: das WeakPointer problem!

                reader->SetFileName(fileName);


                mitk::DataTreeIterator* it=tree->inorderIterator();

                node=mitk::DataTreeNode::New();
                node->SetData(reader->GetOutput());
                it->add(node); 

 /*   node=mitk::DataTreeNode::New();
    node->SetData(reader->GetOutput());
    it->add(node); */
/*
	mitk::Image::Pointer img=reader->GetOutput();
//	mitk::Image::Pointer img=dynamic_cast<mitk::Image*>((*it).get()->GetData());
	img->Update();

	vtkImageThreshold *threshold=vtkImageThreshold::New();
		threshold->DebugOn();
		threshold->SetInput(img->GetVtkImageData());
		threshold->ThresholdByLower(200);
		threshold->SetInValue(0);
		threshold->Update();

	mitk::Image::Pointer result=mitk::Image::New();
		result->Initialize(img->GetPic());
		result->SetVolume(threshold->GetOutput()->GetPointData()->GetScalars()->GetVoidPointer(0));
	
    node=mitk::DataTreeNode::New();
    node->SetData(result);
    it->add(node); 
*/
                initWidgets(it);

                //mitk::DataTreeNode::Pointer node2=mitk::DataTreeNode::New();
                //node2->SetData(mitkMultiWidget->mitkWidget3->GetRenderer()->GetWorldGeometry2DData());
                //it->add(node2);


                // schräg
                //initWidget(it,
                //	   mitkMultiWidget->mitkWidget4,
                //	   Vector3f(0,0,0),
                //	   Vector3f(header->n[0],header->n[1],0),
                //	   Vector3f(0,0,header->n[2])
                //	   );
                //       ipPicFree(header);
                delete it;

			    mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
			    levWinProp->SetLevelWindow(reader->GetOutput()->GetLevelWindow());
			    node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
            }
        }
        else if( strstr(fileName, ".ves")!=0 )
        {
            mitk::VesselTreeFileReader::Pointer reader = mitk::VesselTreeFileReader::New();
            std::cout << "loading " << fileName << " as ves ... " << std::endl;
            reader->SetFileName(fileName);          
            reader->Update();
            mitk::DataTreeIterator* it=tree->inorderIterator();

            node=mitk::DataTreeNode::New();
            node->SetData(reader->GetOutput());
            it->add(node); 

            initWidgets(it);
            delete it;
        }
}

void QmitkMainTemplate::fileOpenImageSequence()
{
    QString fileName = QFileDialog::getOpenFileName(NULL,"DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;stl files (*.stl)");

    if ( !fileName.isNull() )
    {
        int fnstart = fileName.findRev( QRegExp("[/\\\\]"), fileName.length() );
        if(fnstart<0) fnstart=0;
        int start = fileName.find( QRegExp("[0-9]"), fnstart );
        if(start<0)
        {
            fileOpen(fileName.ascii());
            return;
        }
        
        char prefix[1024], pattern[1024];

        strncpy(prefix, fileName.ascii(), start);
        prefix[start]=0;

        int stop=fileName.find( QRegExp("[^0-9]"), start );
        sprintf(pattern, "%%s%%0%uu%s",stop-start,fileName.ascii()+stop);

        mitk::PicFileReader::Pointer reader;
        reader=mitk::PicFileReader::New();

        std::cout << "loading " << fileName << " interpreted as image sequence mask: prefix " << prefix << " pattern: " << pattern << std::endl;

        reader->SetFilePrefix(prefix);
        reader->SetFilePattern(pattern);

        mitk::DataTreeIterator* it=tree->inorderIterator();

        mitk::DataTreeNode::Pointer node;
        node=mitk::DataTreeNode::New();
        node->SetData(reader->GetOutput());
        it->add(node);

        initWidgets(it);

        mitkMultiWidget->updateMitkWidgets();

        delete it;
    }
}

void QmitkMainTemplate::initWidget(mitk::DataTreeIterator* it,
				   QmitkSelectableGLWidget* widget,
				   const Vector3f& normal, const Vector3f& right )
{

    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();

    //FIXME: ohne den Pointer-Umweg meckert gcc  
    mitk::PlaneView* view = new mitk::PlaneView(Plane<float>(Vector3f(0,0,0),normal),
        right,
        Vector2f(0,0),
        Vector2f(0,0)
        );
    plane->SetPlaneView(*view);
    delete view;

    widget->GetRenderer()->SetData(it);
    widget->GetRenderer()->SetWorldGeometry(plane);
    widget->GetRenderer()->GetDisplayGeometry()->Fit();

    //mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
    //node->SetData(widget->GetRenderer()->GetWorldGeometry2DData());
    //it->add(node);

    widget->update();
}

void QmitkMainTemplate::initWidget(mitk::DataTreeIterator* it,
				   QmitkSelectableGLWidget* widget,
				   const Vector3f& origin, 
				   const Vector3f& right,
				   const Vector3f& bottom) 
{

    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();

    //FIXME: ohne den Pointer-Umweg meckert gcc  
    mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom,
        Vector2f(0,0),
        Vector2f(0,0)
        );
    plane->SetPlaneView(*view);
    delete view;

    widget->GetRenderer()->SetData(it);
    widget->GetRenderer()->SetWorldGeometry(plane);
    widget->GetRenderer()->GetDisplayGeometry()->Fit();

    //mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
    mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
    node->SetData(widget->GetRenderer()->GetWorldGeometry2DData());
    //geometryMapper->SetDataIteratorForTexture(it);
    //node->SetMapper(2, geometryMapper);
    it->add(node);
    widget->update();
}

void QmitkMainTemplate::fileSave()
{

}

void QmitkMainTemplate::fileSaveAs()
{

}

void QmitkMainTemplate::filePrint()
{

}

void QmitkMainTemplate::fileExit()
{

}

void QmitkMainTemplate::editUndo()
{

}

void QmitkMainTemplate::editRedo()
{

}

void QmitkMainTemplate::editCut()
{

}

void QmitkMainTemplate::editCopy()
{

}

void QmitkMainTemplate::editPaste()
{

}

void QmitkMainTemplate::editFind()
{

}

void QmitkMainTemplate::helpIndex()
{

}

void QmitkMainTemplate::helpContents()
{

}

void QmitkMainTemplate::helpAbout()
{

}


void QmitkMainTemplate::init()
{
    mitkMultiWidget=NULL;

    //this seems to be a bug of Qt3.1.1's designer: The object name of ToolBar is not initialized.
    ToolBar->setName("ToolBar");

    //create the data tree
    tree=mitk::DataTree::New();
    tree->Register(); //FIXME: da DataTreeIterator keinen Smartpointer auf DataTree hält, wird tree sonst gelöscht.
    //create root of data tree (empty)
    mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
    tree->setRoot(node); 
}

/*!
	\brief basic initialization of main widgets

	The method is should be called at the end of the initialize-method of its 
	subclasses.
*/
void QmitkMainTemplate::initialize()
{
    initializeQfm();
    QWidget* defaultMain = qfm->getDefaultMain();

    if(defaultMain!=NULL)
    {
        QBoxLayout *layoutdraw=dynamic_cast<QBoxLayout *>(defaultMain->layout());
        if(layoutdraw==NULL)
            layoutdraw = new QHBoxLayout(defaultMain);

		mitkMultiWidget = new QmitkStdMultiWidget(defaultMain, "QmitkMainTemplate::QmitkStdMultiWidget");
        layoutdraw->addWidget(mitkMultiWidget);

        connect(mitkMultiWidget->levelWindowWidget,SIGNAL(levelWindow(mitk::LevelWindow*)),this,SLOT(changeLevelWindow(mitk::LevelWindow*)) );
    }
    initializeFunctionality();
}


/*!
 
*/
void QmitkMainTemplate::initializeFunctionality()
{

}

/*!
	\brief this method initializes the Qmitk functionality mediator

	When subclassing this template class the developer can overwrite the method 
	to provide different layout templates
*/
void QmitkMainTemplate::initializeQfm()
{
    QHBoxLayout *hlayout;
    hlayout=new QHBoxLayout(MainWidget);   

	  //create an QmitkFctMediator. This is an invisible object that controls, manages and mediates functionalities
    qfm=new QmitkFctMediator(MainWidget);
		
    //create an QmitkButtonFctLayoutTemplate. This is an simple example for an layout of the different widgets, of which
    //a functionality and the management consists: the main widget, the control widget and a menu for selecting the
    //active functionality.
    QmitkControlsRightFctLayoutTemplate* layoutTemplate=new QmitkControlsRightFctLayoutTemplate(MainWidget, "QmitkControlsLeftFctLayoutTemplate");
    hlayout->addWidget(layoutTemplate);

    //let the QmitkFctMediator know about the layout. This includes the toolbar and the layoutTemplate.
    qfm->initialize(this);
}


mitk::DataTree* QmitkMainTemplate::getDataTree()
{
    return tree;
}


void QmitkMainTemplate::initWidgets( mitk::DataTreeIterator * it )
{
		
	const mitk::BoundingBox::BoundsArrayType bounds = mitk::DataTree::ComputeBoundingBox(it)->GetBounds();
	printf("\nboundingbox\n");   

            initWidget(it,
                mitkMultiWidget->mitkWidget1,
                Vector3f(bounds[0],bounds[2],bounds[4]),
                Vector3f(bounds[1],bounds[2],bounds[4]),
                Vector3f(bounds[0],bounds[3],bounds[4])
                );
	printf("\nw1 init\n");   

            // YZ
            initWidget(it,mitkMultiWidget->mitkWidget2,
                Vector3f(bounds[0],bounds[2],bounds[4]),
                Vector3f(bounds[0],bounds[3],bounds[4]),
                Vector3f(bounds[0],bounds[2],bounds[5])
                );
            // XZ
            initWidget(it,
                mitkMultiWidget->mitkWidget3,
                Vector3f(bounds[0],bounds[2],bounds[4]),
                Vector3f(bounds[1],bounds[2],bounds[4]),
                Vector3f(bounds[0],bounds[2],bounds[5])
            );
    mitkMultiWidget->mitkWidget4->GetRenderer()->SetData(it);
}



QmitkStdMultiWidget* QmitkMainTemplate::getMultiWidget()
{
    return mitkMultiWidget;
}

void QmitkMainTemplate::parseCommandLine()
{
    //command line arguments set?
    //The following is a simple method to assess command line arguments.
    int i;
    for(i=1;i<qApp->argc();++i)
    {
    //    const char *param=qApp->argv()[i];
    //    if(param[0]=='-')
    //    {
    //        if(strcmp(param,"-hrm")==0)
    //            hrm=true;
    //        if(strcmp(param,"-number")==0) // e.g., -number 7
    //        {
    //            number= atoi(qApp->argv()[++i]);
    //        }
    //    }
    }
    if(qApp->argc()>1)
        fileOpen(qApp->argv()[qApp->argc()-1]);
}



void QmitkMainTemplate::changeLevelWindow(mitk::LevelWindow* lw )
{
	
	mitk::DataTreeIterator* it=tree->inorderIterator()->clone();
	while(it->hasNext()) 
    {
        it->next();
	
		mitk::LevelWindowProperty::Pointer levWinProp = dynamic_cast<mitk::LevelWindowProperty*>(it->get()->GetPropertyList()->GetProperty("levelwindow").GetPointer());
		if(levWinProp!=NULL) 
        {
			mitk::LevelWindow levWin = levWinProp->GetLevelWindow();

			levWin.SetMin(lw->GetMin());
			levWin.SetMax(lw->GetMax());
			levWin.SetRangeMin(lw->GetRangeMin());
			levWin.SetRangeMax(lw->GetRangeMax());
			
			levWinProp->SetLevelWindow(levWin);

		}                  
	}
	delete it;
  mitkMultiWidget->updateMitkWidgets();
}
