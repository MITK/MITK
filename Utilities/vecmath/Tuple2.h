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
#ifndef TUPLE2_H
#define TUPLE2_H

#include "VmUtil.h"

VM_BEGIN_NS

/**
  * A generic 2 element tuple that is represented by
  * point x,y coordinates.
  * @version specification 1.1, implementation $Revision$, $Date$
  * @author Kenji hiranabe
  */
template<class T>
class Tuple2 {
/*
 * $Log$
 * Revision 1.2  2003/04/22 14:42:11  max
 * made inclusions of vecmath header files "local" (for gcc 3.3 support)
 *
 * Revision 1.1  2002/11/16 12:25:36  mark
 * vecmath bibliothek
 * in cvs, da einige bugs behoben
 *
 * Revision 1.4  1999/10/22  08:37:10  hiranabe
 * added operator=. work around for IRIX6.4 + MIPSPro7.2.
 * Thanks > Katsuaki KAWACHI <kawachi@cim.pe.u-tokyo.ac.jp>
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
    enum { DIMENSION = 2 };

    /**
     * The x coordinate.
     */
     T x;

    /**
      * The y coordinate.
      */
    T y;

    /**
      * Constructs and initializes a Tuple from the specified xy coordinates.
      * @param xvalue the x coordinate
      * @param yvalue the y coordinate
      */
    Tuple2(T xvalue, T yvalue): x(xvalue), y(yvalue) { }

    /**
     * Constructs and initializes a Tuple from the specified array.
     * @param t the array of length 2 containing xy in order
     */
    Tuple2(const T t[]): x(t[0]), y(t[1]) { }


#if 0
    /**
      * Constructs and initializes a Tuple2 from the specified Tuple2f.
      * this is special version for float to double or complex
      * @param t1 the Tuple2 containing the initialization x y data
      */
    Tuple2(const Tuple2<float>& t1): x(t1.x), y(t1.y) { }

#endif

    /**
      * Constructs and initializes a Tuple2 to (0,0).
      */
    Tuple2(): x(0), y(0) { }

    /**
     * Sets the value of this tuple to the specified xy coordinates.
     * @param xvalue the x coordinate
     * @param yvalue the y coordinate
     */
    void set(T xvalue, T yvalue) {
        x = xvalue;
        y = yvalue;
    }

    /**
     * Sets the value of this tuple from the 2 values specified in the array.
     * @param t the array of length 2 containing xy in order
     */
    void set(const T t[]) {
        x = t[0];
        y = t[1];
    }

    /**
      * Sets the value of this tuple to the value of the Tuple2 argument.
      * @param t1 the tuple to be copied
      */
    void set(const Tuple2& t1) {
        x = t1.x;
        y = t1.y;
    }

    /**
      * Copies the value of the elements of this tuple into the array t[]. 
      * @param t the array that will contain the values of the vector
      *          asserting t has room for 2 values.
      */
    void get(T t[]) const {
        t[0] = x;
        t[1] = y;
    }

    /**
      * Gets the value of this tuple and copies the values into the Tuple2.
      * @param t Tuple2 object into which that values of this object are copied
      */
    void get(Tuple2* t) const {
        assert(t);
        t->x = x;
        t->y = y;
    }

    /**
     * Sets the value of this tuple to the vector sum of tuples t1 and t2.
     * @param t1 the first tuple
     * @param t2 the second tuple
     */
    void add(const Tuple2& t1, const Tuple2& t2) {
        x = t1.x + t2.x;
        y = t1.y + t2.y;
    }

    /**
     * Sets the value of this tuple to the vector sum of itself and tuple t1.
     * @param t1  the other tuple
     */
    void add(const Tuple2& t1) {
        x += t1.x;
        y += t1.y;
    }

    /**
     * Sets the value of this tuple to the vector difference of tuple t1 and t2 (this = t1 - t2).
     * @param t1 the first tuple
     * @param t2 the second tuple
     */
    void sub(const Tuple2& t1, const Tuple2& t2) {
        x = t1.x - t2.x;
        y = t1.y - t2.y;
    }

