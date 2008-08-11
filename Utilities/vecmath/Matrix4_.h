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
#ifndef MATRIX4__H
#define MATRIX4__H

#include "VmUtil.h"
#include "Matrix3_.h"
#include "Vector4.h"
#include "Point4.h"

VM_BEGIN_NS

/**
 * A  4 x 4 matrix.
 * @version specification 1.1, implementation $Revision$, $Date$
 * @author Kenji hiranabe
 */
template<class T>
class Matrix4 {
protected:
    static T abs(T t) { return VmUtil<T>::abs(t); }
/*
 * $Log$
 * Revision 1.3  2005/10/13 13:50:04  maleike
 * FIX: doxygen warnings
 *
 * Revision 1.2  2003/04/22 14:42:11  max
 * made inclusions of vecmath header files "local" (for gcc 3.3 support)
 *
 * Revision 1.1  2002/11/16 12:25:36  mark
 * vecmath bibliothek
 * in cvs, da einige bugs behoben
 *
 * Revision 1.3  1999/10/06  02:52:46  hiranabe
 * Java3D 1.2 and namespace
 *
 * Revision 1.2  1999/05/26  00:59:37  hiranabe
 * support Visual C++
 *
 * Revision 1.1  1999/03/04  11:07:09  hiranabe
 * Initial revision
 *
 * Revision 1.1  1999/03/04  11:07:09  hiranabe
 * Initial revision
 *
 */
public:
    /**
     * the type for values
     */
    typedef T value_type;
    /**
     * the type for index
     */
    typedef size_t size_type;
    /**
     * dimension
     */
    enum { DIMENSION = 4 };
    /**
     * the type for tuple
     */
    typedef Tuple4<T> tuple_type;
    /**
     * the type for vector
     */
    typedef Vector4<T> vector_type;
    /**
     * the type for point
     */
    typedef Point4<T> point_type;

    /**
      * The first element of the first row.
      */
    T m00;

    /**
      * The second element of the first row.
      */
    T m01;

    /**
      * third element of the first row.
      */
    T m02;

    /**
      * The fourth element of the first row.
      */
    T m03;

    /**
      * The first element of the second row.
      */
    T m10;

    /**
      * The second element of the second row.
      */
    T m11;

    /**
      * The third element of the second row.
      */
    T m12;

    /**
      * The fourth element of the second row.
      */
    T m13;

    /**
      * The first element of the third row.
      */
    T m20;

    /**
      * The second element of the third row.
      */
    T m21;

    /**
      * The third element of the third row.
      */
    T m22;

    /**
      * The fourth element of the third row.
      */
    T m23;

    /**
      * The first element of the fourth row.
      */
    T m30;

    /**
      * The second element of the fourth row.
      */
    T m31;

    /**
      * The third element of the fourth row.
      */
    T m32;

    /**
      * The fourth element of the fourth row.
      */
    T m33;

    /**
      * Constructs and initializes a Matrix4 from the specified 16 values.
      * @param m00 the [0][0] element
      * @param m01 the [0][1] element
      * @param m02 the [0][2] element
      * @param m03 the [0][3] element
      * @param m10 the [1][0] element
      * @param m11 the [1][1] element
      * @param m12 the [1][2] element
      * @param m13 the [1][3] element
      * @param m20 the [2][0] element
      * @param m21 the [2][1] element
      * @param m22 the [2][2] element
      * @param m23 the [2][3] element
      * @param m30 the [3][0] element
      * @param m31 the [3][1] element
      * @param m32 the [3][2] element
      * @param m33 the [3][3] element
      */
    Matrix4(T m00, T m01, T m02, T m03, 
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33);

    /**
      * Constructs and initializes a Matrix4 from the specified 16
      * element array.  this.m00 =v[0], this.m01=v[1], etc.
      * @param  v the array of length 16 containing in order
      */
    Matrix4(const T v[]);

    /**
      * Constructs and initializes a Matrix4 from the specified 4x4
      * element array.  this.m00 =m[0][0], this.m01=m[0][1], etc.
      * @param  m the array of 4 x 4 containing in order
      */
    Matrix4(const T m[][4]);

    /**
      * Constructs and initializes a Matrix4 from the quaternion,
      * translation, and scale values; the scale is applied only to the
      * rotational components of the matrix (upper 3x3) and not to the
      * translational components.
      * @param q1  The quaternion value representing the rotational component
      * @param t1  The translational component of the matrix
      * @param s  The scale value applied to the rotational components
      */
    Matrix4(const Quat4<T>& q1, const Vector3<T>& t1, T s);

#if 0
    /**
      * Constructs and initializes a Matrix4 from the quaternion,
      * translation, and scale values; the scale is applied only to the
      * rotational components of the matrix (upper 3x3) and not to the
      * translational components.
      * @param q1  The quaternion value representing the rotational component
      * @param t1  The translational component of the matrix
      * @param s  The scale value applied to the rotational components
      */
    Matrix4(Quat4f q1, Vector3d t1, T s) {
	set(q1, t1, s);
    }

