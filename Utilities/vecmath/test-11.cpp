/*
   Copyright (C) 1997,1998,1999
   Kenji Hiranabe, Eiwa System Management, Inc.

   This program is free software.
   Implemented by Kenji Hiranabe(hiranabe@esm.co.jp),
   conforming to the Java(TM) 3D API specification by Sun Microsystems.

   Permission to use, copy, modify, distribute and sell this software
   and its documentation for any purpose is hereby granted without fee,
   provided that the above copyright notice appear in all copies and
   that both that copyright notice and this permission notice appear
   in supporting documentation. Kenji Hiranabe and Eiwa System Management,Inc.
   makes no representations about the suitability of this software for any
   purpose.  It is provided "AS IS" with NO WARRANTY.
*/
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif



#include "vecmath.h"
#define assert_(c,m) assert(((void)m, c));
static double epsilon = 1.0e-5;

#ifdef VM_INCLUDE_NAMESPACE
using namespace kh_vecmath;
#endif


template<class T>
T abs(T a) { return a < 0 ? -a : a; }

bool equals_v(double m1, double m2) {	return abs(m1 - m2) < epsilon;}
bool equals_v(float m1, float m2) {	return abs(m1 - m2) < (float)epsilon;}
bool equals(const Matrix3d& m1, const Matrix3d& m2) { return m1.epsilonEquals(m2, epsilon);}
bool equals(const Matrix3f& m1, const Matrix3f& m2) { return m1.epsilonEquals(m2, epsilon);}
bool equals(const Matrix4d& m1, const Matrix4d& m2) { return m1.epsilonEquals(m2, epsilon);}
bool equals(const Matrix4f& m1, const Matrix4f& m2) { return m1.epsilonEquals(m2, epsilon);}
bool equals(const Tuple4d& m1, const Tuple4d& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Tuple3d& m1, const Tuple3d& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Point4d& m1, const Point4d& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Point3d& m1, const Point3d& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Vector4d& m1, const Vector4d& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Vector3d& m1, const Vector3d& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Tuple4f& m1, const Tuple4f& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Tuple3f& m1, const Tuple3f& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Point4f& m1, const Point4f& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Point3f& m1, const Point3f& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Vector4f& m1, const Vector4f& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const Vector3f& m1, const Vector3f& m2) {	return m1.epsilonEquals(m2, epsilon);}
bool equals(const AxisAngle4d& a1, const AxisAngle4d& a2) {
	if (0 < a1.x*a2.x + a1.y*a2.y + a1.z*a2.z) {  // same direction
	    return equals_v(a1.y*a2.z - a1.z*a2.y, 0) &&
            equals_v(a1.z*a2.x - a1.x*a2.z, 0) &&
            equals_v(a1.x*a2.y - a1.y*a2.x, 0) &&
            equals_v(a1.angle, a2.angle);
	} else {
	    return equals_v(a1.y*a2.z - a1.z*a2.y, 0) &&
            equals_v(a1.z*a2.x - a1.x*a2.z, 0) &&
            equals_v(a1.x*a2.y - a1.y*a2.x, 0) &&
            (
             equals_v(a1.angle, -a2.angle) || 
             equals_v(a1.angle + a2.angle, 2*M_PI) || 
             equals_v(a1.angle + a2.angle, -2*M_PI)
             );
	}
}
bool equals(const AxisAngle4f& a1, const AxisAngle4f& a2) {
	if (0 < a1.x*a2.x + a1.y*a2.y + a1.z*a2.z) {  // same direction
	    return equals_v(a1.y*a2.z - a1.z*a2.y, 0) &&
            equals_v(a1.z*a2.x - a1.x*a2.z, 0) &&
            equals_v(a1.x*a2.y - a1.y*a2.x, 0) &&
            equals_v(a1.angle, a2.angle);
	} else {
	    return equals_v(a1.y*a2.z - a1.z*a2.y, 0) &&
            equals_v(a1.z*a2.x - a1.x*a2.z, 0) &&
            equals_v(a1.x*a2.y - a1.y*a2.x, 0) &&
            (
             equals_v(a1.angle, -a2.angle) || 
             equals_v(a1.angle + a2.angle, (float)(2*M_PI)) || 
             equals_v(a1.angle + a2.angle, (float)(-2*M_PI))
             );
	}
}

