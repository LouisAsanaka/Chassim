package raidzero.pathgen;

import java.util.function.DoubleUnaryOperator;
import org.apache.commons.math3.analysis.interpolation.HermiteInterpolator;

/**
 * Calculations for generating path from waypoints.
 */
public class PathGenerator {

    /**
     * Number of query points per inch of approximate distance.
     *
     * <p>Increasing this number results in more query points and greater
     * accuracy for path generation.
     */
    private static final double QUERY_INTERVAL = 1;

    /**
     * Generates a path that passes through the given waypoints on the field.
     *
     * <p>The returned path contains data which can be passed to motion profile to execute the path.
     * The robot will accelerate at a constant rate, then reach a constant cruise velocity, then
     * decelerate at the same rate. In other words, the velocity vs time graph shows an isosceles
     * trapezoid.
     *
     * @param waypoints points that the path should pass through
     * @param cruiseVelocity the target constant cruise velocity of the robot in in/100ms
     * @param targetAcceleration the target constant acceleration (and deceleration) of the robot in
     * in/100ms/s
     * @return an array of points on the path
     */
    public static PathPoint[] generatePath(Point[] waypoints,
    double cruiseVelocity, double targetAcceleration) {

        var queryData = getQueryData(waypoints);
        var splines = calculateSplines(waypoints, queryData);

        var path = new PathPoint[queryData.queryCount];
        for (var i = 0; i < path.length; i++) {
            path[i] = new PathPoint();
        }

        calculatePathPoints(path, cruiseVelocity, targetAcceleration, splines, queryData);

        return path;

    }

    public static class QueryData {
        public double[] cumulativeWaypointDistances;
        public double totalWaypointDistance;
        public int queryCount;
    }

    public static QueryData getQueryData(Point[] waypoints) {
        var queryData = new QueryData();
        queryData.cumulativeWaypointDistances = calculateCumulativeDistances(waypoints);
        queryData.totalWaypointDistance = queryData.cumulativeWaypointDistances
            [queryData.cumulativeWaypointDistances.length - 1];
        queryData.queryCount =
            (int) Math.ceil(queryData.totalWaypointDistance / QUERY_INTERVAL) + 1;
        return queryData;
    }

    private static double[] calculateCumulativeDistances(Point[] waypoints) {
        var cumulativeDistances = new double[waypoints.length];
        for (var i = 1; i < waypoints.length; i++) {
            cumulativeDistances[i] = cumulativeDistances[i - 1]
                + Math.hypot(waypoints[i].x - waypoints[i - 1].x,
                    waypoints[i].y - waypoints[i - 1].y);
        }
        return cumulativeDistances;
    }

    public static class SplinePair {
        public HermiteInterpolator x;
        public HermiteInterpolator y;
    }

    public static SplinePair calculateSplines(Point[] waypoints, QueryData queryData) {
        var splinePair = new SplinePair();
        splinePair.x = new HermiteInterpolator();
        splinePair.y = new HermiteInterpolator();
        for (var i = 0; i < waypoints.length; i++) {
            double waypointDistance = queryData.cumulativeWaypointDistances[i];
            double waypointX = waypoints[i].x;
            double waypointY = waypoints[i].y;
            waypoints[i].angle.ifPresentOrElse(ang -> {
                splinePair.x.addSamplePoint(waypointDistance, new double[] { waypointX },
                    new double[] { Math.cos(Math.toRadians(ang)) });
                splinePair.y.addSamplePoint(waypointDistance, new double[] { waypointY },
                    new double[] { Math.sin(Math.toRadians(ang)) });
            }, () -> {
                splinePair.x.addSamplePoint(waypointDistance, new double[] { waypointX });
                splinePair.y.addSamplePoint(waypointDistance, new double[] { waypointY });
            });
        }
        return splinePair;
    }