    /**
      * Constructs a new matrix with the same values as the Matrix4f parameter.
      * @param m1 The source matrix.
      */
    Matrix4(Matrix4f m1) {
	set(m1);
    }

    /**
      * Constructs and initializes a Matrix4 from the rotation matrix,
      * translation, and scale values; the scale is applied only to the
      * rotational components of the matrix (upper 3x3) and not to the
      * translational components.
      * @param m1  The rotation matrix representing the rotational components
      * @param t1  The translational components of the matrix
      * @param s  The scale value applied to the rotational components
      */
    Matrix4(Matrix3f m1, Vector3d t1, T s) {
	// why no set(Matrix3f, Vector3d, T) ?
	// set(Matrix3f, Vector3f, float) is there.
	// feel inconsistent.
	set(m1);
	mulRotationScale(s);
	setTranslation(t1);
	m33 = 1.0;
    }
#endif

    /**
      * Constructs and initializes a Matrix4 from the rotation matrix,
      * translation, and scale values; the scale is applied only to the
      * rotational components of the matrix (upper 3x3) and not to the
      * translational components.
      * @param m1  The rotation matrix representing the rotational components
      * @param t1  The translational components of the matrix
      * @param s  The scale value applied to the rotational components
      */
    Matrix4(const Matrix3<T>& m1, const Vector3<T>& t1, T s);

    /**
      * Constructs and initializes a Matrix4 to all zeros.
      */
    Matrix4();

    /**
      * Sets 16 values	
      * @param m00 the [0][0] element
      * @param m01 the [0][1] element
      * @param m02 the [0][2] element
      * @param m03 the [0][3] element
      * @param m10 the [1][0] element
      * @param m11 the [1][1] element
      * @param m12 the [1][2] element
      * @param m13 the [1][3] element
      * @param m20 the [2][0] element
      * @param m21 the [2][1] element
      * @param m22 the [2][2] element
      * @param m23 the [2][3] element
      * @param m30 the [3][0] element
      * @param m31 the [3][1] element
      * @param m32 the [3][2] element
      * @param m33 the [3][3] element
      */
    void set(T m00, T m01, T m02, T m03, 
             T m10, T m11, T m12, T m13,
             T m20, T m21, T m22, T m23,
             T m30, T m31, T m32, T m33);

    /**
      * Sets the values in this Matrix4 equal to the row-major array parameter
      * (ie, the first four elements of the array will be copied into the first
      * row of this matrix, etc.).
      */
    void set(const T m[]);

    /**
      * Sets the values in this Matrix4 equal to the row-major array parameter
      * (ie, the first four elements of the array will be copied into the first
      * row of this matrix, etc.).
      */
    void set(const T m[][4]);

    /**
      * Sets the value of this matrix to a copy of the
      * passed matrix m1.
      * @param m1 the matrix to be copied
      */
    void set(const Matrix4<T>& m1);

    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * values in the T precision Matrix3d argument; the other elements of
      * this matrix are initialized as if this were an identity matrix
      * (ie, affine matrix with no translational component).
      * @param m1 the 3x3 matrix
      */
    void set(const Matrix3<T>& m1);

    /**
      * Sets the value of this matrix to the matrix conversion of the
      * (T precision) quaternion argument. 
      * @param q1 the quaternion to be converted 
      */
    void set(const Quat4<T>& q1);

    /**
      * Sets the value of this matrix to the matrix conversion of the
      * T precision axis and angle argument. 
      * @param a1 the axis and angle to be converted 
      */
    void set(const AxisAngle4<T>& a1);

    /**
     * Sets this Matrix4 to identity.
     */
    void setIdentity();

    /**
     * Sets the specified element of this matrix4d to the value provided.
     * @param row  the row number to be modified (zero indexed)
     * @param column  the column number to be modified (zero indexed)
     * @param value the new value
     */
    void setElement(size_t row, size_t column, T value);

    /**
     * Retrieves the value at the specified row and column of this matrix.
     * @param row  the row number to be retrieved (zero indexed)
     * @param column  the column number to be retrieved (zero indexed)
     * @return the value at the indexed element
     */
    T getElement(size_t row, size_t column) const;

    /**
     * Retrieves the lvalue at the specified row and column of this matrix.
     * @param row  the row number to be retrieved (zero indexed)
     * @param column  the column number to be retrieved (zero indexed)
     * @return the lvalue at the indexed element
     */
    T& getElementReference(size_type row, size_type column);

    /**
      * Performs an SVD normalization of this matrix in order to acquire the
      * normalized rotational component; the values are placed into the Matrix3d parameter.
      * @param m1 matrix into which the rotational component is placed
      */
    void get(Matrix3<T>* m1) const;

#if 0
    /**
      * Performs an SVD normalization of this matrix in order to acquire the
      * normalized rotational component; the values are placed into the Matrix3f parameter.
      * @param m1 matrix into which the rotational component is placed
      */
    void get(Matrix3f m1) const{
	SVD(m1);
    }
#endif