template<class T>
void Vector3Test(T) {
	Vector3<T> zeroVector;
	Vector3<T> v1(2,3,4);
	Vector3<T> v2(2,5,-8);
	Vector3<T> v3;
	v3.cross(v1, v2);

	// check cross and dot.
	assert(equals_v(v3.dot(v1), 0));
	assert(equals_v(v3.dot(v2), 0));

	// check alias-safe
	v1.cross(v1, v2);
	assert(equals(v1, Vector3<T>(-44,24,4)));

	// check length
	assert(equals_v(v2.lengthSquared(), (T)93));
	assert(equals_v(v2.length(), (T)sqrt(93)));

	// check normalize
	v1.set(v2);
	v2.normalize();
	assert(equals_v(v2.length(), 1));
	v1.cross(v2,v1);
	assert(equals(v1, zeroVector));

	// check Angle
	v1.set(1,2,3);
	v2.set(-1,-6,-3);
	T ang = v1.angle(v2);
	assert(equals_v(T(v1.length()*v2.length()*cos(ang)), v1.dot(v2)));

	// check Angle (0)
	v1.set(v2);
	ang = v1.angle(v2);
	assert(equals_v(ang, 0));
	assert(equals_v(T(v1.length()*v2.length()*cos(ang)), v1.dot(v2)));

	// check small Angle
	v1.set(1,2,3);
	v2.set(1,2,T(3.00001));
	ang = v1.angle(v2);
	assert(equals_v(T(v1.length()*v2.length()*cos(ang)), v1.dot(v2)));

	// check large Angle
	v1.set(1,2,3);
	v2.set(-1,-2,T(-3.00001));
	ang = v1.angle(v2);
	assert(equals_v(T(v1.length()*v2.length()*cos(ang)), v1.dot(v2)));
}

