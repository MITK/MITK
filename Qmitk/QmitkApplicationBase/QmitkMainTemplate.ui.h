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
#include <vtkSTLReader.h>
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>
#include <mitkFloatProperty.h>
#include <qregexp.h>

mitk::FloatProperty::Pointer opacityprop=NULL;

void QmitkMainTemplate::fileNew()
{

}

void QmitkMainTemplate::fileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(NULL,"DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;stl files (*.stl)");

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
            //mitkMultiWidget->mitkWidget1->GetRenderer()->SetData(it);
            //mitkMultiWidget->mitkWidget2->GetRenderer()->SetData(it);
            //mitkMultiWidget->mitkWidget3->GetRenderer()->SetData(it);
            mitkMultiWidget->mitkWidget4->GetRenderer()->SetData(it);

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

	initWidgets(node);
	    
            //mitk::DataTreeNode::Pointer node2=mitk::DataTreeNode::New();
            //node2->SetData(mitkMultiWidget->mitkWidget3->GetRenderer()->GetWorldGeometry2DData());
            //it->add(node2);


            mitkMultiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);
            mitkMultiWidget->mitkWidget4->GetRenderer()->SetData(it);
            mitkMultiWidget->mitkWidget4->GetRenderer()->GetDisplayGeometry()->Fit();
            // schräg
            //initWidget(it,
            //	   mitkMultiWidget->mitkWidget4,
            //	   Vector3f(0,0,0),
            //	   Vector3f(header->n[0],header->n[1],0),
            //	   Vector3f(0,0,header->n[2])
            //	   );
            //       ipPicFree(header);
            delete it;
        }
    }
    if(node != NULL)
    {
        static int count = 0;
	if(count>0)
        {
            //sliderFP->setMinValue(0);
            //sliderFP->setMaxValue(100);
            //sliderFP->setValue(50);

            float color[3]={1.0f,0.0f,0.0f};
            mitk::ColorProperty::Pointer colorprop = new mitk::ColorProperty(color);
            node->GetPropertyList()->SetProperty("color", colorprop);

            //mitk::FloatProperty::Pointer 
                opacityprop = new mitk::FloatProperty(0.5f);
            node->GetPropertyList()->SetProperty("opacity", opacityprop);
        }
        ++count;
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

        initWidgets(node);

        mitkMultiWidget->updateMitkWidgets();

        delete it;
    }
}

void QmitkMainTemplate::initWidget(mitk::DataTreeIterator* it,
				   QmitkSelectableGLWidget* widget,
				   const Vector3f& normal, const Vector3f& right ) {
  
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
				   const Vector3f& bottom) {
  
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

  mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
  mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
  node->SetData(widget->GetRenderer()->GetWorldGeometry2DData());
  geometryMapper->SetDataIteratorForTexture(it);
  node->SetMapper(2, geometryMapper);
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
    //this seems to be a bug of Qt3.1.1's designer: The object name of ToolBar is not initialized.
    ToolBar->setName("ToolBar");

    //create the data tree
    tree=mitk::DataTree::New();
    tree->Register(); //FIXME: da DataTreeIterator keinen Smartpointer auf DataTree hält, wird tree sonst gelöscht.
    //create root of data tree (empty)
    mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
    tree->setRoot(node); 

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

void QmitkMainTemplate::destroy()
{
    tree->clear();
}

/*!
	\brief basic initialization of main widgets

	The method is should be called at the end of the initialize-method of its 
	subclasses.
*/
void QmitkMainTemplate::initialize()
{
		initializeQfm();
		initializeFunctionality();

    QWidget* defaultMain = qfm->getDefaultMain();

		if(defaultMain!=NULL)
    {
        QBoxLayout *layoutdraw=dynamic_cast<QBoxLayout *>(defaultMain->layout());
        if(layoutdraw==NULL)
           layoutdraw = new QHBoxLayout(defaultMain);
     
   layoutdraw->addWidget(mitkMultiWidget);
    }
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

		mitkMultiWidget = new QmitkStdMultiWidget(qfm->getDefaultMain(), "QmitkMainTemplate::QmitkStdMultiWidget");

}


mitk::DataTree* QmitkMainTemplate::getDataTree()
{
    return tree;
}


void QmitkMainTemplate::initWidgets( mitk::DataTreeNode::Pointer node )
{
           mitk::DataTreeIterator* it=tree->inorderIterator();
	   
	printf("\nrequesting boundingbox\n");   
            mitk::BoundingBox::ConstPointer bb = node->GetData()->GetGeometry()->GetBoundingBox();
	  printf("boundsArrayType\n");
            const mitk::BoundingBox::BoundsArrayType bounds = bb->GetBounds();
	printf("\nboundingbox\n");   

            //sliderYZ->setMinValue(bounds[0]);
            //sliderYZ->setMaxValue(bounds[1]);
            //sliderXZ->setMinValue(bounds[2]);
            //sliderXZ->setMaxValue(bounds[3]);
            //sliderXY->setMinValue(bounds[4]);
            //sliderXY->setMaxValue(bounds[5]);
            //// XY
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
}



QmitkStdMultiWidget* QmitkMainTemplate::getMultiWidget()
{
    return mitkMultiWidget;
}