    /**
      * Sets the value of this tuple to the vector difference of itself and tuple t1 (this = this - t1).
      * @param t1 the other tuple
      */
    void sub(const Tuple2& t1) {
        x -= t1.x;
        y -= t1.y;
    }

    /**
     * Sets the value of this tuple to the negation of tuple t1. 
     * @param t1 the source vector
     */
    void negate(const Tuple2& t1) {
        x = -t1.x;
        y = -t1.y;
    }

    /**
     * Negates the value of this vector in place.
     */
    void negate() {
        x = -x;
        y = -y;
    }

    /**
     * Sets the value of this tuple to the scalar multiplication of tuple t1.
     * @param s the scalar value
     * @param t1 the source tuple
     */
    void scale(T s, const Tuple2& t1) {
        x = s*t1.x;
        y = s*t1.y;
    }

    /**
     * Sets the value of this tuple to the scalar multiplication of itself.
     * @param s the scalar value
     */
    void scale(T s) {
        x *= s;
        y *= s;
    }

    /**
     * Sets the value of this tuple to the scalar multiplication of tuple t1 and then
     * adds tuple t2 (this = s*t1 + t2).
     * @param s the scalar value
     * @param t1 the tuple to be multipled
     * @param t2 the tuple to be added
     */
    void scaleAdd(T s, const Tuple2& t1, const Tuple2& t2) {
        x = s*t1.x + t2.x;
        y = s*t1.y + t2.y;
    }

    /**
     * Sets the value of this tuple to the scalar multiplication of itself and then
     * adds tuple t1 (this = s*this + t1).
     * @param s the scalar value
     * @param t1 the tuple to be added
     */
    void scaleAdd(T s, const Tuple2& t1) {
        x = s*x + t1.x;
        y = s*y + t1.y;
    }

    /**
     * Returns a hash number based on the data values in this object. 
     * Two different Tuple2 objects with identical data  values
     * (ie, returns true for equals(Tuple2) ) will return the same hash number.
     * Two vectors with different data members may return the same hash value,
     * although this is not likely.
     */
    size_t hashCode() const {
        return VmUtil<T>::hashCode(sizeof *this, this);
    }

    /**
     * Returns true if all of the data members of Tuple2 t1 are equal to the corresponding
     * data members in this
     * @param t1 the vector with which the comparison is made.
     */
    bool equals(const Tuple2& t1) const {
        return t1.x == x && t1.y == y;
    }

    /**
     * Returns true if the L-infinite distance between this tuple and tuple t1 is
     * less than or equal to the epsilon parameter, otherwise returns false. The L-infinite
     * distance is equal to MAX[abs(x1-x2), abs(y1-y2)].
     * @param t1 the tuple to be compared to this tuple
     * @param epsilon the threshold value
     */
    bool epsilonEquals(const Tuple2& t1, T epsilon) const {
        return (VmUtil<T>::abs(t1.x - x) <= epsilon) &&
            (VmUtil<T>::abs(t1.y - y) <= epsilon);
    }

    /**
     * Clamps the tuple parameter to the range [low, high] and places the values
     * into this tuple.
     * @param min the lowest value in the tuple after clamping
     * @param max the highest value in the tuple after clamping
     * @param t the source tuple, which will not be modified
     */
    void clamp(T min, T max, const Tuple2& t) {
        set(t);
        clamp(min, max);
    }
      
    /**
     * Clamps the minimum value of the tuple parameter to the min parameter
     * and places the values into this tuple.
     * @param min the lowest value in the tuple after clamping
     * @parm t the source tuple, which will not be modified
     */
    void clampMin(T min, const Tuple2& t) {
        set(t);
        clampMin(min);
    }

    /**
     * Clamps the maximum value of the tuple parameter to the max parameter and
     * places the values into this tuple.
     * @param max the highest value in the tuple after clamping
     * @param t the source tuple, which will not be modified
     */
    void clampMax(T max, const Tuple2& t) {
        set(t);
        clampMax(max);
    }
      

    /**
     * Sets each component of the tuple parameter to its absolute value and
     * places the modified values into this tuple.
     * @param t the source tuple, which will not be modified
     */
    void absolute(const Tuple2& t) {
        set(t);
        absolute();
    }