template<class T>
void Matrix3Test(T) {
	Matrix3<T> O = Matrix3<T>();
	Matrix3<T> I = Matrix3<T>(); I.setIdentity();
	Matrix3<T> m1 =Matrix3<T>();
	Matrix3<T> m2 = Matrix3<T>();
	const T v[] = { 2,1,4, 1,-2,3, -3,-1,1 };

	// check get/set
	for (int i = 0; i < 3; i++) {
	    for (int j = 0; j < 3; j++)
            m1.setElement(i, j, i*2*j + 3);
	}
	for (int i2 = 0; i2 < 3; i2++) {
	    for (int j = 0; j < 3; j++)
            assert(equals_v(m1.getElement(i2, j), i2*2*j + 3));
	}

	// check mul with O, I
	m1.set(v);
	m2 = m1;
	m2.mul(O);
	assert(equals(m2, O));
	m2.mul(m1, I);
	assert(equals(m2, m1));

	// check determinant
	assert(equals_v(m1.determinant(), -36));

	// check negate, add
	m2.negate(m1);
	m2.add(m1);
	assert(equals(m2, O));

	// check mul, sub
	m2.negate(m1);
	Matrix3<T> m3 = m1;
	m3.sub(m2);
	m3.mul(0.5);
	assert(equals(m1, m3));
	
	// check invert
	m3.invert(m2);
	m3.mul(m2);
	assert(equals(m3, I));

	// translate
	Point3<T> p1 = Point3<T>(1,2,3);

	// rotZ
	// rotate (1,0,0) 30degree abount z axis -> (cos 30,sin 30,0)
	p1.set(1,0,0);
	m1.rotZ(T(M_PI/6));
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(cos(M_PI/6), sin(M_PI/6), 0)));

	// rotY
	// rotate() (1,0,0) 60degree about y axis -> (cos 60,0,-sin 60)
	p1.set(1,0,0);
	m1.rotY(T(M_PI/3));
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(cos(M_PI/3), 0, -sin(M_PI/3))));

	// rot around arbitary axis
	// rotate() (1,0,0) 60degree about y axis -> (cos 60,0,-sin 60)
	AxisAngle4<T> a1 = AxisAngle4<T>(0,1,0,T(M_PI/3));
	p1.set(1,0,0);
	m1.set(a1);
	m1.transform(p1, &p1);
	assert(equals(p1, Point3<T>(cos(M_PI/3), 0, -sin(M_PI/3))));

	// use quat.
	Quat4<T> q1 = Quat4<T>();
	p1.set(1,0,0);
	q1.set(a1);
	m2.set(q1);
	assert(equals(m1, m2));
	m2.transform(p1, &p1);
	assert(equals(p1, Point3<T>(cos(M_PI/3), 0, -sin(M_PI/3))));

	// Mat <-> Quat <-> Axis
	a1.set(1,2,-3,T(M_PI/3));
	Mat3QuatAxisAngle(a1);

	// Mat <-> Quat <-> Axis (near PI case)
	a1.set(1,2,3,T(M_PI));
	Mat3QuatAxisAngle(a1);
	// Mat <-> Quat <-> Axis (near PI, X major case )
	a1.set(1,T(.1),T(.1),T(M_PI));
	Mat3QuatAxisAngle(a1);
	// Mat <-> Quat <-> Axis (near PI, Y major case )
	a1.set(T(.1),1,T(.1),T(M_PI));
	Mat3QuatAxisAngle(a1);
	// Mat <-> Quat <-> Axis (near PI, Z major case )
	a1.set(T(.1),T(.1),1,T(M_PI));
	Mat3QuatAxisAngle(a1);

	// isometric view 3 times 2/3 turn
	a1.set(1,1,1,T(2*M_PI/3));
	m1.set(a1);
	//printf("m1="+m1);
	p1.set(1,0,0);
	//printf("p1="+p1);
	m1.transform(&p1);
	//printf("after transform p1="+p1);
	assert(equals(p1, Point3<T>(0,1,0)));
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(0,0,1)));
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(1,0,0)));

	// check normalize, normalizeCP
	m1.set(a1);
	assert(equals_v(m1.determinant(), 1));
	assert(equals_v(m1.getScale(), 1));
	m2.set(a1);
	m2.normalize();
	assert(equals(m1, m2));
	m2.set(a1);
	m2.normalizeCP();
	assert(equals(m1, m2));
	T scale = 3.0;
	m2.rotZ(T(-M_PI/4));
	m2.mul(scale);
	assert(equals_v(m2.determinant(), scale*scale*scale));
	assert(equals_v(m2.getScale(), scale));
	m2.normalize();
	assert(equals_v(m2.determinant(), 1));
	assert(equals_v(m2.getScale(), 1));
	m2.rotX(T(M_PI/3));
	m2.mul(scale);
	assert(equals_v(m2.determinant(), scale*scale*scale));
	assert(equals_v(m2.getScale(), scale));
	m2.normalizeCP();
	assert(equals_v(m2.determinant(), 1));
	assert(equals_v(m2.getScale(), 1));

	// transpose and inverse
	m1.set(a1);
	m2.invert(m1);
	m1.transpose();
	assert(equals(m1, m2));
}

template <class T>
void Mat4QuatAxisAngle(AxisAngle4<T>& a1) {
	Matrix4<T> m1 = Matrix4<T>();
	Matrix4<T> m2 = Matrix4<T>();
	AxisAngle4<T> a2 = AxisAngle4<T>();
	Quat4<T> q1 = Quat4<T>();
	Quat4<T> q2 = Quat4<T>();

	// Axis <-> Quat
	q1.set(a1);
	a2.set(q1);
	// a1.v parallels to a2.v 
	assert(equals(a1, a2));
	q2 = Quat4<T>();
	q2.set(a2);
	assert(equals(q1, q2));

	// Quat <-> Mat
	q1.set(a1);
	m1.set(q1);
	q2.set(m1);
	assert(equals(q1, q2));
	m2.set(q2);
	assert(equals(m1, m2));

	// Mat <-> AxisAngle
	m1.set(a1);
	a2.set(m1);
	//System.out.println("a1="+a1);
	//System.out.println("a2="+a2);
	//System.out.println("m1="+m1);
	assert(equals(a1, a2));
	m2.set(a1);
	assert(equals(m1, m2));
	a1.x *= 2; a1.y *= 2; a1.z *= 2;
	m2.set(a1);
	a1.x = -a1.x; a1.y = -a1.y; a1.z = -a1.z; a1.angle = -a1.angle;
	m2.set(a1);
	assert(equals(m1, m2));
}


