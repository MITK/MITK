#include "mitkRotationOperation.h"
#include "mitkTestingMacros.h"
#include "mitkPlanePositionManager.h"
#include "mitkSliceNavigationController.h"
#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkImage.h"
#include "mitkSurface.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkDataNode.h"
#include "mitkStringProperty.h"
#include "mitkBaseProperty.h"
#include "mitkInteractionConst.h"
#include "vnl/vnl_vector.h"
#include <itkAffineGeometryFrame.h>
#include "mitkGetModuleContext.h"

std::vector<mitk::PlaneGeometry::Pointer> m_Geometries;
std::vector<unsigned int> m_SliceIndices;

mitk::PlanePositionManagerService* m_Service;


int SetUpBeforeTest()
{
    //Getting Service
    mitk::ServiceReference serviceRef = mitk::GetModuleContext()->GetServiceReference<mitk::PlanePositionManagerService>();
    m_Service = dynamic_cast<mitk::PlanePositionManagerService*>(mitk::GetModuleContext()->GetService(serviceRef));

    if (m_Service == 0)
        return EXIT_FAILURE;

    //Creating different Geometries
    m_Geometries.reserve(100);
    mitk::PlaneGeometry::PlaneOrientation views[] = {mitk::PlaneGeometry::Transversal, mitk::PlaneGeometry::Sagittal, mitk::PlaneGeometry::Frontal};
    for (unsigned int i = 0; i < 100; ++i)
    {
        mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();

        mitk::ScalarType width = 256+(0.01*i);
        mitk::ScalarType height = 256+(0.002*i);

        mitk::Vector3D right;
        mitk::Vector3D down;
        right[0] = 1;
        right[1] = i;
        right[2] = 0.5;
        down[0] = i*0.02;
        down[1] = 1;
        down[2] = i*0.03;

        mitk::Vector3D spacing;
        mitk::FillVector3D(spacing, 1.0*0.02*i, 1.0*0.15*i, 1.0);

        mitk::Vector3D rightVector;
        mitk::FillVector3D(rightVector, 0.02*(i+1), 0+(0.05*i), 1.0);

        mitk::Vector3D downVector;
        mitk::FillVector3D(downVector, 1, 3-0.01*i, 0.0345*i);

        vnl_vector<mitk::ScalarType> normal = vnl_cross_3d(rightVector.GetVnlVector(), downVector.GetVnlVector());
        normal.normalize();
        normal *= 1.5;

        mitk::Vector3D origin;
        origin.Fill(1);
        origin[0] = 12 + 0.03*i;


        mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
        mitk::Matrix3D matrix;
        matrix.GetVnlMatrix().set_column(0, rightVector.GetVnlVector());
        matrix.GetVnlMatrix().set_column(1, downVector.GetVnlVector());
        matrix.GetVnlMatrix().set_column(2, normal);
        transform->SetMatrix(matrix);
        transform->SetOffset(origin);


        plane->InitializeStandardPlane(width, height, transform,
                                       views[i%3], i,
                                       true, false);

        m_Geometries.push_back(plane);
    }
}