    /**
     * Clamps this tuple to the range [low, high].
     * @param min the lowest value in this tuple after clamping
     * @param max the highest value in this tuple after clamping
     */
    void clamp(T min, T max) {
        clampMin(min);
        clampMax(max);
    }

    /**
     * Clamps the minimum value of this tuple to the min parameter.
     * @param min the lowest value in this tuple after clamping
     */
    void clampMin(T min) {
        if (x < min)
            x = min;
        if (y < min)
            y = min;
    }

    /**
     * Clamps the maximum value of this tuple to the max parameter.
     * @param max the highest value in the tuple after clamping
     */
    void clampMax(T max) {
        if (x > max)
            x = max;
        if (y > max)
            y = max;
    }

    /**
     * Sets each component of this tuple to its absolute value.
     */
    void absolute() {
        if (x < 0.0)
            x = -x;
        if (y < 0.0)
            y = -y;
    }

    /**
     * Linearly interpolates between tuples t1 and t2 and places the
     * result into this tuple: this = (1-alpha)*t1 + alpha*t2.
     * @param t1 the first tuple
     * @param t2 the second tuple
     * @param alpha the alpha interpolation parameter
     */
    void interpolate(const Tuple2& t1, const Tuple2& t2, T alpha) {
        set(t1);
        interpolate(t2, alpha);
    }


    /**
     * Linearly interpolates between this tuple and tuple t1 and places the
     * result into this tuple: this = (1-alpha)*this + alpha*t1.
     * @param t1 the first tuple
     * @param alpha the alpha interpolation parameter
     *
     */
    void interpolate(const Tuple2& t1, T alpha) {
        T beta = 1 - alpha;
        x = beta*x + alpha*t1.x;
        y = beta*y + alpha*t1.y;
    }

    /**
     * Returns a string that contains the values of this Tuple2. The form is (x,y).
     * @return the String representation
     */
#ifdef VM_INCLUDE_TOSTRING
VM_STRING_STD::string toString() const {
        VM_TOSTRING
    }
#endif

    // copy constructor and operator = is made by complier

    bool operator==(const Tuple2<T>& t1) const {
        return equals(t1);
    }

#ifdef VM_INCLUDE_SUBSCRIPTION_OPERATOR
    T operator[](size_t index) const {
        assert(index < (size_t)DIMENSION);
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        default:
            // error !
            return 0;
        }
    }
    T& operator[](size_t index) {
        static T dummy;
        assert(index < (size_t)DIMENSION);
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        default:
            // error !
            return dummy;
        }
    }
#endif

    Tuple2& operator=(const Tuple2& t1) {
        set(t1);
        return *this;
    }

    Tuple2& operator+=(const Tuple2& t1) {
        add(t1);
        return *this;
    }
    Tuple2& operator-=(const Tuple2& t1) {
        sub(t1);
        return *this;
    }
    Tuple2& operator*=(T s) {
        scale(s);
        return *this;
    }
    Tuple2 operator+(const Tuple2& t1) const {
        return (Tuple2(*this)).operator+=(t1);
    }
    Tuple2 operator-(const Tuple2<T>& t1) const {
        return (Tuple2(*this)).operator-=(t1);
    }
    Tuple2 operator*(T s) const {
        return (Tuple2(*this)).operator*=(s);
    }
};

VM_END_NS

template <class T>
inline
VM_VECMATH_NS::Tuple2<T> operator*(T s, const VM_VECMATH_NS::Tuple2<T>& t1) {
    return (VM_VECMATH_NS::Tuple2<T>(t1)).operator*=(s);
}

#ifdef VM_INCLUDE_IO
template <class T>
VM_IOSTREAM_STD::ostream& operator<<(VM_IOSTREAM_STD::ostream& o, const VM_VECMATH_NS::Tuple2<T>& t1) {
    return o << "(" << t1.x << "," << t1.y << ")";
}
#endif

VM_BEGIN_NS

typedef Tuple2<double> Tuple2d;
typedef Tuple2<float> Tuple2f;

VM_END_NS


#endif /* TUPLE2_H */
