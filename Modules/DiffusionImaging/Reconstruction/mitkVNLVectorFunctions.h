/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkVNLVectorFunctions_h_
#define __mitkVNLVectorFunctions_h_

#include "MitkDiffusionImagingExports.h"
#include "vnl/vnl_vector.h"

namespace mitk
{

namespace mitk_vnl_function
{

template<class T>
vnl_vector<T> element_abs (vnl_vector<T> const& v1)
{
  vnl_vector<T> result(v1.size());

  for(int i = 0 ; i < v1.size(); i++)
       result[i] = std::fabs(v1[i]);

  return result;
}

template<class CurrentValue, class WntValue>
vnl_vector<WntValue> element_cast (vnl_vector<CurrentValue> const& v1)
{
  vnl_vector<WntValue> result(v1.size());

  for(int i = 0 ; i < v1.size(); i++)
       result[i] = static_cast<WntValue>(v1[i]);

  return result;
}


template<class T>
vnl_vector<T> element_pow (vnl_vector<T> const& v1, int p)
{
  vnl_vector<T> result(v1.size());

  for(int i = 0 ; i < v1.size(); i++)
       result[i] = std::pow(v1[i], p);

  return result;
}

template<class T>
vnl_vector<T> element_condition_smallerThan (vnl_vector<T> const& v1, vnl_vector<T> const& v2, vnl_vector<T> const& v3)
{

#ifndef NDEBUG
   if (v1.size() != v2.size())
     vnl_error_vector_dimension ("element_condition_smaller: condition value", v1.size(), v2.size());
   if (v1.size() != v3.size())
     vnl_error_vector_dimension ("element_condition_smaller: accept value", v1.size(), v3.size());
#endif

  vnl_vector<T> result(v1.size());

  for(int i = 0 ; i < v1.size(); i++)
    v1[i] < v2[i] ? result[i] =  v3[i] : result[i] = 0;

  return result;
}

template<class T>
vnl_vector<T> element_condition_smallerThanEqual (vnl_vector<T> const& v1, vnl_vector<T> const& v2, vnl_vector<T> const& v3)
{

#ifndef NDEBUG
   if (v1.size() != v2.size())
     vnl_error_vector_dimension ("element_condition_smallerThanEqual: condition value", v1.size(), v2.size());
   if (v1.size() != v3.size())
     vnl_error_vector_dimension ("element_condition_smallerThanEqual: accept value", v1.size(), v3.size());
#endif

  vnl_vector<T> result(v1.size());

  for(int i = 0 ; i < v1.size(); i++)
    v1[i] <= v2[i] ? result[i] =  v3[i] : result[i] = 0;

  return result;
}

template<class T>
vnl_vector<T> element_condition_greaterThan (vnl_vector<T> const& v1, vnl_vector<T> const& v2, vnl_vector<T> const& v3)
{

#ifndef NDEBUG
   if (v1.size() != v2.size())
     vnl_error_vector_dimension ("element_condition_greaterThan: condition value", v1.size(), v2.size());
   if (v1.size() != v3.size())
     vnl_error_vector_dimension ("element_condition_greaterThan: accept value", v1.size(), v3.size());
#endif

  vnl_vector<T> result(v1.size());

  for(int i = 0 ; i < v1.size(); i++)
    v1[i] > v2[i] ? result[i] =  v3[i] : result[i] = 0;

  return result;
}

template<class T>
vnl_vector<T> element_condition_greaterThanEqual (vnl_vector<T> const& v1, vnl_vector<T> const& v2, vnl_vector<T> const& v3)
{

#ifndef NDEBUG
   if (v1.size() != v2.size())
     vnl_error_vector_dimension ("element_condition_greaterThanEqual: condition value", v1.size(), v2.size());
   if (v1.size() != v3.size())
     vnl_error_vector_dimension ("element_condition_greaterThanEqual: accept value", v1.size(), v3.size());
#endif

  vnl_vector<T> result(v1.size());

  for(int i = 0 ; i < v1.size(); i++)
    v1[i] >= v2[i] ? result[i] =  v3[i] : result[i] = 0;

  return result;
}


}
}

#endif //__mitkVNLVectorFunctions_h_

