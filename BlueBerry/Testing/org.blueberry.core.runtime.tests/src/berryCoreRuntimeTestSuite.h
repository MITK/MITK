/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date: 2009-09-07 12:05:58 +0200 (Mo, 07 Sep 2009) $
 Version:   $Revision: 18832 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef BERRYCORERUNTIMETESTSUITE_H_
#define BERRYCORERUNTIMETESTSUITE_H_

#include <CppUnit/TestSuite.h>

namespace berry {

class CoreRuntimeTestSuite : public CppUnit::TestSuite
{
public:

  CoreRuntimeTestSuite();
};

}

#endif /* BERRYCORERUNTIMETESTSUITE_H_ */