    /**
      * Performs an SVD normalization of this matrix to calculate the rotation
      * as a 3x3 matrix, the translation, and the scale. None of the matrix values are modified.
      * @param m1 The normalized matrix representing the rotation
      * @param t1 The translation component
      * @return The scale component of this transform
      */
    T get(Matrix3<T>* m1, Vector3<T>* t1) const;

    /**
      * Performs an SVD normalization of this matrix in order to acquire the
      * normalized rotational component; the values are placed into
      * the Quat4f parameter.
      * @param q1 quaternion into which the rotation component is placed
      */
    void get(Quat4<T>* q1) const;

#if 0
    /**
      * Performs an SVD normalization of this matrix to calculate the rotation
      * as a 3x3 matrix, the translation, and the scale. None of the matrix values are modified.
      * @param m1 The normalized matrix representing the rotation
      * @param t1 The translation component
      * @return The scale component of this transform
      */
    T get(Matrix3f m1, Vector3d t1) {
	get(t1);
	return SVD(m1);
    }

    /**
      * Performs an SVD normalization of this matrix in order to acquire the
      * normalized rotational component; the values are placed into
      * the Quat4f parameter.
      * @param q1 quaternion into which the rotation component is placed
      */
    void get(Quat4f q1) {
	q1.set(this);
	q1.normalize();
    }


    /**
      * Gets the upper 3x3 values of this matrix and places them into the matrix m1.
      * @param m1 The matrix that will hold the values
      */
    void getRotationScale(Matrix3f m1) {
	m1.m00 = (float)m00; m1.m01 = (float)m01; m1.m02 = (float)m02;
	m1.m10 = (float)m10; m1.m11 = (float)m11; m1.m12 = (float)m12;
	m1.m20 = (float)m20; m1.m21 = (float)m21; m1.m22 = (float)m22;
    }
#endif

    /**
      * Retrieves the translational components of this matrix.
      * @param trans the vector that will receive the translational component
      */
    void get(Vector3<T>* trans) const {
        assert(trans != 0);
        trans->x = m03;
        trans->y = m13;
        trans->z = m23;
    }


    /**
      * Gets the upper 3x3 values of this matrix and places them into the matrix m1.
      * @param m1 The matrix that will hold the values
      */
    void getRotationScale(Matrix3<T>* m1) const;

    /**
      * Performs an SVD normalization of this matrix to calculate and return the
      * uniform scale factor. This matrix is not modified.
      * @return the scale factor of this matrix
      */
    T getScale() const;

    /**
      * Replaces the upper 3x3 matrix values of this matrix with the values in the matrix m1.
      * @param m1 The matrix that will be the new upper 3x3
      */
    void setRotationScale(const Matrix3<T>& m1);

#if 0
    /**
      * Replaces the upper 3x3 matrix values of this matrix with the values in the matrix m1.
      * @param m1 The matrix that will be the new upper 3x3
      */
    void setRotationScale(Matrix3f m1) {
	m00 = m1.m00; m01 = m1.m01; m02 = m1.m02;
	m10 = m1.m10; m11 = m1.m11; m12 = m1.m12;
	m20 = m1.m20; m21 = m1.m21; m22 = m1.m22;
    }
#endif

    /**
      * Sets the scale component of the current matrix by factoring out the
      * current scale (by doing an SVD) from the rotational component and
      * multiplying by the new scale.
      * note: this method doesn't change m44.
      * @param scale the new scale amount
      */
    void setScale(T scale);

    /**
     * Sets the specified row of this matrix4d to the four values provided.
     * @param row  the row number to be modified (zero indexed)
     * @param x the first column element
     * @param y the second column element
     * @param z the third column element
     * @param w the fourth column element
     */
    void setRow(size_t row, T x, T y, T z, T w);

    /**
     * Sets the specified row of this matrix4d to the Vector provided.
     * @param row the row number to be modified (zero indexed)
     * @param v the replacement row
     */
    void setRow(size_t row, const Vector4<T>& v);

    /**
      * Sets the specified row of this matrix4d to the four values provided.
      * @param row the row number to be modified (zero indexed)
      * @param v the replacement row
      */
    void setRow(size_t row, const T v[]);

    /**
     * Copies the matrix values in the specified row into the
     * vector parameter.
     * @param row the matrix row
     * @param v The vector into which the matrix row values will be copied
     */
    void getRow(size_t row, Vector4<T>* v) const;

    /**
      * Copies the matrix values in the specified row into the
      * array parameter.
      * @param row the matrix row
      * @param v The array into which the matrix row values will be copied
      */
    void getRow(size_t row, T v[]) const;

    /**
      * Sets the specified column of this matrix4d to the four values provided.
      * @param  column the column number to be modified (zero indexed)
      * @param x the first row element
      * @param y the second row element
      * @param z the third row element
      * @param w the fourth row element
      */
    void setColumn(size_t column, T x, T y, T z, T w);

