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
#include <vtkSTLWriter.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkPImageWriter.h>
#include <vtkPImageWriter.h>
#include <map>
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>
#include <mitkLevelWindowProperty.h>
#include <mitkVesselTreeFileReader.h>
#include <LevelWindow.h>
#include <DataTree.h>
#include <mitkFloatProperty.h>
#include <qregexp.h>

#include <mitkStringProperty.h>
#include <qstring.h>
#include "ImageTimeSelector.h"
#include "ImageChannelSelector.h"
//#include <mitkImageToItk.h>
//#include <itkThresholdImageFilter.h>

#include <StateMachineFactory.h>
#include <UndoController.h>
#include <StateMachine.h>
#include <EventMapper.h>
#include <GlobalInteraction.h>

#ifdef MITK_DICOM_ENABLED
#include <mitkDICOMFileReader.h>
#endif

#ifdef MITK_INTERNAL
#include <mitkDSRFileReader.h>
#include <mitkCylindricToCartesianFilter.h>
#endif

#include <mitkInteractionConst.h>

mitk::FloatProperty::Pointer opacityprop=NULL;

void QmitkMainTemplate::fileNew()
{

}

void QmitkMainTemplate::fileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(NULL,"all (*.seq *.pic *.pic.gz *.seq.gz *.pvtk *.stl *.vtk *.ves);;DKFZ Pic (*.seq *.pic *.pic.gz *.seq.gz);;surface files (*.stl *.vtk);;stl files (*.stl);;vtk surface files (*.vtk);;vtk image files (*.pvtk);;ves files (*.ves)");

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

            mitk::DataTreeIterator* it=tree->inorderIterator();

            node=mitk::DataTreeNode::New();
            node->SetData(surface);
            it->add(node);

            initWidgets(it);

            delete it;
        }
    }
    else
    if(strstr(fileName, ".vtk")!=0)
    {
        std::cout << "loading " << fileName << " as vtk ... " << std::endl;

        vtkPolyDataReader *reader = vtkPolyDataReader::New();
			reader->SetFileName(fileName);
			reader->Update();

        if(reader->GetOutput()!=NULL)
        {
            mitk::SurfaceData::Pointer surface = mitk::SurfaceData::New();
            surface->SetVtkPolyData(reader->GetOutput());
            
            mitk::DataTreeIterator* it=tree->inorderIterator();

            node=mitk::DataTreeNode::New();
            node->SetData(surface);
            it->add(node);

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
            
         QString tmpName = fileName;
         tmpName = tmpName.right(tmpName.length() - tmpName.findRev("/") - 1);     
         mitk::StringProperty::Pointer nameProp = new mitk::StringProperty(tmpName.ascii());

         node->SetProperty("fileName",nameProp);
        
			initWidgets(it);


	/*
	the following is for testing the itk-adaptor - will be removed soon.
	typedef signed short   PixelType;
	const unsigned int Dimension = 3;

	typedef itk::Image< PixelType, Dimension > ImageType;

	mitk::ImageToItk<PixelType, Dimension>::Pointer myImageToItkWrapper = mitk::ImageToItk<PixelType, Dimension>::New();
	myImageToItkWrapper->SetMitkImage(reader->GetOutput());

	typedef itk::ThresholdImageFilter<ImageType> myFilterType;
	myFilterType::Pointer myFilter = myFilterType::New();
	myFilter->SetInput( myImageToItkWrapper );
	myFilter->ThresholdAbove(100);
	myFilter->ThresholdBelow(10);
	myFilter->Update();

	unsigned int i, dimensions[Dimension];
	for(i=0;i<Dimension;++i) dimensions[i]=myFilter->GetOutput()->GetLargestPossibleRegion().GetSize().GetSize()[i];

	mitk::Image::Pointer imgWithitkFilterResult = mitk::Image::New();
	//	imgWithitkFilterResult->Initialize(mitk::PixelType(typeid(PixelType)), 
	//		Dimension, 
	//		dimensions);
	imgWithitkFilterResult->InitializeByItk(myFilter->GetOutput());
	imgWithitkFilterResult->SetVolume(myFilter->GetOutput()->GetBufferPointer());

	node=mitk::DataTreeNode::New();
	node->SetData(imgWithitkFilterResult);
	it->add(node); 
	*/
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

            reader->Update();
			mitk::LevelWindowProperty::Pointer levWinProp = new mitk::LevelWindowProperty();
            
            mitk::LevelWindow levelWindow;
            levelWindow.SetAuto( reader->GetOutput()->GetPic() );
			levWinProp->SetLevelWindow(levelWindow);
			node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
            mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);
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
#ifdef MITK_DICOM_ENABLED
	else
    if(strstr(fileName, "DCM")!=0)
	{
		mitk::DICOMFileReader::Pointer reader;

		reader=mitk::DICOMFileReader::New();
		std::cout << "loading " << fileName << " as pic ... " << std::endl;

		reader->SetFileName(fileName);

		mitk::DataTreeIterator* it=tree->inorderIterator();

		node=mitk::DataTreeNode::New();
		node->SetData(reader->GetOutput());
		it->add(node); 

        mitk::LevelWindow levelWindow;
        reader->Update();
        levelWindow.SetAuto( reader->GetOutput()->GetPic() );
		node->SetLevelWindow(levelWindow, NULL);
        mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);

		initWidgets(it);
	}
