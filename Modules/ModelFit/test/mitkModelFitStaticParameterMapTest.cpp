/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>
#include "mitkTestingMacros.h"

#include "mitkModelFitStaticParameterMap.h"
#include "mitkModelFitException.h"

int mitkModelFitStaticParameterMapTest(int  /*argc*/, char*[] /*argv[]*/)
{
	MITK_TEST_BEGIN("mitkModelFitStaticParameterMapTest")

	mitk::modelFit::StaticParameterMap map;
	mitk::modelFit::StaticParameterMap::ValueType list;
	list.push_back(0);
	map.Add("test", list);
	MITK_TEST_CONDITION_REQUIRED(map.Get("test").size() == 1 &&
								 map.Get("test").front() == 0,
								 "Testing if Add successfully adds a new list.");

	map.Add("test", list);
	MITK_TEST_CONDITION_REQUIRED(map.Get("test").size() == 1 &&
								 map.Get("test")[0] == 0,
								 "Testing if Add doesn't add to an existing list.");

	MITK_TEST_FOR_EXCEPTION(std::range_error, map.Get(""));

	mitk::modelFit::StaticParameterMap::ValueType list2;
	list2.push_back(3);
	map.Add("abc", list2);
	MITK_TEST_CONDITION_REQUIRED(map.Get("test").size() == 1 &&
								 map.Get("test").front() == 0 &&
								 map.Get("abc").size() == 1 &&
								 map.Get("abc").front() == 3,
								 "Testing if Get returns the correct list.");

	list.push_back(1);
	map.Add("def", list);
	list.push_back(2);
	// Adding a list with 3 values when there's already a list with two values
	MITK_TEST_FOR_EXCEPTION(mitk::modelFit::ModelFitException, map.Add("ghi", list));

	map.Clear();
	MITK_TEST_FOR_EXCEPTION(std::range_error, map.Get("test"));

	list2.push_back(5);
	list2.push_back(2);
	list2.push_back(4);
	list2.push_back(1);
	mitk::modelFit::StaticParameterMap::ValueType list3;
	list3.push_back(3);
	list3.push_back(1);
	list3.push_back(4);
	list3.push_back(2);
	list3.push_back(5);
	map.Add("abc", list2);
	map.Add("def", list3);
	map.Sort();
	MITK_TEST_CONDITION_REQUIRED(map.Get("abc")[0] == 1 &&
								 map.Get("abc")[1] == 2 &&
								 map.Get("abc")[2] == 3 &&
								 map.Get("abc")[3] == 4 &&
								 map.Get("abc")[4] == 5 &&
								 map.Get("def")[0] == 5 &&
								 map.Get("def")[1] == 4 &&
								 map.Get("def")[2] == 3 &&
								 map.Get("def")[3] == 2 &&
								 map.Get("def")[4] == 1,
								 "Testing if Sort successfully sorts by the first key");

	map.Sort("def");
	MITK_TEST_CONDITION_REQUIRED(map.Get("abc")[0] == 5 &&
								 map.Get("abc")[1] == 4 &&
								 map.Get("abc")[2] == 3 &&
								 map.Get("abc")[3] == 2 &&
								 map.Get("abc")[4] == 1 &&
								 map.Get("def")[0] == 1 &&
								 map.Get("def")[1] == 2 &&
								 map.Get("def")[2] == 3 &&
								 map.Get("def")[3] == 4 &&
								 map.Get("def")[4] == 5,
								 "Testing if Sort successfully sorts by the given key");

	mitk::modelFit::StaticParameterMap::ValueType list4;
	list4.push_back(0);
	map.Clear();
	map.Add("ghi", list4);
	map.Add("abc", list2);
	map.Add("def", list3);

	map.Sort();
	MITK_TEST_CONDITION_REQUIRED(map.Get("ghi")[0] == 0 &&
								 map.Get("abc")[0] == 1 &&
								 map.Get("abc")[1] == 2 &&
								 map.Get("abc")[2] == 3 &&
								 map.Get("abc")[3] == 4 &&
								 map.Get("abc")[4] == 5 &&
								 map.Get("def")[0] == 5 &&
								 map.Get("def")[1] == 4 &&
								 map.Get("def")[2] == 3 &&
								 map.Get("def")[3] == 2 &&
								 map.Get("def")[4] == 1,
								 "Testing if Sort successfully sorts if the map includes " <<
								 "one-value-lists");

	MITK_TEST_END()
}
