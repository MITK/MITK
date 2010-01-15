/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef CHERRYCALLHISTORY_H_
#define CHERRYCALLHISTORY_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include <Poco/Exception.h>

#include <vector>
#include <ostream>

namespace cherry {

/**
 * <code>CallHistory</code> is used to record the invocation
 * of methods within a target object.  This is useful during
 * lifecycle testing for an object.
 * <p>
 * To use <code>CallHistory</code> ..
 * <ol>
 * <li>Create a CallHistory in the target or pass one in.</li>
 * <li>Invoke some test scenario.  </li>
 * <li>If a method is called on the target record the invocation
 *  in the call history</li>
 * <li>Verify the call history after the test scenario is
 *  complete.</li>
 * </ol>
 * </p><p>
 * Each <code>CallHistory</code> has a target which is used to
 * verify the method names passed to the history.  If an invalid
 * name is passed an <code>IllegalArgumentException</code> will
 * be thrown.
 * </p>
 */
class CallHistory : public Object {

private:

  std::vector<std::string> methodList;

  // Class classType;

public:

  cherryObjectMacro(CallHistory)

    /**
     * Creates a new call history for an object.
     *
     * @param target the call history target.
     */
    CallHistory(/*Object target*/);

    /**
     * Adds a method name to the call history.
     *
     * @param methodName the name of a method
     */
    void Add(const std::string& methodName);

    /**
     * Clears the call history.
     */
    void Clear();

    /**
     * Returns whether a list of methods have been called in
     * order.
     *
     * @param testNames an array of the method names in the order they are expected
     * @return <code>true</code> if the methods were called in order
     */
    bool VerifyOrder(const std::vector<std::string>& testNames) const
            throw(Poco::InvalidArgumentException);

    /**
     * Returns whether a method has been called.
     *
     * @param methodName a method name
     * @return <code>true</code> if the method was called
     */
    bool Contains(const std::string& methodName) const ;

    /**
     * Returns whether a list of methods were called.
     *
     * @param methodNames a list of methods
     * @return <code>true</code> if the methods were called
     */
    bool Contains(const std::vector<std::string>& methodNames) const;

    /**
     * Returns whether the list of methods called is empty.
     *
     * @return <code>true</code> iff the list of methods is empty
     */
    bool IsEmpty() const;

    /**
     * Prints the call history to the console.
     */
    void PrintTo(std::ostream& out) const;

private:

    /**
     * Throws an exception if the method name is invalid
     * for the given target class.
     */
    void TestMethodName(const std::string& methodName) const throw(Poco::InvalidArgumentException);

};

}

#endif /* CHERRYCALLHISTORY_H_ */
