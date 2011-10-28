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

//#include "mitkExtractImageFilter.h"
//#include "mitkExtractDirectedPlaneImageFilterNew.h"


//TODO: Test GetInstance() / GetService

//SNCs of the image/surface
mitk::SliceNavigationController* sncTransversal;
mitk::SliceNavigationController* sncSagittal;
mitk::SliceNavigationController* sncCoronal;

mitk::Image* testImage;
mitk::Surface* testSurface;

void SetUpBeforeTest()
{
    //Creating different

}

void TearDownAfterTest()
{

}

void testAddPlanePosition()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### A d d P l a n e P o s i t i o n #########");

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance() != NULL, "Testing GetInstance() of PlanePositionManager");

    //Initializing StandardPlanes
    mitk::PlaneGeometry::Pointer pos1 = mitk::PlaneGeometry::New();

    mitk::ScalarType width = 256;
    mitk::ScalarType height = 256;

    mitk::Vector3D right;
    mitk::Vector3D down;
    right[0] = 1;
    right[1] = 0;
    right[2] = 0;
    down[0] = 0;
    down[1] = 1;
    down[2] = 0;

    mitk::Vector3D spacing;
    mitk::FillVector3D(spacing, 1.0, 1.0, 1.0);

    pos1->InitializeStandardPlane(width, height, right.Get_vnl_vector(), down.Get_vnl_vector(), &spacing);

    mitk::PlaneGeometry::Pointer pos2 = mitk::PlaneGeometry::New();
    //width, height, vnl right/down, spacing
    mitk::ScalarType width2 = 256;
    mitk::ScalarType height2 = 256;
    mitk::Vector3D right2;
    mitk::Vector3D down2;

    right2[0] = 0.75;
    right2[1] = 0.2;
    right2[2] = 0;

    down2[0] = 0.1;
    down2[1] = 1.1;
    down2[2] = 0.0;

    mitk::Vector3D spacing2;
    mitk::FillVector3D(spacing2, 1.0, 1.0, 1.0);

    pos2->InitializeStandardPlane(width2, height2, right2.Get_vnl_vector(), down2.Get_vnl_vector(), &spacing2);

    unsigned int id1 = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(pos1, 20);

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == 1,"Checking for correct number of planepositions");
    MITK_TEST_CONDITION(id1 == 0, "Testing for correct ID");

    //Adding the same Pos again->nothing should change
    unsigned int id2 = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(pos1, 20);

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == 1,"Checking for correct number of planepositions");
    MITK_TEST_CONDITION(id2 == id1, "Testing for correct ID");

    //Adding a new Pos -> NumberOfPos should increase
    unsigned int id3 = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(pos2, 20);

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == 2,"Checking for correct number of planepositions");
    MITK_TEST_CONDITION(id3 == 1, "Testing for correct ID");
    MITK_TEST_OUTPUT(<<id3<<"   ****   "<<mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions());

}

