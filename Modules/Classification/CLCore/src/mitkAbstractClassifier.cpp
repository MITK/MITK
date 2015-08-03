/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkAbstractClassifier.h>


void mitk::AbstractClassifier::MethodForBuild()
{
    //class A
    //void F1 (print "1")
    //virtual void F2 (print "1")

    //class B : A
    //void F1 (print "2")
    //void F2 (print "2")


    //A* var = new B;
    //B* var2 = new B;
    //A->F1()  --> 1
    //B->F1()  --> 2

 //   A->F2()  --> 2  // schau in dem Objekt welcher Typ vorhanden ist.
}