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


#ifndef CHERRYISIZEPROVIDER_H_
#define CHERRYISIZEPROVIDER_H_

#include "cherryUiDll.h"

namespace cherry {

/**
 * Interface implemented by objects that are capable of computing
 * a preferred size
 *
 * @since 3.1
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI ISizeProvider {

    /**
     * Constant used to indicate infinite size. This is equal to Integer.MAX_VALUE, ensuring
     * that it is greater than any other integer.
     */
   static const int INF;

    /**
     * Returns a bitwise combination of flags indicating how and when computePreferredSize should
     * be used. When called with horizontal=true, this indicates the usage of computePreferredSize(true,...)
     * for computing widths. When called with horizontal=false, this indicates the usage of computeSize(false,...)
     * for computing heights. These flags are used for optimization. Each flag gives the part more control
     * over its preferred size but slows down the layout algorithm. Parts should return the minimum set
     * of flags necessary to specify their constraints.
     * <p>
     * If the return value of this function ever changes, the part must call <code>flushLayout</code> before
     * the changes will take effect.
     * </p>
     *
     * <ul>
     * <li>SWT.MAX: The part has a maximum size that will be returned by computePreferredSize(horizontal,
     *     INF, someWidth, INF)</li>
     * <li>SWT.MIN: The part has a minimum size that will be returned by computePreferredSize(horizontal,
     *     INF, someWidth, 0)</li>
     * <li>SWT.WRAP: Indicates that computePreferredSize makes use of the availablePerpendicular argument. If this
     *     flag is not specified, then the third argument to computePreferredSize will always be set to
     *     INF. The perpendicular size is expensive to compute, and it is usually only used
     *     for wrapping parts.
     * <li>SWT.FILL: The part may not return the preferred size verbatim when computePreferredSize is
     *     is given a value between the minimum and maximum sizes. This is commonly used if the part
     *     wants to use a set of predetermined sizes instead of using the workbench-provided size.
     *     For example, computePreferredSize(horizontal, availableSpace, someWidth,
     *     preferredSize) may return the nearest predetermined size. Note that this flag should
     *     be used sparingly. It can prevent layout caching and cause the workbench layout algorithm
     *     to degrade to exponential worst-case runtime. If this flag is omitted, then
     *     computePreferredSize may be used to compute the minimum and maximum sizes, but not for
     *     anything in between.</li>
     * </ul>
     *
     * @param width a value of true or false determines whether the return value applies when computing
     * widths or heights respectively. That is, getSizeFlags(true) will be used when calling
     * computePreferredSize(true,...)
     * @return any bitwise combination of SWT.MAX, SWT.MIN, SWT.WRAP, and SWT.FILL
     */
    virtual int GetSizeFlags(bool width) = 0;

    /**
     * <p>
     * Returns the best size for this part, given the available width and height and the workbench's
     * preferred size for the part. Parts can overload this to enforce a minimum size, maximum size,
     * or a quantized set of preferred sizes. If width == true, this method computes a width in pixels.
     * If width == false, this method computes a height. availableParallel and availablePerpendicular
     * contain the space available, and preferredParallel contains the preferred result.
     * </p>
     *
     * <p>
     * This method returns an answer that is less than or equal to availableParallel and as
     * close to preferredParallel as possible. Return values larger than availableParallel will
     * be truncated.
     * </p>
     *
     * <p>
     * Most presentations will define a minimum size at all times, and a maximum size that only applies
     * when maximized.
     * </p>
     *
     * <p>
     * The getSizeFlags method controls how frequently this method will be called and what information
     * will be available when it is. Any subclass that specializes this method should also specialize
     * getSizeFlags. computePreferredSize(width, INF, someSize, 0) returns
     * the minimum size of the control (if any). computePreferredSize(width, INF, someSize,
     * INF) returns the maximum size of the control.
     * </p>
     *
     * <p>
     * Examples:
     * <ul>
     * <li>To maintain a constant size of 100x300 pixels: {return width ? 100 : 300}, getSizeFlags(boolean) must
     *     return SWT.MIN | SWT.MAX</li>
     * <li>To grow without constraints: {return preferredResult;}, getSizeFlags(boolean) must return 0.</li>
     * <li>To enforce a width that is always a multiple of 100 pixels, to a minimum of 100 pixels:
     *    <code>
     *        {
     *              if (width && preferredResult != INF) {
     *                  int result = preferredResult - ((preferredResult + 50) % 100) + 50;
     *                  result = Math.max(100, Math.min(result, availableParallel - (availableParallel % 100)));
     *
     *                  return result;
     *              }
     *              return preferredResult;
     *         }
     *    </code>
     *     In this case, getSizeFlags(boolean width) must return (width ? SWT.FILL | SWT.MIN: 0)
     * </ul>
     * <li>To maintain a minimum area of 100000 pixels:
     *     <code>
     *     {return availablePerpendicular < 100 ? 1000 : 100000 / availablePerpendicular;}
     *     </code>
     *     getSizeFlags(boolean width) must return SWT.WRAP | SWT.MIN;
     * </li>
     * </p>
     *
     * @param width indicates whether a width (=true) or a height (=false) is being computed
     * @param availableParallel available space. This is a width (pixels) if width == true, and a height (pixels)
     *  if width == false. A return value larger than this will be ignored.
     * @param availablePerpendicular available space perpendicular to the direction being measured
     * or INF if unbounded (pixels). This
     * is a height if width == true, or a height if width == false. Implementations will generally ignore this
     * argument unless they contain wrapping widgets. Note this argument will only contain meaningful information
     * if the part returns the SWT.WRAP flag from getSizeFlags(width)
     * @param preferredResult preferred size of the control (pixels, <= availableParallel). Set to
     * INF if unknown or unbounded.
     * @return returns the preferred size of the control (pixels). This is a width if width == true or a height
     * if width == false. Callers are responsible for rounding down the return value if it is larger than
     * availableParallel. If availableParallel is INF, then a return value of INF
     * is permitted, indicating that the preferred size of the control is unbounded.
     *
     * @see ISizeProvider#getSizeFlags(boolean)
     */
    virtual int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult) = 0;
};

}

#endif /* CHERRYISIZEPROVIDER_H_ */
