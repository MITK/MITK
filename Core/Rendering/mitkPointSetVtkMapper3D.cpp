#include "mitkPointSetVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkBoolProperty.h"
#include "mitkFloatProperty.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"
#include "mitkOpenGLRenderer.h"


#include <vtkActor.h>
#include <vtkFollower.h>
#include <vtkAssembly.h>
#include <vtkProp3DCollection.h>
#include <vtkRenderer.h>


#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <stdlib.h>

//##ModelId=3E70F60301D5
const mitk::PointSet* mitk::PointSetVtkMapper3D::GetInput()
{
    if (this->GetNumberOfInputs() < 1)
    {
        return 0;
    }

    return static_cast<const mitk::PointSet * > ( GetData() );
}

//##ModelId=3E70F60301E9
void mitk::PointSetVtkMapper3D::GenerateData()
{
}

//##ModelId=3E70F60301F2
void mitk::PointSetVtkMapper3D::GenerateOutputInformation()
{
}

//##ModelId=3E70F60301F4
mitk::PointSetVtkMapper3D::PointSetVtkMapper3D()
{
    m_VtkPolyDataMapper = vtkPolyDataMapper::New();
//    m_PointVtkPolyDataMapper = vtkPolyDataMapper::New();
//    m_TextVtkPolyDataMapper = vtkPolyDataMapper::New();
        
//    m_PointActor = vtkActor::New();
//    m_PointActor->SetMapper(m_PointVtkPolyDataMapper);
        
//    m_TextActor = vtkFollower::New();
//    m_TextActor = vtkActor::New();
//    m_TextActor->SetMapper(m_TextVtkPolyDataMapper);

//    m_Actor = vtkAssembly::New();
    m_Actor = vtkActor::New();
    m_Actor->SetMapper(m_VtkPolyDataMapper);
   
		m_Prop3D = m_Actor;
		m_Prop3D->Register(NULL);


		m_vtkPointList = vtkAppendPolyData::New();
		m_vtkTextList = vtkAppendPolyData::New();
		m_contour = vtkPolyData::New();
	  m_tubefilter = vtkTubeFilter::New();


}

//##ModelId=3E70F60301F5
mitk::PointSetVtkMapper3D::~PointSetVtkMapper3D()
{
    m_VtkPolyDataMapper->Delete();
    m_Actor->Delete();

//	  if (m_Actor != NULL)
//    {
//        vtkProp* prop;
//        vtkProp3DCollection* pc = m_Actor->GetParts();
//        pc->InitTraversal();
//        while ( ( prop = pc->GetNextProp() ) )
//        {
//            if (prop != NULL)
//            {
//               prop->Delete();
//            }
//        }
//        m_Actor->Delete();
//    }
//    m_PointActor->Delete();
}

//##ModelId=3EF19FA803BF
void mitk::PointSetVtkMapper3D::Update(mitk::BaseRenderer* renderer)
{
    if(IsVisible(renderer)==false)
    {
        m_Actor->VisibilityOff();
        return;
    }
    m_Actor->VisibilityOn();

		m_vtkPointList->Delete();
		m_vtkTextList->Delete();
		m_contour->Delete();
	  m_tubefilter->Delete();

		m_vtkPointList = vtkAppendPolyData::New();
		m_vtkTextList = vtkAppendPolyData::New();
		m_contour = vtkPolyData::New();
	  m_tubefilter = vtkTubeFilter::New();


    mitk::PointSet::Pointer input  = const_cast<mitk::PointSet*>(this->GetInput());
    mitk::PointSet::PointSetType::Pointer pointList;

		pointList = input->GetPointSet();

    mitk::PointSet::PointsContainer::Iterator i;

		int j;
//		vtkAppendPolyData *m_vtkPointList = vtkAppendPolyData::New();
//		vtkAppendPolyData *m_vtkTextList = vtkAppendPolyData::New();

		bool makeContour;
		if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("contour").GetPointer()) == NULL)
			makeContour = false;
		else
			makeContour = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("contour").GetPointer())->GetBool();

		if (makeContour)
		{

//			vtkPolyData *m_contour = vtkPolyData::New();
		  vtkPoints *points = vtkPoints::New();
		  vtkCellArray *polys = vtkCellArray::New();

      for (j=0, i=pointList->GetPoints()->Begin(); i!=pointList->GetPoints()->End() ; i++,j++)
			{
				int cell[2] = {j-1,j};
        points->InsertPoint(j,i.Value()[0],i.Value()[1],i.Value()[2]);
				if (j>0)
					polys->InsertNextCell(2,cell);
			}


			bool close;
			if (dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("close").GetPointer()) == NULL)
				close = false;
			else
				close = dynamic_cast<mitk::BoolProperty *>(this->GetDataTreeNode()->GetProperty("close").GetPointer())->GetBool();

			if (close) {
				int cell[2] = {j-1,0};
				polys->InsertNextCell(2,cell);
			}

			m_contour->SetPoints(points);
		  points->Delete();
  		m_contour->SetLines(polys);
		  polys->Delete();
			m_contour->Update();