#endif
#ifdef MITK_INTERNAL
	else
    if(strstr(fileName, "Native")!=0)
	{
		mitk::DSRFileReader::Pointer reader;

		reader=mitk::DSRFileReader::New();
		std::cout << "loading " << fileName << " as pic ... " << std::endl;

		reader->SetFileName(fileName);
        mitk::ImageTimeSelector::Pointer timeSelector=mitk::ImageTimeSelector::New();
		timeSelector->SetInput(reader->GetOutput());
	    
		mitk::CylindricToCartesianFilter::Pointer cyl2cart;
      	cyl2cart = mitk::CylindricToCartesianFilter::New();
		mitk::DataTreeIterator* it=tree->inorderIterator();
        
		// Doppler information
		/*timeSelector->SetChannelNr(1);
	 	timeSelector->SetTimeNr(0);
        timeSelector->Update();
       
		mitk::CylindricToCartesianFilter::Pointer cyl2cart;
      	cyl2cart = mitk::CylindricToCartesianFilter::New();
        
		cyl2cart->SetInput(timeSelector->GetOutput());
		mitk::DataTreeIterator* it=tree->inorderIterator();

		node=mitk::DataTreeNode::New();
		node->SetData(cyl2cart->GetOutput());
		it->add(node); 
		node->SetColor(0.3,0.9, 0.2, mitkMultiWidget->mitkWidget2->GetRenderer());
		node->Update();
    */
	    // Backscatter information	
        timeSelector->SetChannelNr(0);
		timeSelector->Update();
		cyl2cart->SetInput(timeSelector->GetOutput());
		node=mitk::DataTreeNode::New();
		cyl2cart->Update();
		node->SetData(cyl2cart->GetOutput());
		it->add(node); 
		//node->SetColor(0.0,0.9, 0.2, mitkMultiWidget->mitkWidget1->GetRenderer());
		//node->SetColor(0.0,0.1, 0.9, mitkMultiWidget->mitkWidget3->GetRenderer());
		node->Update();

///************MeshHandle

//		mitk::SurfaceData::Pointer surfaceDataHandle = mitk::SurfaceData::New();
//		MyvtkAblationMeshHandle * myMeshHandle= MyvtkAblationMeshHandle::New();
//		myMeshHandle->setRadius(5);
//		myMeshHandle->setLength(100);
//		myMeshHandle->GenerateMeshHandle();
//
//		surfaceDataHandle->SetVtkPolyData(myMeshHandle->GetPolyData());
//        node=mitk::DataTreeNode::New();
//		node->SetData(surfaceDataHandle);
//		node->Update();
//		it->add(node);		
/////****************
//
//		mitk::SurfaceData::Pointer surfaceData = mitk::SurfaceData::New();
//		MyvtkAblationMesh * myMesh= MyvtkAblationMesh::New();
//		myMesh->setAngle(30);
////		myMesh->setLength(20);
//		
//		//die Parametern sollten plausibel angegeben werden
//    	myMesh->setWidthOfHead(20);
//		myMesh->setNumberOfSectors(5);
//		myMesh->setPartionsOfSector(1,1);
//		myMesh->setPartionsOfSector(2,2);
//		myMesh->setPartionsOfSector(3,3);
//		myMesh->setPartionsOfSector(4,4);
//		myMesh->setPartionsOfSector(5,5);
//		myMesh->setSectorLength(1,50);
//		myMesh->setSectorLength(2,50);
//		myMesh->setSectorLength(3,50);
//		myMesh->setSectorLength(4,50);
//		myMesh->setSectorLength(5,50);
//		myMesh->setTubeRadius(5);
//
//		myMesh->GenerateMesh();
//		//Maptest****************************************
//        int cellindex = myMesh->GetCellIndex("QE1.1");
//		cellindex = myMesh->GetCellIndex("QE2.1");
//		cellindex = myMesh->GetCellIndex("QE3.2");
//		cellindex = myMesh->GetCellIndex("LE1.1");
//		cellindex = myMesh->GetCellIndex("LE3.1");
//		cellindex = myMesh->GetCellIndex("LE2.2");
//        cellindex = myMesh->GetCellIndex("LE2.3");
//		cellindex = myMesh->GetCellIndex("LE3.4");
//		cellindex = myMesh->GetCellIndex("LE3.5");
//		cellindex = myMesh->GetCellIndex("LE4.2");
//		
//
//		myMesh->GetCellIndex("QE1.1");
//       //Maptest*****************************************
//
//		surfaceData->SetVtkPolyData(myMesh->GetPolyData());
//        node=mitk::DataTreeNode::New();
//		node->SetData(surfaceData);
//		bool wireframe=true;
//		node->SetVisibility(wireframe, this->mitkMultiWidget->mitkWidget2->GetRenderer(),"wireframe");
//		this->mitkMultiWidget->updateMitkWidgets(); 
//		//		node->SetProperty("wireframe",true);
//		it->add(node);
//
//		node->Update();
		
		mitk::LevelWindow levelWindow;
        reader->Update();
        levelWindow.SetAuto( cyl2cart->GetOutput()->GetPic() );
		node->SetLevelWindow(levelWindow, NULL);
		
        mitkMultiWidget->levelWindowWidget->setLevelWindow(levelWindow);

		initWidgets(it);
	}