void testGetPlanePosition()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### G e t P l a n e P o s i t i o n #########");
    unsigned int numPos = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
    MITK_TEST_OUTPUT(<<"Current number of Positions: "<<numPos);

    mitk::PlaneGeometry::Pointer pos = mitk::PlaneGeometry::New();
    //width, height, vnl right/down, spacing
    mitk::ScalarType width = 512;
    mitk::ScalarType height = 512;
    mitk::Vector3D right;
    mitk::Vector3D down;

    right[0] = 0.32;
    right[1] = 0.24;
    right[2] = 0.0;

    down[0] = 0.0;
    down[1] = 0.85;
    down[2] = 1.0;

    mitk::Vector3D spacing;
    mitk::FillVector3D(spacing, 0.746, 1.235, 0.23);

    pos->InitializeStandardPlane(width, height, right.Get_vnl_vector(), down.Get_vnl_vector(), &spacing);

    unsigned int id = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(pos, 20);

    mitk::RestorePlanePositionOperation* op = mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id);

    MITK_TEST_CONDITION(op != 0,"Testing received Pos is not null");

    MITK_TEST_CONDITION(op->GetHeight() == height, "Testing for correct height");
    MITK_TEST_CONDITION(op->GetWidth() == width, "Testing for correct width");

    MITK_TEST_CONDITION(((op->GetSpacing()[0]-spacing[0]) < 0.00001 && (op->GetSpacing()[1]-spacing[1]) < 0.00001 &&
                         (op->GetSpacing()[2]-spacing[2]) < 0.00001), "Testing for correct spacing");

    mitk::AffineTransform3D* tr1 = pos->GetIndexToWorldTransform();
    mitk::AffineTransform3D* tr2 = op->GetTransform();
    bool sameMatrix(true);

    for (unsigned int i = 0; i < 3; i++)
    {
        for (unsigned int j = 0; j < 3; j++)
        {
            if(tr1->GetMatrix()[i][j]-tr2->GetMatrix()[i][j] > 0.00001)
            {
                sameMatrix=false;
                break;
            }
        }
    }

    mitk::Vector3D offsetDiff = tr1->GetOffset() - tr2->GetOffset();
    bool sameOffset(offsetDiff[0] < 0.00001 && offsetDiff[1] < 0.00001 && offsetDiff[2] < 0.00001);

    MITK_TEST_CONDITION((sameMatrix && sameOffset), "Testing for correct transform");

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(100000000) == 0, "Trying to get non existing pos");

}

void testRemovePlanePosition()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### R e m o v e P l a n e P o s i t i o n #########");
    unsigned int numPos = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
    MITK_TEST_OUTPUT(<<"Testing RemovePlanePosition - Current number of Position: "<<numPos);

    bool removed = mitk::PlanePositionManager::GetInstance()->RemovePlanePosition( 0 );
    bool stillExists (mitk::PlanePositionManager::GetInstance()->GetPlanePosition(0));

    MITK_TEST_CONDITION((removed && !stillExists), "Testing remove planepos");
}

void testRestorePlanePosition()
{
    //Set up a Geometry3D
    mitk::Geometry3D::Pointer geometry3d = mitk::Geometry3D::New();
    float bounds[ ] = {-10.0, 17.0, -12.0, 188.0, 13.0, 211.0};
    geometry3d->Initialize();
    geometry3d->SetFloatBounds(bounds);

    mitk::AffineTransform3D::MatrixType matrix;
    matrix.SetIdentity();
    matrix(1,1) = 2;
    mitk::AffineTransform3D::Pointer transform;
    transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(matrix);
    geometry3d->SetIndexToWorldTransform(transform);

    //CreateSNC
    mitk::SliceNavigationController::Pointer sliceCtrl = mitk::SliceNavigationController::New();
    sliceCtrl->SetInputWorldGeometry(geometry3d);
    sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Original);

    //Rotate Geometry3D
    double angle = 35.0;
    mitk::Vector3D rotationVector; mitk::FillVector3D( rotationVector, 1, 0, 0 );
    mitk::Point3D center = geometry3d->GetCenter();
    mitk::RotationOperation* op = new mitk::RotationOperation( mitk::OpROTATE, center, rotationVector, angle );
    geometry3d->ExecuteOperation(op);

    sliceCtrl->Update();

    //Save PlanePositon
    const mitk::Geometry2D* plane = dynamic_cast<const mitk::Geometry2D*> (dynamic_cast< const mitk::SlicedGeometry3D*>(
      sliceCtrl->GetCurrentGeometry3D())->GetGeometry2D(0));

    unsigned int size = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
    unsigned int id = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(plane, sliceCtrl->GetSlice()->GetPos());
    //Reinit Geometry3d

    // Restore the initial plane pose by undoing the previous rotation
    // operation @Swiveller
    mitk::RotationOperation* opReset = new mitk::RotationOperation( mitk::OpROTATE, center,
      rotationVector, -angle );
    geometry3d->ExecuteOperation(opReset);

    sliceCtrl->Update();

    //Restore PlanePosition
    mitk::RestorePlanePositionOperation* restoreOp = mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id);
    sliceCtrl->ExecuteOperation(restoreOp);
    sliceCtrl->Update();

    MITK_TEST_CONDITION(mitk::MatrixEqualElementWise(sliceCtrl->GetCreatedWorldGeometry()->GetIndexToWorldTransform()->GetMatrix(), restoreOp->GetTransform()->GetMatrix()),"Testing for correct restoration of geometry");
    MITK_TEST_OUTPUT(<<"RestoredMatrix: "<<sliceCtrl->GetCreatedWorldGeometry()->GetIndexToWorldTransform()->GetMatrix()<<"  OriginalMatrix: "<<restoreOp->GetTransform()->GetMatrix());
    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetIndexToWorldTransform()->GetOffset(), restoreOp->GetTransform()->GetOffset()), "Testing for correct restoration of geometry");
    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetSpacing(), restoreOp->GetSpacing()), "Testing for correct restoration of geometry");
    //MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetEx, restoreOp->GetSpacing()), "Testing for correct restoration of geometry");
    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetCreatedWorldGeometry()->GetSpacing(), restoreOp->GetSpacing()), "Testing for correct restoration of geometry");
    MITK_TEST_CONDITION(mitk::Equal(sliceCtrl->GetSlice()->GetPos(), restoreOp->GetPos()), "Testing for correct restoration of geometry");


}