template<class T>
void Mat3QuatAxisAngle(AxisAngle4<T>& a1) {
	Matrix3<T> m1 = Matrix3<T>();
	Matrix3<T> m2 = Matrix3<T>();
	AxisAngle4<T> a2 = AxisAngle4<T>();
	Quat4<T> q1 = Quat4<T>();
	Quat4<T> q2 = Quat4<T>();

	// Axis <-> Quat
	q1.set(a1);
	a2.set(q1);
	// a1.v parallels to a2.v 
	assert(equals(a1, a2));
	q2 = Quat4<T>();
	q2.set(a2);
	assert(equals(q1, q2));

	// Quat <-> Mat
	q1.set(a1);
	m1.set(q1);
	q2.set(m1);
	assert(equals(q1, q2));
	m2.set(q2);
	assert(equals(m1, m2));

	// Mat <-> AxisAngle
	m1.set(a1);
	a2.set(m1);
	//printf("a1="+a1);
	//printf("a2="+a2);
	//printf("m1="+m1);
	assert(equals(a1, a2));
	m2.set(a1);
	assert(equals(m1, m2));
	a1.x *= 2; a1.y *= 2; a1.z *= 2;
	m2.set(a1);
	a1.x = -a1.x; a1.y = -a1.y; a1.z = -a1.z; a1.angle = -a1.angle;
	m2.set(a1);
	assert(equals(m1, m2));

}

