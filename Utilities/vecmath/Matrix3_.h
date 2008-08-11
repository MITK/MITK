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
#ifndef MATRIX3__H
#define MATRIX3__H

#include "VmUtil.h"
#include "Point3.h"
#include "Vector3.h"

VM_BEGIN_NS

template<class T> class Quat4;
template<class T> class AxisAngle4;

/**
 * A 3 x 3 matrix.
 * Primarily to support rotations
 * @version specification 1.1, implementation $Revision$, $Date$
 * @author Kenji hiranabe
 */
template<class T>
class Matrix3 {
protected:
    static T abs(T t) { return VmUtil<T>::abs(t); }
    static T sin(T t) { return VmUtil<T>::sin(t); }
    static T cos(T t) { return VmUtil<T>::cos(t); }
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
    enum { DIMENSION = 3 };
    /**
     * the type for tuple
     */
    typedef Tuple3<T> tuple_type;
    /**
     * the type for vector
     */
    typedef Vector3<T> vector_type;
    /**
     * the type for point
     */
    typedef Point3<T> point_type;

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
      * Constrcts and initializes a Matrix3 from the specified nine values.
      * @param m00 the [0][0] element
      * @param m01 the [0][1] element
      * @param m02 the [0][2] element
      * @param m10 the [1][0] element
      * @param m11 the [1][1] element
      * @param m12 the [1][2] element
      * @param m20 the [2][0] element
      * @param m21 the [2][1] element
      * @param m22 the [2][2] element
      */
    Matrix3(T m00, T m01, T m02,
            T m10, T m11, T m12,
            T m20, T m21, T m22);

    /**
      * Constructs and initializes a Matrix3 from the specified 9
      * element array.  this.m00 =v[0], this.m01=v[1], etc.
      * @param  v the array of length 9 containing in order
      */
    Matrix3(const T v[]);

    /**
      * Constructs and initializes a Matrix3 from the specified 3x3
      * element array.  this.m00 =m[0][0], this.m01=m[0][1], etc.
      * @param  m the array of 3 x 3 containing in order
      */
#ifdef VM_INCLUDE_CONVERSION_FROM_2DARRAY
    Matrix3(const T m[][3]);
#endif

#if 0
    /**
      * Constructs a new matrix with the same values as the Matrix3f parameter.
      * @param m1 The source matrix.
      */
    Matrix3(Matrix3f m1): 
        m00(m1.m00), m01(m1.m01), m02(m1.m02),
        m10(m1.m10), m11(m1.m11), m12(m1.m12),
        m20(m1.m20), m21(m1.m21), m22(m1.m22) { }
#endif

    /**
      * Constructs and initializes a Matrix3 to all zeros.
      */
    Matrix3();

    /**
      * Sets 9 values	
      * @param m00 the [0][0] element
      * @param m01 the [0][1] element
      * @param m02 the [0][2] element
      * @param m10 the [1][0] element
      * @param m11 the [1][1] element
      * @param m12 the [1][2] element
      * @param m20 the [2][0] element
      * @param m21 the [2][1] element
      * @param m22 the [2][2] element
      */
    void set(T m00, T m01, T m02,
             T m10, T m11, T m12,
             T m20, T m21, T m22);

    /**
      * Sets the value of this matrix to the value of the Matrix3
      * argument.
      * @param m1 The source matrix.
      */
    void set(const Matrix3& m1);

    /**
      * Sets the values in this Matrix3 equal to the row-major array parameter
      * (ie, the first four elements of the array will be copied into the first
      * row of this matrix, etc.).
      * @param  m the array of length 9 containing in order
      */
    void set(const T m[]);

    /**
      * Sets the values in this Matrix3 equal to the row-major array parameter
      * (ie, the first four elements of the array will be copied into the first
      * row of this matrix, etc.).
      * @param  m the array of 3x3 containing in order (T m[3][3]) 
      */
#ifdef VM_INCLUDE_CONVERSION_FROM_2DARRAY
    void set(const T m[][3]);
#endif

    /**
     * Sets this Matrix3 to identity.
     */
    void setIdentity();

    /**
      * Sets the scale component of the current matrix by factoring out the
      * current scale (by doing an SVD) from the rotational component and
      * multiplying by the new scale.
      * @param scale the new scale amount
      */
    void setScale(T scale);

    /**
     * Sets the specified element of this matrix3d to the value provided.
     * @param row  the row number to be modified (zero indexed)
     * @param column  the column number to be modified (zero indexed)
     * @param value the new value
     */
    void setElement(size_type row, size_type column, T value);

