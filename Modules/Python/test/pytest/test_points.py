import mitk


def test_point_vector_types():
    point2d = mitk.Point2D(1.0, 2.0)
    assert point2d.x == 1.0
    assert point2d.y == 2.0

    point3d = mitk.Point3D(1.0, 2.0, 3.0)
    assert point3d.z == 3.0

    vector2d = mitk.Vector2D(4.0, 5.0)
    assert vector2d.x == 4.0

    vector3d = mitk.Vector3D(4.0, 5.0, 6.0)
    assert vector3d.z == 6.0
