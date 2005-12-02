package router;

import org.eclipse.draw2d.Connection;
import org.eclipse.draw2d.ConnectionRouter;
import org.eclipse.draw2d.geometry.Point;

/**
 * Generic support for ConnectionRouters.
 */
public abstract class MyAbstractRouter
	implements ConnectionRouter
{

private static final Point START = new Point();
private static final Point END = new Point();

/**
 * Returns the constraint for the given Connection.
 * 
 * @param connection The connection
 * @return The constraint
 * @since 2.0
 */
public Object getConstraint(Connection connection) {
	return null;
}

/**
 * A convenience method for obtaining a connection's endpoint.  The connection's endpoint
 * is a point in absolute coordinates obtained by using its source and target {@link
 * ConnectionAnchor}. The returned Point is a static singleton that is reused to reduce
 * garbage collection. The caller may modify this point in any way. However, the point
 * will be reused and its values overwritten during the next call to this method.
 * 
 * @param connection The connection
 * @return The endpoint
 * @since 2.0
 */
protected Point getEndPoint(Connection connection) {
	Point ref;
	if (connection.getSourceAnchor().getReferencePoint().equals(connection.getTargetAnchor().getReferencePoint())) {
		int height = connection.getTargetAnchor().getOwner().getSize().height;
		int width = connection.getTargetAnchor().getOwner().getSize().width;
		ref = connection.getSourceAnchor().getReferencePoint();
		ref.x = ref.x + width / 2;
		ref.y = ref.y - height / 4;
		return END.setLocation(connection.getTargetAnchor().getLocation(ref));
	}
	else {
		ref = connection.getSourceAnchor().getReferencePoint();
	}
	return END.setLocation(connection.getTargetAnchor().getLocation(ref));
}

/**
 * A convenience method for obtaining a connection's start point.  The connection's
 * startpoint is a point in absolute coordinates obtained by using its source and target
 * {@link ConnectionAnchor}. The returned Point is a static singleton that is reused to
 * reduce garbage collection. The caller may modify this point in any way. However, the
 * point will be reused and its values overwritten during the next call to this method.
 * 
 * @param conn The connection
 * @return The start point
 * @since 2.0
 */
protected Point getStartPoint(Connection conn) {
	Point ref;
	if (conn.getSourceAnchor().getReferencePoint().equals(conn.getTargetAnchor().getReferencePoint())) {
		int height = conn.getTargetAnchor().getOwner().getSize().height;
		int width = conn.getTargetAnchor().getOwner().getSize().width;
		ref = conn.getTargetAnchor().getReferencePoint();
		ref.x = ref.x + width / 2;
		ref.y = ref.y - height / 2;
		ref.x = ref.x - width / 6;
		return START.setLocation(conn.getSourceAnchor().getLocation(ref));
	}
	else {
		ref = conn.getTargetAnchor().getReferencePoint();
	}
	return START.setLocation(conn.getSourceAnchor().getLocation(ref));
}

/**
 * Causes the router to discard any cached information about the given Connection.
 * 
 * @param connection The connection to invalidate
 * @since 2.0
 */
public void invalidate(Connection connection) { }

/**
 * Removes the given Connection from this routers list of Connections it is responsible
 * for.
 * 
 * @param connection The connection to remove
 * @since 2.0
 */
public void remove(Connection connection) { }

/**
 * Sets the constraint for the given Connection.
 * 
 * @param connection The connection
 * @param constraint The constraint
 * @since 2.0
 */
public void setConstraint(Connection connection, Object constraint) { }

}
