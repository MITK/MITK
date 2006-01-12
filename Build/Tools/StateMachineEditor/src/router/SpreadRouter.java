package router;


import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.PointList;
import javax.vecmath.*;
import router.MyAutomaticRouter;

/**
 * Automatic router that spreads its {@link Connection Connections} in a
 * fan-like fashion upon collision.
 */
public class SpreadRouter extends MyAutomaticRouter {
	private int separation = 10;
	
	/**
	 * Returns the separation in pixels between fanned connections.
	 * 
	 * @return the separation
	 * @since 2.0
	 */
	public int getSeparation() {
		return separation;
	}

	/**
	 * Modifies a given PointList that collides with some other PointList. The
	 * given <i>index</i> indicates that this is the i<sup>th</sup> PointList
	 * in a group of colliding points.
	 * 
	 * @param points
	 *            the colliding points
	 * @param index
	 *            the index
	 */
	protected void handleCollision(PointList points, int index) {
		Point start = points.getFirstPoint();
		Point end = points.getLastPoint();

		if (start.equals(end)) {
			return;
		}
		
		double k = 20;
		
		Point2d s = new Point2d( start.x, start.y );
		Point2d e = new Point2d( end.x, end.y );		
		Vector2d v = new Vector2d( e );
		v.sub( s );
		Vector2d vk = new Vector2d( v );
		vk.scale( 1.0 / k );
		Vector2d h = new Vector2d( vk.y, -vk.x );
		h.normalize();
		Point2d help = new Point2d();
		Point2d p = new Point2d();
		for ( int i=1; i < k; i++){
			double fact = Math.sin( Math.PI * (double) i / k ) * separation * (double) index;
			p.set( vk );
			p.scale( i );
			p.add( s );
			
			help.set( h );
			help.scale( fact );
			p.add( help );
			Point pp = new Point( p.x, p.y );			
			points.insertPoint( pp, i );
		}
	}
	
	/**
	 * Modifies a given PointList that collides with some other PointList. The
	 * given <i>index</i> indicates that this is the i<sup>th</sup> PointList
	 * in a group of colliding points.
	 * 
	 * @param points
	 *            the colliding points
	 * @param index
	 *            the index
	 */
	/* (non-Javadoc)
	 * @see router.MyAutomaticRouter#handleCollision1(org.eclipse.draw2d.geometry.PointList, int)
	 */
	protected void handleCollision1(PointList points, int index) {
		int i = -1;
		Point start = points.getFirstPoint();
		Point end = points.getLastPoint();
		Point2d s = new Point2d(start.x, start.y);
		Point2d e = new Point2d(end.x, end.y);
		Vector2d vk = new Vector2d(s.x, s.y);
		vk.sub(e);
		
		double radius = vk.length() / 2 + index * separation / 2;
		double r = radius*radius;
		double xa = e.x;
		double ya = e.y;
		double xb = s.x;
		double yb = s.y;


		double a = -((-2*ya)-(-2*yb))/((-2*xa)-(-2*xb));
		double b = -((xa*xa+ya*ya-r)-(xb*xb+yb*yb-r))/((-2*xa)-(-2*xb));
		double p = (-2*(xa-b)*a-2*ya)/(a*a+1);
		double q = ((xa-b)*(xa-b)+ya*ya-r)/(a*a+1);
		double y1 = -p/2 - Math.sqrt((p*p)/4 -q);
		double x1 = a*y1+b;
		
		Point2d midPoint = new Point2d(x1, y1);
		
		double xmin = 0;
		double xmax = 0;
		xmin = midPoint.x - radius;
		xmax = midPoint.x + radius;
		points.removeAllPoints();
		double xmaxi = 0;
		// test if the state is in the -X area
		if (xmax < 0) {
			xmaxi = xmax;
		}
		else {
			xmaxi = xmax + 1;
		}
		if (s.y > midPoint.y) {
			if (xmin > 0) {
				for ( int x = (int) s.x; x > (int) xmin; x--) {
					double y = midPoint.y + Math.sqrt(radius * radius - (x - midPoint.x) * (x - midPoint.x));
					Point v = new Point(x, y);
					i++;
					points.insertPoint(v, i);
				}
			}
			else {
				for ( int x = (int) s.x; x >= (int) xmin; x--) {
					double y = midPoint.y + Math.sqrt(radius * radius - (x - midPoint.x) * (x - midPoint.x));
					Point v = new Point(x, y);
					i++;
					points.insertPoint(v, i);
				}
			}
			for (int x = (int) xmin + 1; x < (int) xmaxi; x++) {
				double y = midPoint.y - Math.sqrt(radius * radius - (x - midPoint.x) * (x - midPoint.x));
				Point v = new Point(x, y);
				i++;
				points.insertPoint(v, i);
			}
		}
		else if (xmin < s.x) {
			for (int x = (int) s.x; x < (int) xmaxi; x++) {
				double y = midPoint.y - Math.sqrt(radius * radius - (x - midPoint.x) * (x - midPoint.x));
				Point v = new Point(x, y);
				i++;
				points.insertPoint(v, i);
			}
		}
		for (int x = (int) xmax - 1; x >= (int) e.x; x--) {
			double y = midPoint.y + Math.sqrt(radius * radius - (x - midPoint.x) * (x - midPoint.x));
			Point v = new Point(x, y);
			i++;
			points.insertPoint(v, i);
		}
	}

	/**
	 * Sets the colliding {@link Connection Connection's} separation in pixels.
	 * 
	 * @param value
	 *            the separation
	 * @since 2.0
	 */
	public void setSeparation(int value) {
		separation = value;
	}

}