int testAddPlanePosition()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### A d d P l a n e P o s i t i o n #########");

    MITK_TEST_CONDITION(m_Service != NULL, "Testing getting of PlanePositionManagerService");

    unsigned int currentID(m_Service->AddNewPlanePosition(m_Geometries.at(0),0));

    bool error = ((m_Service->GetNumberOfPlanePositions() != 1)||(currentID != 0));

    if(error)
    {
        MITK_TEST_CONDITION(m_Service->GetNumberOfPlanePositions() == 1,"Checking for correct number of planepositions");
        MITK_TEST_CONDITION(currentID == 0, "Testing for correct ID");
        return EXIT_FAILURE;
    }

    //Adding new planes
    for(unsigned int i = 1; i < m_Geometries.size(); ++i)
    {
        unsigned int newID = m_Service->AddNewPlanePosition(m_Geometries.at(i),i);
        error = ((m_Service->GetNumberOfPlanePositions() != i+1)||(newID != (currentID+1)));

        if (error)
        {
            MITK_TEST_CONDITION(m_Service->GetNumberOfPlanePositions() == i+1,"Checking for correct number of planepositions");
            MITK_TEST_CONDITION(newID == (currentID+1), "Testing for correct ID");
            MITK_TEST_OUTPUT(<<"New: "<<newID<<" Last: "<<currentID);
            return EXIT_FAILURE;
        }
        currentID = newID;
    }

    unsigned int numberOfPlanePos = m_Service->GetNumberOfPlanePositions();

    //Adding existing planes -> nothing should change
    for(unsigned int i = 0; i < (m_Geometries.size()-1)*0.5; ++i)
    {
        unsigned int newID = m_Service->AddNewPlanePosition(m_Geometries.at(i*2),i*2);
        error = ((m_Service->GetNumberOfPlanePositions() != numberOfPlanePos)||(newID != i*2));
        if (error)
        {
            MITK_TEST_CONDITION( m_Service->GetNumberOfPlanePositions() == numberOfPlanePos, "Checking for correct number of planepositions");
            MITK_TEST_CONDITION(newID == i*2, "Testing for correct ID");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;

}

int testGetPlanePosition()
{
    mitk::PlaneGeometry* plane;
    mitk::RestorePlanePositionOperation* op;
    bool error(true);

    MITK_TEST_OUTPUT(<<"Starting Test: ######### G e t P l a n e P o s i t i o n #########");

    //Testing for existing planepositions
    for (unsigned int i = 0; i < m_Geometries.size(); ++i)
    {
        plane = m_Geometries.at(i);
        mitk::PlaneGeometry* test = m_Geometries.at(i);
        op = m_Service->GetPlanePosition(i);
        error = ( !mitk::Equal(op->GetHeight(),plane->GetExtent(1)) ||
                  !mitk::Equal(op->GetWidth(),plane->GetExtent(0)) ||
                  !mitk::Equal(op->GetSpacing(),plane->GetSpacing()) ||
                  !mitk::Equal(op->GetTransform()->GetOffset(),plane->GetIndexToWorldTransform()->GetOffset()) ||
                  !mitk::Equal(op->GetDirectionVector().Get_vnl_vector(),plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2).normalize()) ||
                  !mitk::MatrixEqualElementWise(op->GetTransform()->GetMatrix(), plane->GetIndexToWorldTransform()->GetMatrix()) );

        if( error )
        {
            MITK_TEST_OUTPUT(<<"Iteration: "<<i)
            MITK_TEST_CONDITION( mitk::Equal(op->GetHeight(),plane->GetExtent(1)) && mitk::Equal(op->GetWidth(),plane->GetExtent(0)), "Checking for correct extent");
            MITK_TEST_CONDITION( mitk::Equal(op->GetSpacing(),plane->GetSpacing()), "Checking for correct spacing");
            MITK_TEST_CONDITION( mitk::Equal(op->GetTransform()->GetOffset(),plane->GetIndexToWorldTransform()->GetOffset()), "Checking for correct offset");
            MITK_INFO<<"Op: "<<op->GetDirectionVector()<<" plane: "<<plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)<<"\n";
            MITK_TEST_CONDITION( mitk::Equal(op->GetDirectionVector().Get_vnl_vector(),plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)), "Checking for correct direction");
            MITK_TEST_CONDITION( mitk::MatrixEqualElementWise(op->GetTransform()->GetMatrix(), plane->GetIndexToWorldTransform()->GetMatrix()), "Checking for correct matrix");
            return EXIT_FAILURE;
        }
    }

    //Testing for not existing planepositions
    error = ( m_Service->GetPlanePosition(100000000) != 0 ||
              m_Service->GetPlanePosition(-1) != 0 );

    if (error)
    {
        MITK_TEST_CONDITION(m_Service->GetPlanePosition(100000000) == 0, "Trying to get non existing pos");
        MITK_TEST_CONDITION(m_Service->GetPlanePosition(-1) == 0, "Trying to get non existing pos");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int testRemovePlanePosition()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### R e m o v e P l a n e P o s i t i o n #########");
    unsigned int size = m_Service->GetNumberOfPlanePositions();

    bool removed (true);
    //Testing for invalid IDs
    removed = m_Service->RemovePlanePosition( -1 );
    removed = m_Service->RemovePlanePosition( 1000000 );
    unsigned int size2 = m_Service->GetNumberOfPlanePositions();

    if (removed)
    {
        MITK_TEST_CONDITION(removed == false, "Testing remove not existing planepositions");
        MITK_TEST_CONDITION(size == size2, "Testing remove not existing planepositions");
        return EXIT_FAILURE;
    }

    //Testing for valid IDs
    for (unsigned int i = 0; i < m_Geometries.size()*0.5; i++)
    {
        removed = m_Service->RemovePlanePosition( i );
        unsigned int size2 = m_Service->GetNumberOfPlanePositions();
        removed = (size2 == (size-(i+1)));
        if (!removed)
        {
            MITK_TEST_CONDITION(removed == true, "Testing remove existing planepositions");
            MITK_TEST_CONDITION(size == (size-i+1), "Testing remove existing planepositions");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int testRemoveAll()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### R e m o v e A l l #########");

    unsigned int numPos = m_Service->GetNumberOfPlanePositions();
    MITK_INFO<<numPos;

    m_Service->RemoveAllPlanePositions();

    bool error (true);

    error = (m_Service->GetNumberOfPlanePositions() != 0 ||
             m_Service->GetPlanePosition(60) != 0);

    if (error)
    {
        MITK_TEST_CONDITION(m_Service->GetNumberOfPlanePositions() == 0, "Testing remove all pos");
        MITK_TEST_CONDITION(m_Service->GetPlanePosition(60) == 0, "Testing remove all pos");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int mitkPlanePositionManagerTest(int argc, char* argv[])
{
    MITK_TEST_OUTPUT(<<"Starting Test PlanePositionManager");
    SetUpBeforeTest();
    int result;
    MITK_TEST_CONDITION_REQUIRED( (result = testAddPlanePosition()) == EXIT_SUCCESS, "");
    MITK_TEST_CONDITION_REQUIRED( (result = testGetPlanePosition()) == EXIT_SUCCESS, "");
    MITK_TEST_CONDITION_REQUIRED( (result = testRemovePlanePosition()) == EXIT_SUCCESS, "");
    MITK_TEST_CONDITION_REQUIRED( (result = testRemoveAll()) == EXIT_SUCCESS, "");
    return EXIT_SUCCESS;
}
