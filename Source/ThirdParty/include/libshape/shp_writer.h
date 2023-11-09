#pragma once

#include <string>

namespace shp {
    struct Layer;

    class ShpWriter {
    public:
        static void write(const std::string& filename, const Layer& layer);
        //static void readDbf(const std::string& filename, AttributeDatabase& out_attribute_database);
    };
}