void testDataStorageNodeRemoved()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### D a t a S t o r a g e N o d e R e m o v e d #########");

    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

    mitk::PlanePositionManager::GetInstance()->SetDataStorage(ds);

    mitk::PlaneGeometry::Pointer pos = mitk::PlaneGeometry::New();
    //width, height, vnl right/down, spacing
    mitk::ScalarType width = 1024;
    mitk::ScalarType height = 1024;
    mitk::Vector3D right;
    mitk::Vector3D down;

    right[0] = 0.3723;
    right[1] = 0.938;
    right[2] = 1.0;

    down[0] = 1.0;
    down[1] = 0.57;
    down[2] = 1.0;

    mitk::Vector3D spacing;
    mitk::FillVector3D(spacing, 0.5674, 1.0, 1.0);

    pos->InitializeStandardPlane(width, height, right.Get_vnl_vector(), down.Get_vnl_vector(), &spacing);

    unsigned int id = mitk::PlanePositionManager::GetInstance()->AddNewPlanePosition(pos, 20);

    std::stringstream markerStream;
    markerStream << "Position";
    markerStream << " ";
    markerStream << id+1;

    mitk::DataNode::Pointer rotatedContourNode = mitk::DataNode::New();

    //rotatedContourNode->SetData(contourMarker);
    rotatedContourNode->SetProperty( "name", mitk::StringProperty::New(markerStream.str()) );
    rotatedContourNode->SetProperty( "isContourMarker", mitk::BoolProperty::New(true));
    ds->Add(rotatedContourNode);

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id) != 0, "Testing DataStorage Node removed");

    ds->Remove(rotatedContourNode);

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetPlanePosition(id) == 0, "Testing DataStorage Node removed");
}

void testRemoveAll()
{
    MITK_TEST_OUTPUT(<<"Starting Test: ######### R e m o v e A l l #########");

    unsigned int numPos = mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions();
    MITK_TEST_OUTPUT(<<"Testing RemovePlanePosition - Current number of Position: "<<numPos);

    mitk::PlanePositionManager::GetInstance()->RemoveAllPlanePositions();

    MITK_TEST_CONDITION(mitk::PlanePositionManager::GetInstance()->GetNumberOfPlanePositions() == 0, "Testing remove all pos");


}

int mitkPlanePositionManagerTest(int argc, char* argv[])
{
    MITK_TEST_OUTPUT(<<"Starting Test PlanePositionManager");
    testAddPlanePosition();
    testGetPlanePosition();
    testRemovePlanePosition();
    testRemoveAll();
    testDataStorageNodeRemoved();
    testRestorePlanePosition();
    return 0;
}
