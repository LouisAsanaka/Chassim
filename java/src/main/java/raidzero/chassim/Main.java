package raidzero.chassim;

import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.IOException;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonDeserializer;

import raidzero.pathgen.PathGenerator;
import raidzero.pathgen.Point;

public class Main {

    public static void main(String[] args) throws IOException {
        var reader = new BufferedReader(new InputStreamReader(System.in));
        var gson = new GsonBuilder()
            .registerTypeAdapter(Point.class, (JsonDeserializer<Point>) (json, type, context) -> {
                    var object = json.getAsJsonObject();
                    var x = object.get("x").getAsDouble();
                    var y = object.get("y").getAsDouble();
                    var angle = object.get("angle");
                    if (angle == null) {
                        return new Point(x, y);
                    }
                    return new Point(x, y, angle.getAsDouble());
                })
            .create();
        while (true) {
            var req = gson.fromJson(reader.readLine(), Request.class);
            var queryData = PathGenerator.getQueryData(req.waypoints);
            var splines = PathGenerator.calculateSplines(req.waypoints, queryData);
            var xQueries = PathGenerator.query(t -> splines.x.value(t)[0], queryData);
            var yQueries = PathGenerator.query(t -> splines.y.value(t)[0], queryData);
            var res = new Response();
            res.path = new ChassimPoint[queryData.queryCount];
            for (var i = 0; i < queryData.queryCount; i++) {
                res.path[i] = new ChassimPoint();
                res.path[i].x = xQueries[i];
                res.path[i].y = yQueries[i];
            }
            PathGenerator.calculatePathPoints(
                res.path, req.cruiseVelocity, req.targetAcceleration, 0.0, splines, queryData);
            System.out.println(gson.toJson(res));
        }
    }

}