    /**
      * Sets the specified column of this matrix4d to the vector provided.
      * @param column the column number to be modified (zero indexed)
      * @param v the replacement column
      */
    void setColumn(size_t column, const Vector4<T>& v);

    /**
      * Sets the specified column of this matrix4d to the four values provided. 
      * @param column  the column number to be modified (zero indexed) 
      * @param v       the replacement column 
      */
    void setColumn(size_t column,  const T v[]);

    /**
     * Copies the matrix values in the specified column into the
     * vector parameter.
     * @param column the matrix column
     * @param v The vector into which the matrix column values will be copied
     */
    void getColumn(size_t column, Vector4<T>* v) const;

    /**
      * Copies the matrix values in the specified column into the
      * array parameter.
      * @param column the matrix column
      * @param v The array into which the matrix column values will be copied
      */
    void getColumn(size_t column, T v[]) const;

    /**
      * Adds a scalar to each component of this matrix.
      * @param scalar The scalar adder.
      */
    void add(T scalar);

    /**
      * Subtracts a scalar from each component of this matrix.
      * @param scalar The scalar adder.
      */
    void sub(T scalar);

    /**
      * Adds a scalar to each component of the matrix m1 and places
      * the result into this. Matrix m1 is not modified.
      * @param scalar The scalar adder.
      * @param m1 The original matrix values.
      */
    void add(T scalar, const Matrix4& m1) {
        set(m1);
        add(scalar);
    }

    /**
     * Sets the value of this matrix to the matrix sum of matrices m1 and m2. 
     * @param m1 the first matrix 
     * @param m2 the second matrix 
     */
    void add(const Matrix4& m1, const Matrix4& m2);

    /**
     * Sets the value of this matrix to sum of itself and matrix m1. 
     * @param m1 the other matrix 
     */
    void add(const Matrix4& m1);

    /**
      * Sets the value of this matrix to the matrix difference
      * of matrices m1 and m2. 
      * @param m1 the first matrix 
      * @param m2 the second matrix 
      */
    void sub(const Matrix4& m1, const Matrix4& m2);

    /**
     * Sets the value of this matrix to the matrix difference of itself
     * and matrix m1 (this = this - m1). 
     * @param m1 the other matrix 
     */
    void sub(const Matrix4& m1);

    /**
      * Sets the value of this matrix to its transpose. 
      */
    void transpose();

    /**
     * Sets the value of this matrix to the transpose of the argument matrix
     * @param m1 the matrix to be transposed 
     */
    void transpose(const Matrix4& m1) {
        // alias-safe
        set(m1);
        transpose();
    }

#if 0
    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * values in the single precision Matrix3f argument; the other elements of
      * this matrix are initialized as if this were an identity matrix
      * (ie, affine matrix with no translational component).
      * @param m1 the 3x3 matrix
      */
    void set(Matrix3f m1)  {
	m00 = m1.m00; m01 = m1.m01; m02 = m1.m02; m03 = 0.0;
	m10 = m1.m10; m11 = m1.m11; m12 = m1.m12; m13 = 0.0;
	m20 = m1.m20; m21 = m1.m21; m22 = m1.m22; m23 = 0.0;
	m30 =    0.0; m31 =    0.0; m32 =    0.0; m33 = 1.0;
    }

    /**
     * Sets the value of this matrix to the matrix conversion of the
     * single precision quaternion argument. 
     * @param q1 the quaternion to be converted 
     */
    void set(Quat4f q1)  {
	setFromQuat(q1.x, q1.y, q1.z, q1.w);
    }

    /**
      * Sets the value of this matrix to the matrix conversion of the
      * single precision axis and angle argument. 
      * @param a1 the axis and angle to be converted 
      */
    void set(AxisAngle4f a1) {
        setFromAxisAngle(a1.x, a1.y, a1.z, a1.angle);
    }
#endif

  /**
    * Sets the value of this matrix from the rotation expressed by the
    * quaternion q1, the translation t1, and the scale s.
    * @param q1  the rotation expressed as a quaternion
    * @param t1  the translation
    * @param s  the scale value
    */
    void set(const Quat4<T>& q1, const Vector3<T>& t1, T s);

#if 0
  /**
    * Sets the value of this matrix from the rotation expressed by the
    * quaternion q1, the translation t1, and the scale s.
    * @param q1  the rotation expressed as a quaternion
    * @param t1  the translation
    * @param s  the scale value
    */
    void set(Quat4f q1, Vector3d t1, T s) {
        set(q1);
        mulRotationScale(s);
        m03 = t1.x;
        m13 = t1.y;
        m23 = t1.z;
    }

  /**
    * Sets the value of this matrix from the rotation expressed by the
    * quaternion q1, the translation t1, and the scale s.
    * @param q1  the rotation expressed as a quaternion
    * @param t1  the translation
    * @param s  the scale value
    */
    void set(Quat4f q1, Vector3f t1, float s) {
	set(q1);
	mulRotationScale(s);
	m03 = t1.x;
	m13 = t1.y;
	m23 = t1.z;
    }