//	   vtkm_tubefilter * m_tubefilter = vtkm_tubefilter::New();
		  m_tubefilter->SetInput(m_contour);
		  m_tubefilter->SetRadius(1);
	//	  m_tubefilter->SetRadiusFactor(7);
	//	  m_tubefilter->SetNumberOfSides(6);
			m_tubefilter->Update();;

		m_vtkPointList->AddInput(m_tubefilter->GetOutput());
	}

  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  GetColor(rgba, renderer);


for (j=0, i=pointList->GetPoints()->Begin(); i!=pointList->GetPoints()->End() ; i++,j++)
	{
	    vtkSphereSource *sphere = vtkSphereSource::New();

			sphere->SetRadius(2);
      sphere->SetCenter(i.Value()[0],i.Value()[1],i.Value()[2]);

			m_vtkPointList->AddInput(sphere->GetOutput());

			if (dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer()) == NULL)
			{}
			else {

				const char * pointLabel =dynamic_cast<mitk::StringProperty *>(this->GetDataTreeNode()->GetProperty("label").GetPointer())->GetString();
				char buffer[20];
 	      std::string l = pointLabel;
				if (input->GetSize()>1)
				{
					sprintf(buffer,"%d",j+1);
					l.append(buffer);
				}

				////// Define the text for the label
					vtkVectorText *label = vtkVectorText::New();
				  label->SetText(l.c_str());

//				// Define the text for the label
//				vtkTextSource *label = vtkTextSource::New();
//			  label->SetText(l.c_str());
//				label->BackingOff();
//				label->SetForegroundColor(rgba[0],rgba[1],rgba[2]);

				//# Set up a transform to move the label to a new position.
				vtkTransform *aLabelTransform =vtkTransform::New();
				aLabelTransform->Identity();
			  aLabelTransform->Translate(i.Value()[0]+2,i.Value()[1]+2,i.Value()[2]);
			  aLabelTransform->Scale(5.7,5.7,5.7);

				//# Move the label to a new position.
				vtkTransformPolyDataFilter *labelTransform = vtkTransformPolyDataFilter::New();
			  labelTransform->SetTransform(aLabelTransform);
			  labelTransform->SetInput(label->GetOutput());

//	      m_TextActor = vtkFollower::New();
//		    m_TextVtkPolyDataMapper = vtkPolyDataMapper::New();
//
////	    	m_TextVtkPolyDataMapper->SetInput(labelTransform->GetOutput());
//	    	m_TextVtkPolyDataMapper->SetInput(label->GetOutput());
//		    m_TextActor->SetMapper(m_TextVtkPolyDataMapper);
//				m_TextActor->SetPosition((*i)[0]+2,(*i)[1]+2,(*i)[2]);
//				m_TextActor->SetScale(10,10,10);
//
////				m_vtkTextList->AddInput(labelTransform->GetOutput());
//		    m_TextActor->GetProperty()->SetColor(rgba);
//				m_TextActor->SetCamera(  dynamic_cast<mitk::OpenGLRenderer *>(renderer)->GetVtkRenderer()->GetActiveCamera());
//		    m_Actor->AddPart(m_TextActor);

					m_vtkPointList->AddInput(labelTransform->GetOutput());
			}


		}

//		m_TextActor->SetCamera(  dynamic_cast<mitk::OpenGLRenderer *>(renderer)->GetVtkRenderer()->GetActiveCamera());

//    m_PointVtkPolyDataMapper->SetInput(m_vtkPointList->GetOutput());

    m_VtkPolyDataMapper->SetInput(m_vtkPointList->GetOutput());
    m_Actor->GetProperty()->SetColor(rgba);
//    m_PointActor->GetProperty()->SetColor(rgba);
//    m_TextActor->GetProperty()->SetColor(rgba);

//    m_Actor->AddPart(m_PointActor);
//    m_Actor->AddPart(m_TextActor);			

    StandardUpdate();
}

//##ModelId=3EF1B44001D5
void mitk::PointSetVtkMapper3D::Update()
{
}

