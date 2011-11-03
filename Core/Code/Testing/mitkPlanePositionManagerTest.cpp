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

//TODO: Test GetInstance() / GetService

std::vector<mitk::PlaneGeometry::Pointer> m_Geometries;
std::vector<unsigned int> m_SliceIndices;


void SetUpBeforeTest()
{
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

//        itk::AffineTransform<mitk::ScalarType,3>::Pointer transform;

//        mitk::Transform3D


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

void TearDownAfterTest()
{

}

int testAddPlanePosition()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### A d d P l a n e P o s i t i o n #########");

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance() != NULL, "Testing GetInstance() of PlanePositionManager");

    mitk::PlaneGeometry::Pointer temp = m_Geometries.at(0);
    MITK_INFO<<"DEBUG: "<<temp->GetIndexToWorldTransform()->GetMatrix();

    unsigned int currentID(mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(m_Geometries.at(0),0));

    bool error = ((mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() != 1)||(currentID != 0));

    if(error)
    {
        MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == 1,"Checking for correct number of planepositions");
        MITK_TEST_CONDITION(currentID == 0, "Testing for correct ID");
        return EXIT_FAILURE;
    }

    for(unsigned int i = 1; i < m_Geometries.size(); ++i)
    {
        unsigned int newID = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(m_Geometries.at(i),i);
        error = ((mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() != i+1)||(newID != (currentID+1)));

        if (error)
        {
            MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == i+1,"Checking for correct number of planepositions");
            MITK_TEST_CONDITION(newID == (currentID+1), "Testing for correct ID");
            MITK_TEST_OUTPUT(<<"New: "<<newID<<" Last: "<<currentID);
            return EXIT_FAILURE;
        }
        currentID = newID;
    }

    unsigned int numberOfPlanePos = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();

    for(unsigned int i = 0; i < (m_Geometries.size()-1)*0.5; ++i)
    {
        unsigned int newID = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(m_Geometries.at(i*2),i*2);
        error = ((mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() != numberOfPlanePos)||(newID != i*2));
        if (error)
        {
            MITK_TEST_CONDITION( mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == numberOfPlanePos, "Checking for correct number of planepositions");
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
        op = mitk::PlanePositionManager::GetInstance()->GetPlanePosition(i);
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
    error = ( mitk::PlanePositionManager::GetInstance()->GetPlanePosition(100000000) != 0 ||
              mitk::PlanePositionManager::GetInstance()->GetPlanePosition(-1) != 0 );

    if (error)
    {
        MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(100000000) == 0, "Trying to get non existing pos");
        MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(-1) == 0, "Trying to get non existing pos");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;


//    MITK_INFO<<a<<" "<<i;
//    MITK_TEST_OUTPUT(<<"Starting Test: ######### G e t P l a n e P o s i t i o n #########");
//    unsigned int numPos = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
//    MITK_TEST_OUTPUT(<<"Current number of Positions: "<<numPos);

//    mitk::PlaneGeometry::Pointer pos = mitk::PlaneGeometry::New();
//    //width, height, vnl right/down, spacing
//    mitk::ScalarType width = 512;
//    mitk::ScalarType height = 512;
//    mitk::Vector3D right;
//    mitk::Vector3D down;

//    right[0] = 0.32;
//    right[1] = 0.24;
//    right[2] = 0.0;

//    down[0] = 0.0;
//    down[1] = 0.85;
//    down[2] = 1.0;

//    mitk::Vector3D spacing;
//    mitk::FillVector3D(spacing, 0.746, 1.235, 0.23);

//    pos->InitializeStandardPlane(width, height, right.Get_vnl_vector(), down.Get_vnl_vector(), &spacing);

//    unsigned int id = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(pos, 20);

//    mitk::RestorePlanePositionOperation* op = mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id);

//    MITK_TEST_CONDITION(op != 0,"Testing received Pos is not null");

//    MITK_TEST_CONDITION(op->GetHeight() == height, "Testing for correct height");
//    MITK_TEST_CONDITION(op->GetWidth() == width, "Testing for correct width");

//    MITK_TEST_CONDITION(((op->GetSpacing()[0]-spacing[0]) < 0.00001 && (op->GetSpacing()[1]-spacing[1]) < 0.00001 &&
//                         (op->GetSpacing()[2]-spacing[2]) < 0.00001), "Testing for correct spacing");

//    mitk::AffineTransform3D* tr1 = pos->GetIndexToWorldTransform();
//    mitk::AffineTransform3D* tr2 = op->GetTransform();
//    bool sameMatrix(true);

//    for (unsigned int i = 0; i < 3; i++)
//    {
//        for (unsigned int j = 0; j < 3; j++)
//        {
//            if(tr1->GetMatrix()[i][j]-tr2->GetMatrix()[i][j] > 0.00001)
//            {
//                sameMatrix=false;
//                break;
//            }
//        }
//    }

//    mitk::Vector3D offsetDiff = tr1->GetOffset() - tr2->GetOffset();
//    bool sameOffset(offsetDiff[0] < 0.00001 && offsetDiff[1] < 0.00001 && offsetDiff[2] < 0.00001);

//    MITK_TEST_CONDITION((sameMatrix && sameOffset), "Testing for correct transform");

//    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(100000000) == 0, "Trying to get non existing pos");

}