    /**
      * Sets the value of this matrix to the T value of the
      * passed matrix4f.
      * @param m1 the matrix4f
      */
    void set(Matrix4f m1) {
	m00 = m1.m00; m01 = m1.m01; m02 = m1.m02; m03 = m1.m03;
	m10 = m1.m10; m11 = m1.m11; m12 = m1.m12; m13 = m1.m13;
	m20 = m1.m20; m21 = m1.m21; m22 = m1.m22; m23 = m1.m23;
	m30 = m1.m30; m31 = m1.m31; m32 = m1.m32; m33 = m1.m33;
    }
#endif

    /**
     * Sets the value of this matrix to the matrix inverse
     * of the passed matrix m1. 
     * @param m1 the matrix to be inverted 
     */
    void invert(const Matrix4<T>& m1);

    /**
     * Sets the value of this matrix to its inverse.
     */
    void invert();

    /**
     * Computes the determinant of this matrix. 
     * @return the determinant of the matrix 
     */
    T determinant() const;

    /**
     * Sets the value of this matrix to a scale matrix with the
     * passed scale amount. 
     * @param scale the scale factor for the matrix 
     */
    void set(T scale);

    /**
      * Modifies the translational components of this matrix to the values of
      * the Vector3d argument; the other values of this matrix are not modified.
      * @param trans the translational component
      */
    void setTranslation(const Vector3<T>& trans) {
        m03 = trans.x;
        m13 = trans.y;  
        m23 = trans.z;
    }

    /**
     * Sets the value of this matrix to a translate matrix by the
     * passed translation value.
     * @param v1 the translation amount
     */
    void set(const Vector3<T>& v1) {
        setIdentity();
        setTranslation(v1);
    }

    /**
     * Sets the value of this matrix to a scale and translation matrix;
     * scale is not applied to the translation and all of the matrix
     * values are modified.
     * @param scale the scale factor for the matrix
     * @param v1 the translation amount
     */
    void set(T scale, const Vector3<T>& v1) {
        set(scale);
        setTranslation(v1);
    }

    /**
     * Sets the value of this matrix to a scale and translation matrix;
     * the translation is scaled by the scale factor and all of the
     * matrix values are modified.
     * @param v1 the translation amount
     * @param scale the scale factor for the matrix
     */
    void set(const Vector3<T>& v1, T scale);

#if 0
    /**
      * Sets the value of this matrix from the rotation expressed by the
      * rotation matrix m1, the translation t1, and the scale s. The translation
      * is not modified by the scale.
      * @param m1 The rotation component
      * @param t1 The translation component
      * @param scale The scale component
      */
    void set(Matrix3f m1, Vector3f t1, float scale) {
	setRotationScale(m1);
	mulRotationScale(scale);
	setTranslation(t1);
	m33 = 1.0;
    }
#endif

    /**
      * Sets the value of this matrix from the rotation expressed by the
      * rotation matrix m1, the translation t1, and the scale s. The translation
      * is not modified by the scale.
      * @param m1 The rotation component
      * @param t1 The translation component
      * @param scale The scale component
      */
    void set(const Matrix3<T>& m1, const Vector3<T>& t1, T scale);

    /**
     * Sets the value of this matrix to a rotation matrix about the x axis
     * by the passed angle. 
     * @param angle the angle to rotate about the X axis in radians 
     */
    void rotX(T angle);

    /**
     * Sets the value of this matrix to a rotation matrix about the y axis
     * by the passed angle. 
     * @param angle the angle to rotate about the Y axis in radians 
     */
    void rotY(T angle);

    /**
     * Sets the value of this matrix to a rotation matrix about the z axis
     * by the passed angle. 
     * @param angle the angle to rotate about the Z axis in radians 
     */
    void rotZ(T angle);

    /**
      * Multiplies each element of this matrix by a scalar.
      * @param scalar The scalar multiplier.
      */
     void mul(T scalar);

    /**
      * Multiplies each element of matrix m1 by a scalar and places the result
      * into this. Matrix m1 is not modified.
      * @param scalar The scalar multiplier.
      * @param m1 The original matrix.
      */
     void mul(T scalar, const Matrix4& m1) {
         set(m1);
         mul(scalar);
     }

    /**
     * Sets the value of this matrix to the result of multiplying itself
     * with matrix m1. 
     * @param m1 the other matrix 
     */
    void mul(const Matrix4& m1) {
        mul(*this, m1);
    }

    /**
     * Sets the value of this matrix to the result of multiplying
     * the two argument matrices together. 
     * @param m1 the first matrix 
     * @param m2 the second matrix 
     */
    void mul(const Matrix4& m1, const Matrix4& m2);

    /**
      * Multiplies the transpose of matrix m1 times the transpose of matrix m2,
      * and places the result into this.
      * @param m1 The matrix on the left hand side of the multiplication
      * @param m2 The matrix on the right hand side of the multiplication
      */
    void mulTransposeBoth(const Matrix4& m1, const Matrix4& m2) {
        mul(m2, m1);
        transpose();
    }

