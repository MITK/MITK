/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestRouter_h
#define mitkRestRouter_h

#include <mitkRestRoute.h>

#include <shared_mutex>
#include <vector>

namespace mitk
{
  /**
   * \brief Finds the route that answers a request.
   *
   * Routes can be added and removed while other threads route requests. Of the
   * routes that match a path, the one with the most literal segments wins, so
   * "/editors/mxn" beats "/editors/:id" whatever order they were added in.
   * Among routes with equally many, the one that has the literal segment at
   * the first position where they differ wins.
   */
  class MITKRESTAPI_EXPORT RestRouter
  {
  public:
    struct Match
    {
      RestHandler Handler;
      std::map<std::string, std::string> PathParams;
    };

    /**
     * \param route A route with a full path, like "/api/v1/manual".
     *
     * \throw mitk::Exception if the method is unknown, the path does not start
     *        with a slash, the handler is empty, the path names a parameter
     *        twice, or a route with the same method and path exists. Paths
     *        that differ only in the names of their parameters are the same.
     */
    RestRouteId Add(RestRoute route);

    void Remove(RestRouteId id);

    /** \brief The match for a request; a HEAD request finds the GET route of the path. */
    std::optional<Match> Find(const std::string& method, const std::string& path) const;

    /** \brief The routes whose paths start with \p prefix, in the order they were added. */
    std::vector<RestRoute> GetRoutes(const std::string& prefix) const;

  private:
    struct Entry
    {
      RestRouteId Id;
      RestRoute Route;
      std::vector<std::string> Segments;
      int LiteralSegmentCount;
    };

    mutable std::shared_mutex m_Mutex;
    std::vector<Entry> m_Entries;
    RestRouteId m_NextId = 1;
  };
}

#endif