int testRemovePlanePosition()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### R e m o v e P l a n e P o s i t i o n #########");
    unsigned int size = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();

    bool removed (true);
    removed = mitk::PlanePositionManager::GetInstance()->RemovePlanePosition( -1 );
    removed = mitk::PlanePositionManager::GetInstance()->RemovePlanePosition( 1000000 );
    unsigned int size2 = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();

    if (removed)
    {
        MITK_TEST_CONDITION(removed == false, "Testing remove not existing planepositions");
        MITK_TEST_CONDITION(size == size2, "Testing remove not existing planepositions");
        return EXIT_FAILURE;
    }

    for (unsigned int i = 0; i < m_Geometries.size()*0.5; i++)
    {
        removed = mitk::PlanePositionManager::GetInstance()->RemovePlanePosition( i );
        unsigned int size2 = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
        removed = (size2 == (size-(i+1)));
        if (!removed)
        {
            MITK_TEST_CONDITION(removed == true, "Testing remove existing planepositions");
            MITK_TEST_CONDITION(size == (size-i+1), "Testing remove existing planepositions");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
//    MITK_TEST_OUTPUT(<<"Testing RemovePlanePosition - Current number of Position: "<<numPos);

//    bool removed = mitk::PlanePositionManager::GetInstance()->RemovePlanePosition( 0 );
//    bool stillExists (mitk::PlanePositionManager::GetInstance()->GetPlanePosition(0));

//    MITK_TEST_CONDITION((removed && !stillExists), "Testing remove planepos");
}

int testRemoveAll()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### R e m o v e A l l #########");

    unsigned int numPos = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
    MITK_INFO<<numPos;

    mitk::PlanePositionManager::GetInstance()->RemoveAllPlanePositions();

    bool error (true);

    error = (mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() != 0 ||
             mitk::PlanePositionManager::GetInstance()->GetPlanePosition(60) != 0);

    if (error)
    {
        MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == 0, "Testing remove all pos");
        MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(60) == 0, "Testing remove all pos");
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


//void testRestorePlanePosition()
//{
//    //Set up a Geometry3D
//    mitk::Geometry3D::Pointer geometry3d = mitk::Geometry3D::New();
//    float bounds[ ] = {-10.0, 17.0, -12.0, 188.0, 13.0, 211.0};
//    geometry3d->Initialize();
//    geometry3d->SetFloatBounds(bounds);

//    mitk::AffineTransform3D::MatrixType matrix;
//    matrix.SetIdentity();
//    matrix(1,1) = 2;
//    mitk::AffineTransform3D::Pointer transform;
//    transform = mitk::AffineTransform3D::New();
//    transform->SetMatrix(matrix);
//    geometry3d->SetIndexToWorldTransform(transform);

//    //CreateSNC
//    mitk::SliceNavigationController::Pointer sliceCtrl = mitk::SliceNavigationController::New();
//    sliceCtrl->SetInputWorldGeometry(geometry3d);
//    sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Original);

