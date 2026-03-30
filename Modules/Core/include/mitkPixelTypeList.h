/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkPixelTypeList.h
 * \brief Compile-time type list infrastructure for pixel type dispatching in MITK.
 *
 * Provides a variadic-style type list (\c PixelTypeList), compile-time accessors
 * (\c GetPixelType, \c PixelTypeLength), a run-time type switch (\c PixelTypeSwitch),
 * and the \c AccessItkImageFunctor helper used by the AccessByItk macro family.
 *
 * \ingroup Core
 */

#ifndef mitkPixelTypeList_h
#define mitkPixelTypeList_h

#include <stdexcept>

namespace mitk
{
  /**
   * \brief Sentinel type used to mark unused slots in a PixelTypeList.
   */
  struct EmptyType
  {
  };

  /**
   * \brief Compile-time list of pixel types (up to 10 types).
   *
   * Recursively defines a \c head (the first type) and a \c tail (the remaining types
   * as another PixelTypeList). The \c length enum counts the number of non-empty types.
   *
   * \tparam T0..T9 The pixel types stored in this list (unused slots default to EmptyType).
   */
  template <typename T0 = EmptyType,
            typename T1 = EmptyType,
            typename T2 = EmptyType,
            typename T3 = EmptyType,
            typename T4 = EmptyType,
            typename T5 = EmptyType,
            typename T6 = EmptyType,
            typename T7 = EmptyType,
            typename T8 = EmptyType,
            typename T9 = EmptyType>
  struct PixelTypeList;

  template <typename T0,
            typename T1,
            typename T2,
            typename T3,
            typename T4,
            typename T5,
            typename T6,
            typename T7,
            typename T8,
            typename T9>
  struct PixelTypeList
  {
    typedef T0 head;
    typedef PixelTypeList<T1, T2, T3, T4, T5, T6, T7, T8, T9> tail;
    enum
    {
      length = tail::length + 1
    };
  };

  template <>
  struct PixelTypeList<EmptyType,
                       EmptyType,
                       EmptyType,
                       EmptyType,
                       EmptyType,
                       EmptyType,
                       EmptyType,
                       EmptyType,
                       EmptyType,
                       EmptyType>
  {
    enum
    {
      length = 0
    };
  };

  /**
   * \brief Compile-time query for the number of types in a PixelTypeList.
   *
   * \tparam TypeList A PixelTypeList instantiation.
   */
  template <typename TypeList>
  struct PixelTypeLength
  {
    enum
    {
      value = TypeList::length
    };
  };

  /**
   * \brief Compile-time accessor that retrieves the type at position \a Index in a PixelTypeList.
   *
   * Recursively walks the list until the requested index is reached.
   * A compile-time error occurs if \a Index is out of range.
   *
   * \tparam TypeList   A PixelTypeList instantiation.
   * \tparam Index      Zero-based index of the desired type.
   * \tparam Step       Current recursion depth (internal, do not specify).
   * \tparam Stop       Recursion stop flag (internal, do not specify).
   * \tparam OutOfRange Out-of-range flag (internal, do not specify).
   */
  template <typename TypeList,
            int Index,                                              // requested element index
            int Step = 0,                                           // current recursion step
            bool Stop = (Index == Step),                            // stop recursion flag
            bool OutOfRange = PixelTypeLength<TypeList>::value == 0 // out of range flag
            >
  struct GetPixelType
  {
    typedef typename GetPixelType<typename TypeList::tail, Index, Step + 1>::type type;
  };

  //"out of range" specialization
  template <typename TypeList, int Index, int Step, bool Stop>
  struct GetPixelType<TypeList, Index, Step, Stop, true>
  {
    // if OutOfRange is 'true' the 'type' is undefined
    // so we'll get a compile-time error
  };

  //"element found" specialization
  template <typename TypeList, int Index, int Step, bool OutOfRange>
  struct GetPixelType<TypeList, Index, Step, true, OutOfRange>
  {
    // the index is equal to the recursion step
    // so the result type is the head of the Typlist and stop!
    typedef typename TypeList::head type;
  };

  ////////////////////////////////////////////////////////////
  // run-time type switch

  /**
   * \brief Run-time type switch over a PixelTypeList.
   *
   * Given a run-time integer index \a i, invokes functor \a f with the corresponding
   * compile-time type from the type list. Returns the boolean result of the functor.
   *
   * \tparam TypeList A PixelTypeList instantiation.
   * \tparam Index    Current recursion index (internal, do not specify).
   * \tparam Stop     Recursion stop flag (internal, do not specify).
   *
   * \throw std::out_of_range If \a i is not a valid index in the type list.
   */
  template <typename TypeList, int Index = 0, bool Stop = (Index == PixelTypeLength<TypeList>::value)>
  struct PixelTypeSwitch;

  template <typename TypeList, int Index, bool Stop>
  struct PixelTypeSwitch
  {
    template <typename F>
    bool operator()(int i, F &f)
    {
      if (i == Index)
      {
        return f.operator()<typename GetPixelType<TypeList, Index>::type>();
      }
      else
      {
        PixelTypeSwitch<TypeList, Index + 1> next;
        return next(i, f);
      }
    }
  };

  template <typename TypeList, int Index>
  struct PixelTypeSwitch<TypeList, Index, true>
  {
    template <typename F>
    bool operator()(int, F &)
    {
      throw std::out_of_range("Index out of range");
    }
  };

  /**
   * \brief Functor that converts a mitk::Image to an itk::Image and invokes a callback.
   *
   * Used internally by the AccessByItk macro family. Checks that the image's pixel type
   * and dimension match the template parameters, then performs the conversion via
   * mitk::ImageToItk and calls the provided callback.
   *
   * \tparam X          The class type on which the callback is a member function pointer.
   * \tparam VDimension The expected image dimension.
   * \tparam T1         Type of the first extra callback argument (or EmptyType).
   * \tparam T2         Type of the second extra callback argument (or EmptyType).
   * \tparam T3         Type of the third extra callback argument (or EmptyType).
   */
  template <typename X, int VDimension, typename T1 = EmptyType, typename T2 = EmptyType, typename T3 = EmptyType>
  struct AccessItkImageFunctor
  {
    typedef void (*CallBack)(T1, T2, T3);

    AccessItkImageFunctor(
      X *cl, CallBack callBack, const mitk::Image *mitkImage, T1 t1 = T1(), T2 t2 = T2(), T3 t3 = T3())
      : cl(cl), callBack(callBack), mitkImage(mitkImage), pixelType(mitkImage->GetPixelType()), t1(t1), t2(t2), t3(t3)
    {
    }

    template <typename PixelType>
    bool operator()()
    {
      if (pixelType != typeid(PixelType))
        return false;
      if (mitkImage->GetDimension() != VDimension)
        return false;

      const_cast<mitk::Image *>(mitkImage)->Update();

      typedef itk::Image<PixelType, VDimension> ImageType;
      typedef mitk::ImageToItk<ImageType> ImageToItkType;
      itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();
      imagetoitk->SetInput(mitkImage);
      imagetoitk->Update();
      cl->*callBack(imagetoitk->GetOutput(), t1, t2, t3);
      return true;
    }

  private:
    X *cl;
    CallBack callBack;

    const mitk::Image *mitkImage;
    const mitk::PixelType &pixelType;
    T1 t1;
    T2 t2;
    T3 t3;
  };

}

#endif