template <class T>
void Matrix4Test(T) {
	Matrix4<T> O = Matrix4<T>();
	Matrix4<T> I = Matrix4<T>(); I.setIdentity();
	Matrix4<T> m1 = Matrix4<T>();
	Matrix4<T> m2 = Matrix4<T>();

	// check get/set
	for (int i = 0; i < 4; i++) {
	    for (int j = 0; j < 4; j++)
            m1.setElement(i, j, i*2*j + 3);
	}
	for (int i2 = 0; i2 < 4; i2++) {
	    for (int j = 0; j < 4; j++)
            assert(equals_v(m1.getElement(i2, j), i2*2*j + 3));
	}

	// check mul with O, I
	m1 = Matrix4<T>(
                      2,1,4,1,
                      -2,3,-3,1,
                      -1,1,2,2,
                      0,8,1,-10);
	m2 = Matrix4<T>(m1);
	m2.mul(O);
	assert_(equals(m2, O), "O = m2 x O");
	m2.mul(m1, I);
    // printf("m2 = %s", m2.toString().c_str());
    // printf("m1 = %s", m1.toString().c_str());
	assert_(equals(m2, m1), "m2 = m1 x I");

	// check negate, add
	m2.negate(m1);
	m2.add(m1);
	assert(equals(m2, O));

	// check mul, sub
	T v[] = { 5,1,4,0,
                   2,3,-4,-1,
                   2,3,-4,-1,
                   1,1,1,1};
	m2.set(v);
	m2.negate(m1);
	Matrix4<T> m3 = Matrix4<T>(m1);
	m3.sub(m2);
	m3.mul(0.5);
	assert(equals(m1, m3));
	
	//printf("4");

	// check invert
	m2 = Matrix4<T>(
                      .5,1,4,1,
                      -2,3,-4,-1,
                      1,9,100,2,
                      -20,2,1,9);
	m3.invert(m2);
	m3.mul(m2);
	assert(equals(m3, I));

	//printf("5");

	// translate
	m1 = Matrix4<T>(
                      -1,2,0,3,
                      -1,1,-3,-1,
                      1,2,1,1,
                      0,0,0,1);
	Point3<T> p1 =  Point3<T>(1,2,3);
	Vector3<T> v1 = Vector3<T>(1,2,3);
	Vector4<T> V2 = Vector4<T>(2,-1,-4,1);

	// printf("m1=" + m1.toString());
    //	assert(m1.toString().equals("[" + NL +
    //                                "  [-1.0	2.0	0.0	3.0]" + NL +
    //                                "  [-1.0	1.0	-3.0	-1.0]" + NL +
    //                                "  [1.0	2.0	1.0	1.0]" + NL +
    // "  [0.0	0.0	0.0	1.0] ]"));

	//printf("6");
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(6,-9,9)));
    //	printf("7");
	m1.transform(V2,&V2);
	assert(equals(V2, Vector4<T>(-1,8,-3,1)));
    //	printf("8");

		      
	
	// rotZ
	// rotate (1,0,0) 30degree abount z axis -> (cos 30,sin 30,0)
	p1.set(1,0,0);
	m1.rotZ(T(M_PI/6));
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(cos(M_PI/6), sin(M_PI/6), 0)));
    //	printf("9");

	// rotY
	// rotate() (1,0,0) 60degree about y axis -> (cos 60,0,-sin 60)
	p1.set(1,0,0);
	m1.rotY(T(M_PI/3));
	m1.transform(&p1);
	assert(equals(p1, Point3<T>( cos(M_PI/3), 0, -sin(M_PI/3))));
    //	printf("10");

	// rot around arbitary axis
	// rotate() (1,0,0) 60degree about y axis -> (cos 60,0,-sin 60)
	AxisAngle4<T> a1 = AxisAngle4<T>(0,1,0,T(M_PI/3));
	p1.set(1,0,0);
	m1.set(a1);
	m1.transform(p1, &p1);
	assert(equals(p1, Point3<T>(
                                  cos(M_PI/3),
                                  0,
                                  -sin(M_PI/3))));
    //	printf("11");

	// use quat.
	Quat4<T> q1 = Quat4<T>();
	p1.set(1,0,0);
	q1.set(a1);
	m2.set(q1);
	assert(equals(m1, m2));
	// printf("12");
	m2.transform(p1, &p1);
	assert(equals(p1, Point3<T>(cos(M_PI/3), 0, -sin(M_PI/3))));
	// printf("13");

	// Mat <-> Quat <-> Axis
	a1.set(1,2,-3,T(M_PI/3));
	Mat4QuatAxisAngle(a1);

	// Mat <-> Quat <-> Axis (near PI case)
	a1.set(1,2,3,T(M_PI));
	Mat4QuatAxisAngle(a1);
	// Mat <-> Quat <-> Axis (near PI, X major case )
	a1.set(1,T(.1),T(.1),T(M_PI));
	Mat4QuatAxisAngle(a1);
	// Mat <-> Quat <-> Axis (near PI, Y major case )
	a1.set(T(.1),1,T(.1),T(M_PI));
	Mat4QuatAxisAngle(a1);
	// Mat <-> Quat <-> Axis (near PI, Z major case )
	a1.set(T(.1),T(.1),1,T(M_PI));
	Mat4QuatAxisAngle(a1);

	// isometric view 3 times 2/3 turn
	a1.set(1,1,1,T(2*M_PI/3));
	m1.set(a1);
	//printf("m1="+m1);
	p1.set(1,0,0);
	//printf("p1="+p1);
	m1.transform(&p1);
	//printf("after transform p1="+p1);
	assert(equals(p1, Point3<T>(0,1,0)));
	// printf("14");
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(0,0,1)));
	//printf("15");
	m1.transform(&p1);
	assert(equals(p1, Point3<T>(1,0,0)));
	// printf("16");

	// check getScale
	m1.set(a1);
	assert(equals_v(m1.determinant(), 1));
	assert(equals_v(m1.getScale(), 1));
	// printf("17");
	m2.set(a1);

	// transpose and inverse
	m1.set(a1);
	m2.invert(m1);
	m1.transpose();
	assert(equals(m1, m2));
	// printf("18");

	// rot, scale, trans
	Matrix3<T> n1;
	n1.set(a1);
	Matrix3<T> n2;
	v1.set(2, -1, -1);
	m1.set(n1, v1, T(0.4));
	m2.set(n1, v1, T(0.4));
	Vector3<T> v2 = Vector3<T>();
	T s = m1.get(&n2, &v2);
	assert(equals(n1, n2));
	assert(equals_v(s, T(0.4)));
	assert(equals(v1, v2));
	assert(equals(m1, m2)); // not modified

}