    /**
     * Retrieves the value at the specified row and column of this matrix.
     * @param row  the row number to be retrieved (zero indexed)
     * @param column  the column number to be retrieved (zero indexed)
     * @return the value at the indexed element
     */
    T getElement(size_type row, size_type column) const;

    /**
     * Retrieves the lvalue at the specified row and column of this matrix.
     * @param row  the row number to be retrieved (zero indexed)
     * @param column  the column number to be retrieved (zero indexed)
     * @return the lvalue at the indexed element
     */
    T& getElementReference(size_type row, size_type column);

    /**
     * Sets the specified row of this matrix3d to the three values provided.
     * @param row  the row number to be modified (zero indexed)
     * @param x the first column element
     * @param y the second column element
     * @param z the third column element
     */
    void setRow(size_type row, T x, T y, T z);

    /**
     * Sets the specified row of this matrix3d to the Vector provided.
     * @param row the row number to be modified (zero indexed)
     * @param v the replacement row
     */
    void setRow(size_type row, const Vector3<T>& v);

    /**
      * Sets the specified row of this matrix3 to the four values provided.
      * @param row the row number to be modified (zero indexed)
      * @param v the replacement row
      */
    void setRow(size_type row, const T v[]);

    /**
      * Copies the matrix values in the specified row into the
      * array parameter.
      * @param row the matrix row
      * @param v The array into which the matrix row values will be copied
      */
    void getRow(size_type row, T v[]) const;

    /**
      * Copies the matrix values in the specified row into the
      * vector parameter.
      * @param row the matrix row
      * @param v The vector into which the matrix row values will be copied
      */
    void getRow(size_type row, Vector3<T>* v) const;

    /**
      * Sets the specified column of this matrix3 to the three values provided.
      * @param  column the column number to be modified (zero indexed)
      * @param x the first row element
      * @param y the second row element
      * @param z the third row element
      */
    void setColumn(size_type column, T x, T y, T z);

    /**
      * Sets the specified column of this matrix3d to the vector provided.
      * @param column the column number to be modified (zero indexed)
      * @param v the replacement column
      */
    void setColumn(size_type column, const Vector3<T>& v);

    /**
      * Sets the specified column of this matrix3d to the four values provided. 
      * @param column  the column number to be modified (zero indexed) 
      * @param v       the replacement column 
      */
    void setColumn(size_type column,  const T v[]);

    /**
      * Copies the matrix values in the specified column into the vector 
      * parameter.
      * @param column the matrix column
      * @param v The vector into which the matrix row values will be copied
      */
    void getColumn(size_type column, Vector3<T>* v) const;

    /**
      * Copies the matrix values in the specified column into the array
      * parameter.
      * @param column the matrix column
      * @param v The array into which the matrix row values will be copied
      */
    void getColumn(size_type column,  T v[]) const;

    /**
      * Performs an SVD normalization of this matrix to calculate and return the
      * uniform scale factor. This matrix is not modified.
      * @return the scale factor of this matrix
      */
    T getScale() const;

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
      * note this method is alias-safe.
      * @param scalar The scalar adder.
      * @param m1 The original matrix values.
      */
    void add(T scalar, const Matrix3& m1);

    /**
     * Sets the value of this matrix to the matrix sum of matrices m1 and m2. 
     * note this method is alias-safe.
     * @param m1 the first matrix 
     * @param m2 the second matrix 
     */
    void add(const Matrix3& m1, const Matrix3& m2);

    /**
     * Sets the value of this matrix to sum of itself and matrix m1. 
     * @param m1 the other matrix 
     */
    void add(const Matrix3& m1);

    /**
      * Sets the value of this matrix to the matrix difference
      * of matrices m1 and m2. 
      * note this method is alias-safe.
      * @param m1 the first matrix 
      * @param m2 the second matrix 
      */
    void sub(const Matrix3& m1, const Matrix3& m2);

    /**
     * Sets the value of this matrix to the matrix difference of itself
     * and matrix m1 (this = this - m1). 
     * @param m1 the other matrix 
     */
    void sub(const Matrix3& m1);

    /**
      * Sets the value of this matrix to its transpose. 
      */
    void transpose();

    /**
     * Sets the value of this matrix to the transpose of the argument matrix.
     * note this method is alias-safe
     * @param m1 the matrix to be transposed 
     */
    void transpose(const Matrix3& m1);

