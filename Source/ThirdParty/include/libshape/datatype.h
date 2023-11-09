#pragma once

#include <string>
#include <vector>

namespace shp {
    /*
     * https://en.wikipedia.org/wiki/Shapefile
     */
    enum class ShapeType : uint8_t {
        Null = 0,
        Point = 1,
        PolyLine = 3,
        Polygon = 5,
        PointZ = 11,
        PolyLineZ = 13,
        PolygonZ = 15,
    };

    struct Point
    {
        Point(const double x, const double y, const double z)
            : x(x), y(y), z(z) {}

        double x;
        double y;
        double z;
    };

    struct Shape {
        std::vector<Point> points;
    };

    struct AttributeDatabase {
        std::vector<std::string> fields;
        std::vector<std::vector<std::string>> attributes;
    };

    struct Layer {
        Layer()
            : type()
        {}

        ShapeType type;
        std::vector<Shape> shapes;
        AttributeDatabase attribute_database;
    };
}