    /**
      * Multiplies matrix m1 times the transpose of matrix m2, and places the
      * result into this.
      * @param m1 The matrix on the left hand side of the multiplication
      * @param m2 The matrix on the right hand side of the multiplication
      */
    void mulTransposeRight(const Matrix4& m1, const Matrix4& m2);
    
    /**
      * Multiplies the transpose of matrix m1 times matrix m2, and places the
      * result into this.
      * @param m1 The matrix on the left hand side of the multiplication
      * @param m2 The matrix on the right hand side of the multiplication
      */
    void mulTransposeLeft(const Matrix4& m1, const Matrix4& m2);

    /**
     * Returns true if all of the data members of Matrix4 m1 are
     * equal to the corresponding data members in this Matrix4. 
     * @param m1 The matrix with which the comparison is made. 
     * @return true or false 
     */
    bool equals(const Matrix4& m1) const;

    /**
      * Returns true if the L-infinite distance between this matrix and matrix
      * m1 is less than or equal to the epsilon parameter, otherwise returns
      * false. The L-infinite distance is equal to MAX[i=0,1,2,3 ; j=0,1,2,3 ;
      * abs(this.m(i,j) - m1.m(i,j)]
      * @param m1 The matrix to be compared to this matrix
      * @param epsilon the threshold value
      */
    bool epsilonEquals(const Matrix4& m1, T epsilon) const;

    /**
     * Returns a hash number based on the data values in this
     * object.  Two different Matrix4 objects with identical data values
     * (ie, returns true for equals(Matrix4) ) will return the same hash
     * number.  Two objects with different data members may return the
     * same hash value, although this is not likely.
     * @return the integer hash value 
     */
    size_t hashCode() const {
        return VmUtil<T>::hashCode(sizeof *this, this);
    }

    /**
     * Transform the vector vec using this Matrix4 and place the
     * result into vecOut.
     * @param vec the T precision vector to be transformed
     * @param vecOut the vector into which the transformed values are placed
     */
    void transform(const Tuple4<T>& vec, Tuple4<T>* vecOut) const;

    /**
     * Transform the vector vec using this Matrix4 and place the
     * result back into vec.
     * @param vec the T precision vector to be transformed
     */
    void transform(Tuple4<T>* vec) const {
        transform(*vec, vec);
    }

#if 0
    /**
     * Transform the vector vec using this Matrix4 and place the
     * result into vecOut.
     * @param vec the single precision vector to be transformed
     * @param vecOut the vector into which the transformed values are placed
     */
    void transform(Tuple4f vec, Tuple4f vecOut) {
	// alias-safe
	vecOut.set(
	    (float)(m00*vec.x + m01*vec.y + m02*vec.z + m03*vec.w),
	    (float)(m10*vec.x + m11*vec.y + m12*vec.z + m13*vec.w),
	    (float)(m20*vec.x + m21*vec.y + m22*vec.z + m23*vec.w),
	    (float)(m30*vec.x + m31*vec.y + m32*vec.z + m33*vec.w)
	    );
    }


    /**
     * Transform the vector vec using this Matrix4 and place the
     * result back into vec.
     * @param vec the single precision vector to be transformed
     */
    void transform(Tuple4f vec)  {
	transform(vec, vec);
    }
#endif

    /**
      * Transforms the point parameter with this Matrix4 and places the result
      * into pointOut. The fourth element of the point input paramter is assumed
      * to be one.
      * @param point the input point to be transformed.
      * @param pointOut the transformed point
      */
    void transform(const Point3<T>& point, Point3<T>* pointOut) const;


    /**
     * Transforms the point parameter with this Matrix4 and
     * places the result back into point.  The fourth element of the
     * point input paramter is assumed to be one.
     * @param point the input point to be transformed.
     */
    void transform(Point3<T>* point) const {
        assert(point != 0);
        transform(*point, point);
    }

#if 0
    /**
      * Transforms the point parameter with this Matrix4 and places the result
      * into pointOut. The fourth element of the point input paramter is assumed
      * to be one.
      * @param point the input point to be transformed.
      * @param pointOut the transformed point
      */
    void transform(Point3f point, Point3f pointOut) {
	pointOut.set(
	    (float)(m00*point.x + m01*point.y + m02*point.z + m03),
	    (float)(m10*point.x + m11*point.y + m12*point.z + m13),
	    (float)(m20*point.x + m21*point.y + m22*point.z + m23)
	    );
    }

    /**
     * Transforms the point parameter with this Matrix4 and
     * places the result back into point.  The fourth element of the
     * point input paramter is assumed to be one.
     * @param point the input point to be transformed.
     */
    void transform(Point3f point) {
	transform(point, point);
    }
#endif

    /**
     * Transforms the normal parameter by this Matrix4 and places the value
     * into normalOut.  The fourth element of the normal is assumed to be zero.
     * @param normal the input normal to be transformed.
     * @param normalOut the transformed normal
     */
    void transform(const Vector3<T>& normal, Vector3<T>* normalOut) const;