#if 0
static  void GMatrixTest() {
	GMatrix I44 = new GMatrix(4,4); // Identity 4x4
	GMatrix O44 = new GMatrix(4,4); O44.setZero(); // O 4x4
	GMatrix O34 = new GMatrix(3,4); O34.setZero(); // O 3x4
	GMatrix m1 = new GMatrix(3,4);
	GMatrix m2 = new GMatrix(3,4);
	Matrix3<T> mm1 = new Matrix3<T>();
	Matrix3<T> mm2 = new Matrix3<T>();

	// get/setElement
	for (int i = 0; i < 3; i++)
	    for (int j = 0; j < 4; j++) {
            m1.setElement(i,j,(i+1)*(j+2));
            if (j < 3)
                mm1.setElement(i,j,(i+1)*(j+2));
	    }
	for (int i = 0; i < 3; i++)
	    for (int j = 0; j < 4; j++) {
            assert(equals(m1.getElement(i,j),(i+1)*(j+2)));
	    }

	m1.get(mm2);
	assert(equals(mm1, mm2));

	// mul with I,O
	m2.mul(m1, I44);
	assert(equals(m1, m2));
	m2.mul(m1, O44);
	assert(equals(O34, m2));

	// LUD
	Matrix4<T> mm3 = new Matrix4<T>(
                                1, 2, 3, 4,
                                -2, 3, -1, 3,
                                -1, -2, -4, 1,
                                1, 1, -1, -2
                                );
	Matrix4<T> mm4 = new Matrix4<T>();
	Matrix4<T> mm5 = new Matrix4<T>();
	mm5.set(mm3);

	// setSize, invert
	m1.setSize(4, 4);
	m2.setSize(4, 4);
	m1.set(mm3);
	// printf("m1=" + m1.toString());
	//assert(m1.toString().equals("[" + NL +
    //"  [1.0	2.0	3.0	4.0]" + NL +
    //                          "  [-2.0	3.0	-1.0	3.0]" + NL +
    //                          "  [-1.0	-2.0	-4.0	1.0]" + NL +
    //                          "  [1.0	1.0	-1.0	-2.0] ]"));

	m2.set(m1);
	m1.invert();
	mm3.invert();
	// printf("mm3 = "+mm3.toString());
	// printf("mm5 = "+mm5.toString());
	mm5.mul(mm3);
	// printf("mm5(==I) = "+mm5.toString());
	assert(equals(mm5, new Matrix4<T>(1,0,0,0,
                                    0,1,0,0,
                                    0,0,1,0,
                                    0,0,0,1)));

	m1.get(mm4);
	// printf("m1 = "+m1.toString());
	// printf("mm3 = "+mm3.toString());
	// printf("mm4 = "+mm4.toString());
	assert(equals(mm3, mm4));
	// printf("m1 = "+m1.toString());
	// printf("m2 = "+m2.toString());
	m1.mul(m2);
	// printf("m1*m2 = "+m1.toString());
	assert(equals(m1, I44));

	// LUD
	Matrix4<T> mm6 = new Matrix4<T>(
                                1, 2, 3, 4,
                                -2, 3, -1, 3,
                                -1, -2, -4, 1,
                                1, 1, -1, -2
                                );
	Vector4<T> vv1 = new Vector4<T>(1,-1,-1,2);
	Vector4<T> vv2 = new Vector4<T>();
	Vector4<T> vv3 = new Vector4<T>(4,2,7,-3);
	mm6.transform(vv1, vv2);
	// printf("mm6 = "+mm6.toString());
	// printf("vv1 = "+vv1.toString());
	// printf("vv2 = "+vv2.toString());
	// printf("vv3 = "+vv3.toString());
	assert(equals(vv2, vv3));

	m1.set(mm6);
	GVector x = new GVector(4);
	GVector v2 = new GVector(4);
	GVector b = new GVector(4);
	x.set(vv1); // (1,-1,-1,2)
	b.set(vv3); // (4,2,7,-3)
	GVector mx = new GVector(4);
	mx.mul(m1, x); // M*x = (4,2,7,-3)
	assert(equals(mx, b));

	GVector p = new GVector(4);
	m1.LUD(m2, p);
	assert(checkLUD(m1, m2, p));
	GVector xx = new GVector(4);
	xx.LUDBackSolve(m2, b, p);
	assert(equals(xx, x));
	
	GMatrix u = new GMatrix(m1.getNumRow(), m1.getNumRow());
	GMatrix w = new GMatrix(m1.getNumRow(), m1.getNumCol());
	GMatrix v = new GMatrix(m1.getNumCol(), m1.getNumCol());
	int rank = m1.SVD(u, w, v);
	assert(rank == 4);
	assert(checkSVD(m1, u, w, v));
	xx.SVDBackSolve(u, w, v, b);
	assert(equals(xx, x));

	// overwrite m1 -LUD-> m1
	// m1.LUD(m1, p);
	// xx.LUDBackSolve(m2, b, p);
	// assert(equals(xx, x));
}