#endif
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

    //ohne den Pointer-Umweg meckert gcc  
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

    //ohne den Pointer-Umweg meckert gcc  
    mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom);
    plane->SetPlaneView(*view);
    delete view;

    widget->GetRenderer()->SetData(it);
    widget->GetRenderer()->SetWorldGeometry(plane);
    widget->GetRenderer()->GetDisplayGeometry()->Fit();

    widget->update();
}

void QmitkMainTemplate::fileSave()
{

}

void QmitkMainTemplate::fileSaveAs()
{
    mitk::DataTreeIterator* it=tree->inorderIterator()->clone();
    while(it->hasNext()) 
    {
        it->next();
		mitk::DataTreeNode::Pointer node=it->get();
		if(node->GetData()!=NULL)
		{
			mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
			if(image!=NULL)
			{
				QString fileName = QFileDialog::getSaveFileName(NULL,"pvtk (*.pvtk)");
				if ( !fileName.isNull() )
				{
					if(fileName.endsWith(".pvtk")==false) fileName+=".pvtk";
					vtkPImageWriter *writer=vtkPImageWriter::New();
						writer->SetInput(image->GetVtkImageData());
						writer->SetFileName(fileName.ascii());
						writer->Write();
						writer->Delete();
				}
			}
			mitk::SurfaceData::Pointer surface = dynamic_cast<mitk::SurfaceData*>(node->GetData());
			if(image!=NULL)
			{
				QString fileName = QFileDialog::getSaveFileName(NULL,"stl (*.stl);;vtk (*.vtk)");
				if ( !fileName.isNull() )
				{
					if(fileName.endsWith(".vtk")) 
					{
						vtkPolyDataWriter *writer=vtkPolyDataWriter ::New();
							writer->SetInput(surface->GetVtkPolyData());
							writer->SetFileName(fileName.ascii());
							writer->Write();
							writer->Delete();
					}
					else
					{
						if(fileName.endsWith(".stl")==false) fileName+=".stl";
						vtkSTLWriter *writer=vtkSTLWriter ::New();
							writer->SetInput(surface->GetVtkPolyData());
							writer->SetFileName(fileName.ascii());
							writer->Write();
							writer->Delete();
					}
				}
			}
		}
    }

}

void QmitkMainTemplate::filePrint()
{

}

void QmitkMainTemplate::fileExit()
{
  qApp->quit();
}

void QmitkMainTemplate::editUndo()
{
    undoController->Undo();
}

void QmitkMainTemplate::editRedo()
{
    undoController->Redo();
}

void QmitkMainTemplate::editCut()
{
    //TEST!!!
    undoController->Undo(false);
}