    /**
     * Transforms the normal parameter by this transform and places the value
     * back into normal.  The fourth element of the normal is assumed to be zero.
     * @param normal the input normal to be transformed.
     */
    void transform(Vector3<T>* normal) const {
        assert(normal != 0);
        transform(*normal, normal);
    }

#if 0
    /**
     * Transforms the normal parameter by this Matrix4 and places the value
     * into normalOut.  The fourth element of the normal is assumed to be zero.
     * @param normal the input normal to be transformed.
     * @param normalOut the transformed normal
     */
    void transform(Vector3f normal, Vector3f normalOut) {
	normalOut.set(
	    (float)(m00 * normal.x + m01 * normal.y + m02 * normal.z),
	    (float)(m10 * normal.x + m11 * normal.y + m12 * normal.z),
	    (float)(m20 * normal.x + m21 * normal.y + m22 * normal.z)
	    );
    }

    /**
     * Transforms the normal parameter by this transform and places the value
     * back into normal.  The fourth element of the normal is assumed to be zero.
     * @param normal the input normal to be transformed.
     */
    void transform(Vector3f normal) {
	transform(normal, normal);
    }
#endif

    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * values in the T precision Matrix3d argument; the other elements of
      * this matrix are unchanged; a singular value decomposition is performed
      * on this object's upper 3x3 matrix to factor out the scale, then this
      * object's upper 3x3 matrix components are replaced by the passed rotation
      * components, and then the scale is reapplied to the rotational
      * components.
      * @param m1 T precision 3x3 matrix
      */
    void setRotation(const Matrix3<T>& m1);

#if 0
    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * values in the single precision Matrix3f argument; the other elements of
      * this matrix are unchanged; a singular value decomposition is performed
      * on this object's upper 3x3 matrix to factor out the scale, then this
      * object's upper 3x3 matrix components are replaced by the passed rotation
      * components, and then the scale is reapplied to the rotational
      * components.
      * @param m1 single precision 3x3 matrix
      */
    void setRotation(Matrix3f m1) {
        T scale = SVD(null, null);
        setRotationScale(m1);
        mulRotationScale(scale);
    }

    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * equivalent values of the quaternion argument; the other elements of this
      * matrix are unchanged; a singular value decomposition is performed on
      * this object's upper 3x3 matrix to factor out the scale, then this
      * object's upper 3x3 matrix components are replaced by the matrix
      * equivalent of the quaternion, and then the scale is reapplied to the
      * rotational components.
      * @param q1 the quaternion that specifies the rotation
      */
    void setRotation(Quat4f q1) {
	T scale = SVD(null, null);

	// save other values
	T tx = m03; 
	T ty = m13; 
	T tz = m23; 
	T w0 = m30;                  
	T w1 = m31;
	T w2 = m32;
	T w3 = m33;

	set(q1);
	mulRotationScale(scale);

	// set back
	m03 = tx;
	m13 = ty;
	m23 = tz;
	m30 = w0;
	m31 = w1;
	m32 = w2;
	m33 = w3;
    }
#endif

    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * equivalent values of the quaternion argument; the other elements of this
      * matrix are unchanged; a singular value decomposition is performed on
      * this object's upper 3x3 matrix to factor out the scale, then this
      * object's upper 3x3 matrix components are replaced by the matrix
      * equivalent of the quaternion, and then the scale is reapplied to the
      * rotational components.
      * @param q1 the quaternion that specifies the rotation
      */
    void setRotation(const Quat4<T>& q1);

    /**
      * Sets the rotational component (upper 3x3) of this matrix to the matrix
      * equivalent values of the axis-angle argument; the other elements of this
      * matrix are unchanged; a singular value decomposition is performed on
      * this object's upper 3x3 matrix to factor out the scale, then this
      * object's upper 3x3 matrix components are replaced by the matrix
      * equivalent of the axis-angle, and then the scale is reapplied to the
      * rotational components.
      * @param a1 the axis-angle to be converted (x, y, z, angle)
      */
    void setRotation(const AxisAngle4<T>& a1);

    /**
      * Sets this matrix to all zeros.
      */
    void setZero();

    /**
      * Negates the value of this matrix: this = -this.
      */
    void negate();

    /**
      * Sets the value of this matrix equal to the negation of of the Matrix4
      * parameter.
      * @param m1 The source matrix
      */
    void negate(const Matrix4& m1) {
        set(m1);
        negate();
    }