    /**
      * Sets the value of this matrix to the matrix conversion of the
      * quaternion argument. 
      * @param q1 the quaternion to be converted 
      */
    void set(const Quat4<T>& q1);   // moved to the implementation file

    /**
      * Sets the value of this matrix to the matrix conversion of the
      * axis and angle argument. 
      * @param a1 the axis and angle to be converted 
      */
    void set(const AxisAngle4<T>& a1);    // moved to the implementation file

#if 0
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
    /**
      * Sets the value of this matrix to the double value of the Matrix3f
      * argument.
      * @param m1 the matrix3f to be converted to double
      */
    void set(Matrix3f m1)  {
        m00 = m1.m00; m01 = m1.m01; m02 = m1.m02;
        m10 = m1.m10; m11 = m1.m11; m12 = m1.m12;
        m20 = m1.m20; m21 = m1.m21; m22 = m1.m22;
    }

#endif

    /**
     * Sets the value of this matrix to the matrix inverse
     * of the passed matrix m1. 
     * @param m1 the matrix to be inverted 
     */
    void invert(const Matrix3& m1);

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
    void mul(T scalar, const Matrix3& m1);

    /**
     * Sets the value of this matrix to the result of multiplying itself
     * with matrix m1. 
     * @param m1 the other matrix 
     */
    void mul(const Matrix3& m1);

    /**
     * Sets the value of this matrix to the result of multiplying
     * the two argument matrices together. 
     * note this method is alias-safe.
     * @param m1 the first matrix 
     * @param m2 the second matrix 
     */
    void mul(const Matrix3& m1, const Matrix3& m2);

    /**
      * Multiplies this matrix by matrix m1, does an SVD normalization of the
      * result, and places the result back into this matrix this =
      * SVDnorm(this*m1).
      * @param m1 the matrix on the right hand side of the multiplication
      */
    void mulNormalize(const Matrix3& m1) {
        mul(m1);
        SVD(this);
    }


    /**
      * Multiplies matrix m1 by matrix m2, does an SVD normalization of the
      * result, and places the result into this matrix this = SVDnorm(m1*m2).
      * @param m1  the matrix on the left hand side of the multiplication
      * @param m2  the matrix on the right hand side of the multiplication
      */
    void mulNormalize(const Matrix3& m1, const Matrix3& m2) {
        mul(m1, m2);
        SVD(this);
    }

    /**
      * Multiplies the transpose of matrix m1 times the transpose of matrix m2,
      * and places the result into this.
      * @param m1 The matrix on the left hand side of the multiplication
      * @param m2 The matrix on the right hand side of the multiplication
      */
    void mulTransposeBoth(const Matrix3& m1, const Matrix3& m2) {
        mul(m2, m1);
        transpose();
    }

    /**
      * Multiplies matrix m1 times the transpose of matrix m2, and places the
      * result into this.
      * @param m1 The matrix on the left hand side of the multiplication
      * @param m2 The matrix on the right hand side of the multiplication
      */
    void mulTransposeRight(const Matrix3& m1, const Matrix3& m2);
    
    /**
      * Multiplies the transpose of matrix m1 times matrix m2, and places the
      * result into this.
      * @param m1 The matrix on the left hand side of the multiplication
      * @param m2 The matrix on the right hand side of the multiplication
      */
    void mulTransposeLeft(const Matrix3& m1, const Matrix3& m2);

    /**
      * Performs singular value decomposition normalization of this matrix.
      */
    void normalize() {
        SVD(this);
    }

    /**
      * Perform singular value decomposition normalization of matrix m1 and
      * place the normalized values into this.
      * @param m1 Provides the matrix values to be normalized
      */
    void normalize(const Matrix3& m1) {
        set(m1);
        SVD(this);
    }

    /**
      * Perform cross product normalization of this matrix.
      */
    void normalizeCP() {
        T s = VmUtil<T>::pow(VmUtil<T>::abs(determinant()), T(-1.0/3.0));
        mul(s);
    }
      
    /**
      * Perform cross product normalization of matrix m1 and place the
      * normalized values into this.
      * @param m1 Provides the matrix values to be normalized
      */
    void normalizeCP(const Matrix3& m1) {
        set(m1);
        normalizeCP();
    }



    /**
     * Returns true if all of the data members of Matrix3 m1 are
     * equal to the corresponding data members in this Matrix3. 
     * @param m1 The matrix with which the comparison is made. 
     * @return true or false 
     */
    bool equals(const Matrix3& m1) const;