//    //Rotate Geometry3D
//    double angle = 35.0;
//    mitk::Vector3D rotationVector; mitk::FillVector3D( rotationVector, 1, 0, 0 );
//    mitk::Point3D center = geometry3d->GetCenter();
//    mitk::RotationOperation* op = new mitk::RotationOperation( mitk::OpROTATE, center, rotationVector, angle );
//    geometry3d->ExecuteOperation(op);

//    sliceCtrl->Update();

//    //Save PlanePositon
//    const mitk::Geometry2D* plane = dynamic_cast<const mitk::Geometry2D*> (dynamic_cast< const mitk::SlicedGeometry3D*>(
//      sliceCtrl->GetCurrentGeometry3D())->GetGeometry2D(0));

//    unsigned int size = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
//    unsigned int id = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(plane, sliceCtrl->GetSlice()->GetPos());
//    //Reinit Geometry3d

//    // Restore the initial plane pose by undoing the previous rotation
//    // operation @Swiveller
//    mitk::RotationOperation* opReset = new mitk::RotationOperation( mitk::OpROTATE, center,
//      rotationVector, -angle );
//    geometry3d->ExecuteOperation(opReset);

//    sliceCtrl->Update();

//    //Restore PlanePosition
//    mitk::RestorePlanePositionOperation* restoreOp = mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id);
//    sliceCtrl->ExecuteOperation(restoreOp);
//    sliceCtrl->Update();

//    MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(sliceCtrl->GetCreatedWorldGeometry()->GetIndexToWorldTransform()->GetMatrix(), restoreOp->GetTransform()->GetMatrix()),"Testing for correct restoration of geometry");
//    MITK_TEST_OUTPUT(<<"RestoredMatrix: "<<sliceCtrl->GetCreatedWorldGeometry()->GetIndexToWorldTransform()->GetMatrix()<<"  OriginalMatrix: "<<restoreOp->GetTransform()->GetMatrix());
//    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetIndexToWorldTransform()->GetOffset(), restoreOp->GetTransform()->GetOffset()), "Testing for correct restoration of geometry");
//    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetSpacing(), restoreOp->GetSpacing()), "Testing for correct restoration of geometry");
//    //MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetEx, restoreOp->GetSpacing()), "Testing for correct restoration of geometry");
//    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetSpacing(), restoreOp->GetSpacing()), "Testing for correct restoration of geometry");
//    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetSlice()->GetPos(), restoreOp->GetPos()), "Testing for correct restoration of geometry");


//}

//void testDataStorageNodeRemoved()
//{
//    MITK_TEST_OUTPUT(<<"Starting Test: ######### D a t a S t o r a g e N o d e R e m o v e d #########");

//    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

//    mitk::PlanePositionManager::GetInstance()->SetDataStorage(ds);

//    mitk::PlaneGeometry::Pointer pos = mitk::PlaneGeometry::New();
//    //width, height, vnl right/down, spacing
//    mitk::ScalarType width = 1024;
//    mitk::ScalarType height = 1024;
//    mitk::Vector3D right;
//    mitk::Vector3D down;

//    right[0] = 0.3723;
//    right[1] = 0.938;
//    right[2] = 1.0;

//    down[0] = 1.0;
//    down[1] = 0.57;
//    down[2] = 1.0;

//    mitk::Vector3D spacing;
//    mitk::FillVector3D(spacing, 0.5674, 1.0, 1.0);

//    pos->InitializeStandardPlane(width, height, right.Get_vnl_vector(), down.Get_vnl_vector(), &spacing);

//    unsigned int id = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(pos, 20);

//    std::stringstream markerStream;
//    markerStream << "Position";
//    markerStream << " ";
//    markerStream << id+1;

//    mitk::DataNode::Pointer rotatedContourNode = mitk::DataNode::New();

//    //rotatedContourNode->SetData(contourMarker);
//    rotatedContourNode->SetProperty( "name", mitk::StringProperty::New(markerStream.str()) );
//    rotatedContourNode->SetProperty( "isContourMarker", mitk::BoolProperty::New(true));
//    ds->Add(rotatedContourNode);

//    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id) != 0, "Testing DataStorage Node removed");

//    ds->Remove(rotatedContourNode);

//    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id) == 0, "Testing DataStorage Node removed");
//}