    /**
     * Returns a string that contains the values of this Matrix4.
     * @return the String representation
     */
#ifdef VM_INCLUDE_TOSTRING
VM_STRING_STD::string toString() const;
#endif

protected:
    /**
      * Performs SVD on this matrix and gets scale and rotation.
      * Rotation is placed into rot3, and rot4.
      * @param rot3 the rotation factor(Matrix3d). if null, ignored
      * @param rot4 the rotation factor(Matrix4) only upper 3x3 elements are changed. if null, ignored
      * @return scale factor
      */
    T SVD(Matrix3<T>* rot3, Matrix4* rot4) const;

#if 0
    /**
      * Performs SVD on this matrix and gets the scale and the pure rotation.
      * The pure rotation is placed into rot.
      * @param rot the rotation factor.
      * @return scale factor
      */
    private float SVD(Matrix3f rot) {
	// this is a simple svd.
	// Not complete but fast and reasonable.
	// See comment in Matrix3d.

	T s = Math.sqrt(
	    (
	     m00*m00 + m10*m10 + m20*m20 + 
	     m01*m01 + m11*m11 + m21*m21 +
	     m02*m02 + m12*m12 + m22*m22
	    )/3.0
	    );

	// zero-div may occur.
	T t = (s == 0.0 ? 0.0 : 1.0/s);

	if (rot != null) {
	    this.getRotationScale(rot);
	    rot.mul((float)t);
	}

	return (float)s;
    }
#endif

    /**
      * Multiplies 3x3 upper elements of this matrix by a scalar.
      * The other elements are unchanged.
      */
    void mulRotationScale(T scale);

    /**
      * Sets only 3x3 upper elements of this matrix to that of m1.
      * The other elements are unchanged.
      */
    void setRotationScale(const Matrix4& m1);

    /**
     * Sets this matrix from the 4 values of quaternion.
     * @param x q.x
     * @param y q.y
     * @param z q.z
     * @param w q.w
     */
    void setFromQuat(T x, T y, T z, T w);

    /**
     * Sets this matrix from the 4 values of axisAngle.
     * @param x a.x
     * @param y a.y
     * @param z a.z
     * @param angle a.angle
     */
    void setFromAxisAngle(T x, T y, T z, T angle);

#if 0
    /**
      * Modifies the translational components of this matrix to the values of
      * the Vector3f argument; the other values of this matrix are not modified.
      * @param trans the translational component
      */
    private void setTranslation(Vector3f trans) {
        m03 = trans.x;
        m13 = trans.y;  
        m23 = trans.z;
    }
#endif

public:
    // copy constructor and operator = is made by complier

    bool operator==(const Matrix4& m1) const {
        return equals(m1);
    }
#ifdef VM_INCLUDE_SUBSCRIPTION_OPERATOR
    T operator()(size_t row, size_t col) const {
        return getElement(row, col);
    }
    T& operator()(size_t row, size_t col) {
        return getElementReference(row, col);
    }
#endif
    Matrix4& operator+=(const Matrix4& m1) {
        add(m1);
        return *this;
    }
    Matrix4& operator-=(const Matrix4& m1) {
        sub(m1);
        return *this;
    }
    Matrix4& operator*=(const Matrix4& m1) {
        mul(m1);
        return *this;
    }
    Matrix4& operator*=(T s) {
        mul(s);
        return *this;
    }
    Matrix4 operator+(const Matrix4& m1) const {
        return (Matrix4(*this)).operator+=(m1);
    }
    Matrix4 operator-(const Matrix4& m1) const {
        return (Matrix4(*this)).operator-=(m1);
    }
    Matrix4 operator*(const Matrix4& m1) const {
        return (Matrix4(*this)).operator*=(m1);
    }
    Matrix4 operator*(T s) const {
        return (Matrix4(*this)).operator*=(s);
    }

};

template <class T>
inline
Matrix4<T> operator*(T s, const Matrix4<T>& m) {
    return (Matrix4<T>(m)).operator*=(s);
}

template <class T>
inline
Matrix4<T> operator*(const Matrix4<T>& m1, const Matrix4<T>& m2) {
    return (Matrix4<T>(m1)).operator*=(m2);
}

template <class T>
inline
Tuple4<T> operator*(const Matrix4<T>& m, const Tuple4<T>& t) {
    Tuple4<T> out;
    m.transform(t,&out); 
    return out;
}

// bug fixed, 12.11.2002 M.Hastenteufel
// tuple3 -> Tuple4
template <class T>
inline
Vector4<T> operator*(const Matrix4<T>& m, const Vector4<T>& t) {
    return operator*(m, (const Tuple4<T>&)t);
}

// bug fixed, 12.11.2002 M.Hastenteufel
// tuple3 -> Tuple4
template <class T>
inline
Point4<T> operator*(const Matrix4<T>& m, const Point4<T>& t) {
    return operator*(m, (const Tuple4<T>&)t);
}

template <class T>
inline
Vector3<T> operator*(const Matrix4<T>& m, const Vector3<T>& t) {
    Vector3<T> out;
    m.transform(t,&out); 
    return out;
}

template <class T>
inline
Point3<T> operator*(const Matrix4<T>& m, const Point3<T>& t) {
    Point3<T> out;
    m.transform(t,&out); 
    return out;
}

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Matrix4<T>& t1);
#endif

VM_BEGIN_NS

typedef Matrix4<double> Matrix4d;
typedef Matrix4<float> Matrix4f;

VM_END_NS


#endif /* MATRIX4__H */