    public static void calculatePathPoints(PathPoint[] path, double cruiseVelocity,
    double targetAcceleration, SplinePair splines, QueryData queryData) {

        // Convert the target acceleration to in/(100ms)^2 for consistent unit of time
        targetAcceleration /= 10;

        var dxQueries = query(splines.x.getPolynomials()[0].derivative()::value, queryData);
        var dyQueries = query(splines.y.getPolynomials()[0].derivative()::value, queryData);

        // The angle for each point is calculated using arctan of the ratio between dy and dx.
        // Since atan2 wraps the angle to be within -180 to 180 (because it has no way of knowing
        // the actual angle of the robot), we estimate the real unwrapped angle by looking at if the
        // current angle would be closer to the previous angle after adding or subtracting 360. This
        // is fine because the robot can't turn more than 180 degrees in between two data points on
        // the path. The angle for the first point should be given.
        for (var i = 0; i < path.length; i++) {
            path[i].angle = Math.toDegrees(Math.atan2(dyQueries[i], dxQueries[i]));
            if (i > 0) {
                while (path[i].angle - path[i - 1].angle > 180) {
                    path[i].angle -= 360;
                }
                while (path[i].angle - path[i - 1].angle < -180) {
                    path[i].angle += 360;
                }
            }
        }

        // Position is calculated with a running total of arc lengths with Riemann sum. Arclength
        // formula integrate(hypot(dy/dt, dx/dt)*dt) where dt is QUERY_INTERVAL. Rectangles are
        // centered at each querypoint, so the cumulative area under curve is half a rectangle each
        // from the last point and the current point. The interval for the last pathpoint is shorter
        // as it is the last waypoint, and QUERY_INTERVAL does not divide into the full path length.
        for (var i = 1; i < path.length; i++) {
            double interval = i == path.length - 1 ?
                queryData.totalWaypointDistance % QUERY_INTERVAL : QUERY_INTERVAL;
            path[i].position = 0.5 * interval * (Math.hypot(dxQueries[i], dyQueries[i])
                + Math.hypot(dxQueries[i - 1], dyQueries[i - 1])) + path[i - 1].position;
        }
        var totalDistance = path[path.length - 1].position;

        // Calculations for velocity and time are in three separate stages for the acceleration,
        // constant velocity, and deceleration parts respectively.
        {
            var reachesCruiseVelocity = false;
            // First stage: we find velocity and time for data points assuming constant acceleration
            // up until either (1) we reach the cruise velocity, or (2) we have not reached cruise
            // velocity and we are halfway through the path already, in which case we must start
            // decelerating.
            int i;
            for (i = 0; path[i].position <= totalDistance / 2; i++) {
                // v^2 = 2 a x
                // v = sqrt(2 a x)
                var velocity = Math.sqrt(2 * targetAcceleration * path[i].position);
                if (velocity > cruiseVelocity) {
                    reachesCruiseVelocity = true;
                    break;
                }
                path[i].velocity = velocity;
                // x = (1/2) a t^2
                // t^2 = 2 x / a
                // t = sqrt(2 x / a)
                path[i].time = Math.sqrt(2 * path[i].position / targetAcceleration);
            }
            // Third stage for velocity: we do the same thing as the first stage, but working
            // backwards instead. We stop when either (1) we reach the cruise velocity, or (2) we
            // reach where stage 1 stopped, in which case there is no constant velocity part.
            int j;
            for (j = path.length - 1; j >= i; j--) {
                var velocity = Math.sqrt(2 * targetAcceleration
                    * (totalDistance - path[j].position));
                if (velocity > cruiseVelocity) {
                    break;
                }
                path[j].velocity = velocity;
            }
            // v = a t
            // t = v / a
            var cruiseStartTime = cruiseVelocity / targetAcceleration;
            // v^2 = 2 a x
            // x = v^2 / (2 a)
            var cruiseStartPosition = cruiseVelocity * cruiseVelocity / (2 * targetAcceleration);
            int k;
            // Second stage: we calculate velocity and time in between the end of first stage and
            // start of third stage. If we never reach the cruise velocity, this loop never runs.
            for (k = i; k <= j; k++) {
                path[k].velocity = cruiseVelocity;
                // t = t0 + delta_t
                // v = delta_x / delta_t
                // delta_t = delta_x / v
                // delta_x = x - x0
                // t = t0 + (x - x0) / v
                path[k].time = cruiseStartTime
                    + (path[k].position - cruiseStartPosition) / cruiseVelocity;
            }
            var decelerateStartPosition = reachesCruiseVelocity
                // It takes the same distance to accelerate and decelerate to/from the same velocity
                ? totalDistance - cruiseStartPosition
                : totalDistance / 2;
            var decelerateStartTime = reachesCruiseVelocity
                ? cruiseStartTime + (decelerateStartPosition - cruiseStartPosition) / cruiseVelocity
                : Math.sqrt(totalDistance / targetAcceleration);
            var decelerateInitialVelocity = reachesCruiseVelocity
                ? cruiseVelocity
                // v^2 = 2 a x_mid
                // v = sqrt(2 a x_mid)
                // x_mid = x_total / 2
                // v = sqrt(a x_total)
                : Math.sqrt(targetAcceleration * totalDistance);
            // Third stage for time: calculate from end of stage 2 to end of path
            for (; k < path.length; k++) {
                // x = x0 + v0 t + (1/2) (-a) t^2
                // (1/2) (-a) t^2 + v0 t + (x0 - x) = 0
                // t = (-v0 +- sqrt(v0^2 - 4 ((1/2) (-a)) (x0 - x))) / (2 ((1/2) (-a)))
                // t = (-v0 +- sqrt(v0^2 - 2 (-a) (x0 - x))) / -a
                path[k].time = decelerateStartTime
                    + (-decelerateInitialVelocity
                        + Math.sqrt(decelerateInitialVelocity * decelerateInitialVelocity
                            - 2 * -targetAcceleration
                                * (decelerateStartPosition - path[k].position)))
                    / -targetAcceleration;
            }
        }

        // Calculate the time differences (i.e. make the times not cumulative)
        for (var i = path.length - 1; i > 0; i--) {
            path[i].time -= path[i - 1].time;
        }

    }

    public static double[] query(DoubleUnaryOperator f, QueryData queryData) {
        var queries = new double[queryData.queryCount];
        for (var i = 0; i < queryData.queryCount - 1; i++) {
            queries[i] = f.applyAsDouble(i * QUERY_INTERVAL);
        }
        queries[queryData.queryCount - 1] = f.applyAsDouble(queryData.totalWaypointDistance);
        return queries;
    }

}