    /**
      * Returns true if the L-infinite distance between this matrix and matrix
      * m1 is less than or equal to the epsilon parameter, otherwise returns
      * false. The L-infinite distance is equal to MAX[i=0,1,2,3 ; j=0,1,2,3 ;
      * abs(this.m(i,j) - m1.m(i,j)]
      * @param m1 The matrix to be compared to this matrix
      * @param epsilon the threshold value
      */
    bool epsilonEquals(const Matrix3& m1, T epsilon) const;

    /**
     * Sets this matrix to all zeros. 
     */
    void setZero();

    /**
      * Negates the value of this matrix: this = -this.
      */
    void negate();

    /**
      * Sets the value of this matrix equal to the negation of of the Matrix3
      * parameter.
      * @param m1 The source matrix
      */
    void negate(const Matrix3& m1) {
        set(m1);
        negate();
    }

    /**
     * Transform the vector vec using this Matrix3 and place the
     * result back into vec.
     * @param t the vector to be transformed
     */
    void transform(Tuple3<T>* t) const {
        transform(*t, t);
    }

    /**
     * Transform the vector vec using this Matrix3 and place the
     * result into vecOut.
     * note this method is alias-safe
     * @param t the double precision vector to be transformed
     * @param result the vector into which the transformed values are placed
     */
    void transform(const Tuple3<T>& t, Tuple3<T>* result) const;

    /**
     * Returns a hash number based on the data values in this
     * object.  Two different Matrix3 objects with identical data values
     * (ie, returns true for equals(Matrix3) ) will return the same hash
     * number.  Two objects with different data members may return the
     * same hash value, although this is not likely.
     * @return the integer hash value 
     */
    size_t hashCode() const {
        return VmUtil<T>::hashCode(sizeof *this, this);
    }

  /**
    * Returns a string that contains the values of this Matrix3.
    * @return the String representation
    */
#ifdef VM_INCLUDE_TOSTRING
VM_STRING_STD::string toString() const;
#endif

protected:
    /**
      * Performs SVD on this matrix and gets scale and rotation.
      * Rotation is placed into rot.
      * @param rot the rotation factor. if null, ignored
      * @return scale factor
      */
    T SVD(Matrix3* rot) const;

    /**
     * Sets this from a Quat4 elements
     */
    void setFromQuat(T x, T y, T z, T w);

    /**
     * Sets this from a Quat4 elements
     */
    void setFromAxisAngle(T x, T y, T z, T angle);

public:
    // copy constructor and operator = is made by complier

    bool operator==(const Matrix3& m1) const {
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

    Matrix3& operator+=(const Matrix3& m1) {
        add(m1);
        return *this;
    }
    Matrix3& operator-=(const Matrix3& m1) {
        sub(m1);
        return *this;
    }
    Matrix3& operator*=(const Matrix3& m1) {
        mul(m1);
        return *this;
    }
    Matrix3& operator*=(T s) {
        mul(s);
        return *this;
    }
    Matrix3 operator+(const Matrix3& m1) const {
        return (Matrix3(*this)).operator+=(m1);
    }
    Matrix3 operator-(const Matrix3& m1) const {
        return (Matrix3(*this)).operator-=(m1);
    }
    Matrix3 operator*(const Matrix3& m1) const {
        return (Matrix3(*this)).operator*=(m1);
    }
    Matrix3 operator*(T s) const {
        return (Matrix3(*this)).operator*=(s);
    }

};

template <class T>
inline
Matrix3<T> operator*(T s, const Matrix3<T>& m) {
    return (Matrix3<T>(m)).operator*=(s);
}

template <class T>
inline
Matrix3<T> operator*(const Matrix3<T>& m1, const Matrix3<T>& m2) {
    return (Matrix3<T>(m1)).operator*=(m2);
}

template <class T>
inline
Tuple3<T> operator*(const Matrix3<T>& m, const Tuple3<T>& t) {
    Tuple3<T> out;
    m.transform(t,&out); 
    return out;
}

template <class T>
inline
Vector3<T> operator*(const Matrix3<T>& m, const Vector3<T>& t) {
    return operator*(m, (const Tuple3<T>&)t);
}

template <class T>
inline
Point3<T> operator*(const Matrix3<T>& m, const Point3<T>& t) {
    return operator*(m, (const Tuple3<T>&)t);
}

VM_END_NS

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Matrix3<T>& t1);
#endif

VM_BEGIN_NS

typedef Matrix3<double> Matrix3d;
typedef Matrix3<float> Matrix3f;

VM_END_NS

#endif /* MATRIX3__H */