static bool checkLUD(GMatrix m, GMatrix LU, GVector permutation) {
	int n = m.getNumCol();
	bool ok = true;
	for (int i = 0; i < n; i++) {
	    for (int j = 0; j < n; j++) {
            T aij = 0.0;
            int min = i < j ? i : j;
            for (int k = 0; k <= min; k++) {
                if (i != k)
                    aij += LU.getElement(i, k)*LU.getElement(k, j);
                else
                    aij += LU.getElement(k, j);
            }
            if (abs(aij - m.getElement((int)permutation.getElement(i),j)) > epsilon) {
                printf("a["+i+","+j+"] = "+aij+"(LU)ij ! = "+m.getElement((int)permutation.getElement(i),j));
                ok = false;
            }
	    }
	}
	return ok;
}
    
static bool checkSVD(GMatrix m, GMatrix u, GMatrix w, GMatrix v) {
	bool ok = true;
	int wsize = w.getNumRow() < w.getNumRow() ? w.getNumRow() : w.getNumCol();
	
	for (int i = 0; i < m.getNumRow(); i++) {
	    for (int j = 0; j < m.getNumCol(); j++) {
            T sum = 0.0;
            for (int k = 0; k < m.getNumCol(); k++) {
                sum += u.getElement(i,k)*w.getElement(k,k)*v.getElement(j,k);
            }
            /* check if SVD is OK */
            if (epsilon < abs(m.getElement(i, j)-sum)) {
                printf("(SVD)ij = "+sum +" != a["+i+","+j+"] = "+m.getElement(i,j));
                ok = false;
            }
	    }
	    
	}
	if (!ok) {
	    System.out.print("[W] = ");
	    printf(w);
	    System.out.print("[U] = ");
	    printf(u);
	    System.out.print("[V] = ");
	    printf(v);
	}
	return ok;
}

#endif

/**
 * test for Mat/Quat/AxisAngle rotations
 */
#ifdef TESTALL
int test_11() {
#else
int main(int, char**) {
#endif
	Vector3Test(1.0);
	Vector3Test(1.0f);

	Matrix3Test(1.0);
	Matrix3Test(1.0f);

	Matrix4Test(1.0);
	Matrix4Test(1.0f);
    return 0;
}
