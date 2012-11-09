/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryProperty.h"

#include <Poco/Hash.h>
#include <Poco/Bugcheck.h>

namespace berry {

Property::Property(Object::Pointer type,
     const std::string& namespaze, const std::string& name)
 : fType(type), fNamespace(namespaze), fName(name) {

 }

 void
 Property::SetPropertyTester(IPropertyTester::Pointer tester) {
   poco_check_ptr(tester);

   fTester= tester;
 }

 bool
 Property::IsInstantiated() {
   return fTester->IsInstantiated();
 }

 bool
 Property::IsDeclaringPluginActive() {
   return fTester->IsDeclaringPluginActive();
 }

 bool
 Property::IsValidCacheEntry(bool forcePluginActivation) {
   if (forcePluginActivation) {
     return this->IsInstantiated() && this->IsDeclaringPluginActive();
   } else {
     return  (this->IsInstantiated() && this->IsDeclaringPluginActive()) ||
         (!this->IsInstantiated() && !this->IsDeclaringPluginActive());
   }
 }

 bool
 Property::Test(Object::Pointer receiver, std::vector<Object::Pointer>& args, Object::Pointer expectedValue) {
   return fTester->Test(receiver, fName, args, expectedValue);
 }

 bool
 Property::operator==(Property& obj) {
   return fType == obj.fType && fNamespace == obj.fNamespace &&
           fName == obj.fName;
 }

 bool
 Property::operator==(Property* obj) {
   return this->operator==(*obj);
 }

 int
 Property::HashCode() {
   return (int) ((Poco::Hash<std::string>()(typeid(fType).name()) << 16) |
           (Poco::Hash<std::string>()(fNamespace) << 8) |
           Poco::Hash<std::string>()(fName));
 }

}