void QmitkMainTemplate::editCopy()
{
    //TEST!!!!
    undoController->Redo(false);
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

    //initialize interaction sub-system: undo-controller, statemachine-factory and global-interaction
    //create undo-controller
    undoController = new mitk::UndoController;

	//create statemachine-factory:
	mitk::StateMachineFactory* stateMachineFactory = new mitk::StateMachineFactory();
	bool smLoadOK = stateMachineFactory->LoadBehavior("../../Framework/SceneInteraction/PointStateMachine.xml");

    //could the behavior file be found?
    if(smLoadOK)
    {
      	//create eventmapper-factory:
    	mitk::EventMapper* eventMapper = new mitk::EventMapper();
    	//Load all possible Events from List
    	bool eventLoadOK = eventMapper->LoadBehavior("../../Framework/SceneInteraction/PointStateMachine.xml");
        //could the behavior file be found?
        if(eventLoadOK)
        {
	        //set up the global StateMachine and register it to EventMapper
	        mitk::GlobalInteraction* globalInteraction = new mitk::GlobalInteraction("global");
        	mitk::EventMapper::SetGlobalStateMachine(globalInteraction);
        }
    }
    else
    {
        std::cout<<"Couldn't find XML-configure-file! Check your branch!"<<std::endl;
    }

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


		// add the diplayed planes of the multiwidget to a node to which the subtree @a planesSubTree points ...
        mitk::DataTreeIterator* it=tree->inorderIterator();
		mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
		it->add(node);
		it->next();
		planesSubTree=dynamic_cast<mitk::DataTreeBase*>(it->getSubTree());
		
        float white[3] = {1.0f,1.0f,1.0f};
        mitk::DataTreeNode::Pointer planeNode;
		// ... of widget 1
        planeNode=mitkMultiWidget->mitkWidget1->GetRenderer()->GetWorldGeometryNode();
        planeNode->SetColor(white, mitkMultiWidget->mitkWidget4->GetRenderer());
		it->add(planeNode);
		// ... of widget 2
        planeNode=mitkMultiWidget->mitkWidget2->GetRenderer()->GetWorldGeometryNode();
        planeNode->SetColor(white, mitkMultiWidget->mitkWidget4->GetRenderer());
		it->add(planeNode);
		// ... of widget 3
        planeNode=mitkMultiWidget->mitkWidget3->GetRenderer()->GetWorldGeometryNode();
        planeNode->SetColor(white, mitkMultiWidget->mitkWidget4->GetRenderer());
        it->add(planeNode);

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
    QmitkControlsRightFctLayoutTemplate* layoutTemplate=new QmitkControlsRightFctLayoutTemplate(MainWidget, "LayoutTemplate");
    hlayout->addWidget(layoutTemplate);

    //let the QmitkFctMediator know about the layout. This includes the toolbar and the layoutTemplate.
    qfm->initialize(this);
}

// FIXME: probably obsolete
// mitk::DataTree* QmitkMainTemplate::getDataTree()
// {
//     return tree;
//}


void QmitkMainTemplate::initWidgets( mitk::DataTreeIterator * it )
{

	mitk::DataTreeIterator *git = planesSubTree->inorderIterator();
	while(git->hasNext())
	{
		git->next();
		if(dynamic_cast<mitk::Geometry2DData*>(git->get()->GetData())!=NULL)
		{
			mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper;
			if(git->get()->GetMapper(2)==NULL)
			{
				geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
				it->get()->SetMapper(2, geometryMapper);
			}
			else
				geometryMapper = dynamic_cast<mitk::Geometry2DDataVtkMapper3D*>(git->get()->GetMapper(2));
			if(geometryMapper!=NULL)
				geometryMapper->SetDataIteratorForTexture(it);
		}
	}
	
	const mitk::BoundingBox::BoundsArrayType bounds = mitk::DataTree::ComputeBoundingBox(it)->GetBounds();
	printf("\nboundingbox\n");   

/*            initWidget(it,
                mitkMultiWidget->mitkWidget1,
                Vector3f(bounds[0],bounds[2],bounds[4]),
                Vector3f(bounds[1],bounds[2],bounds[4]),
                Vector3f(bounds[0],bounds[3],bounds[4])
                );*/
            initWidget(it,
                mitkMultiWidget->mitkWidget1,
                Vector3f(bounds[0],bounds[3],bounds[5]),
                Vector3f(bounds[1],bounds[3],bounds[5]),
                Vector3f(bounds[0],bounds[2],bounds[5])
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
//    if(qApp->argc()>1)
        fileOpen(qApp->argv()[i]);
    }
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
mitk::DataTree::Pointer QmitkMainTemplate::GetTree() {
    return tree;   
}
