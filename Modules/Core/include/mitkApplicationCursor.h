/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkApplicationCursor_h
#define mitkApplicationCursor_h

#include <mitkNumericTypes.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Abstract interface for toolkit-specific cursor implementations.
   *
   * For any GUI toolkit, this class must be sub-classed to provide cursor
   * management functionality. One instance of the sub-class must be registered
   * with mitk::ApplicationCursor. See QmitkApplicationCursor for an example.
   *
   * \sa ApplicationCursor
   */
  class MITKCORE_EXPORT ApplicationCursorImplementation
  {
  public:
    /**
     * \brief Push a new cursor onto the cursor stack (XPM format).
     *
     * \param[in] XPM The cursor image in XPM format.
     * \param[in] hotspotX The x-coordinate of the cursor hotspot.
     * \param[in] hotspotY The y-coordinate of the cursor hotspot.
     */
    virtual void PushCursor(const char *XPM[], int hotspotX, int hotspotY) = 0;

    /**
     * \brief Push a new cursor onto the cursor stack (stream format).
     *
     * \param[in,out] cursorStream Input stream containing cursor data.
     * \param[in] hotspotX The x-coordinate of the cursor hotspot.
     * \param[in] hotspotY The y-coordinate of the cursor hotspot.
     */
    virtual void PushCursor(std::istream &cursorStream, int hotspotX, int hotspotY) = 0;

    /** \brief Restore the previous cursor from the stack. */
    virtual void PopCursor() = 0;

    /**
     * \brief Get the absolute mouse position on screen.
     * \return The current cursor position in screen coordinates.
     */
    virtual const Point2I GetCursorPosition() = 0;

    /**
     * \brief Set the absolute mouse position on screen.
     * \param[in] position The desired cursor position in screen coordinates.
     */
    virtual void SetCursorPosition(const Point2I &position) = 0;

    virtual ~ApplicationCursorImplementation() {}
  };

  /**
   * \brief Singleton for overriding the application's cursor.
   *
   * Provides a stack-based mechanism to override the application cursor with
   * context-dependent cursors. Accepts cursors in XPM format. Push a cursor
   * on top of the stack and later pop it to restore the previous state. This
   * mimics Qt's QApplication::setOverrideCursor() behavior.
   *
   * A toolkit-specific ApplicationCursorImplementation must be registered
   * before any cursor operations are used.
   *
   * \sa ApplicationCursorImplementation
   */
  class MITKCORE_EXPORT ApplicationCursor
  {
  public:
    /**
     * \brief Get the singleton instance.
     * \return Pointer to the singleton ApplicationCursor.
     */
    static ApplicationCursor *GetInstance();

    /**
     * \brief Register a toolkit-specific cursor implementation.
     * \param[in] implementation The implementation to use for cursor operations.
     */
    static void RegisterImplementation(ApplicationCursorImplementation *implementation);

    /**
     * \brief Push a new cursor onto the stack (XPM format).
     *
     * \param[in] XPM The cursor image in XPM format.
     * \param[in] hotspotX The x-coordinate of the cursor hotspot (-1 for default).
     * \param[in] hotspotY The y-coordinate of the cursor hotspot (-1 for default).
     */
    void PushCursor(const char *XPM[], int hotspotX = -1, int hotspotY = -1);

    /**
     * \brief Push a new cursor onto the stack (stream format).
     *
     * \param[in,out] cursorStream Input stream containing cursor data.
     * \param[in] hotspotX The x-coordinate of the cursor hotspot (-1 for default).
     * \param[in] hotspotY The y-coordinate of the cursor hotspot (-1 for default).
     */
    void PushCursor(std::istream &cursorStream, int hotspotX = -1, int hotspotY = -1);

    /** \brief Pop the most recent cursor from the stack, restoring the previous one. */
    void PopCursor();

    /**
     * \brief Get the absolute mouse position on screen.
     * \return The cursor position, or (-1, -1) if querying is not possible.
     */
    const Point2I GetCursorPosition();

    /**
     * \brief Set the absolute mouse position on screen.
     * \param[in] position The desired cursor position in screen coordinates.
     */
    void SetCursorPosition(const Point2I &position);

  protected:
    /** \brief Hidden constructor (singleton pattern). */
    ApplicationCursor();

  private:
    static ApplicationCursorImplementation *m_Implementation;
  };

} // namespace

#endif
