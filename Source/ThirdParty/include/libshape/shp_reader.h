#pragma once

#include <string>

namespace shp {
    struct Layer;

    class ShpReader {
    public:
        static std::shared_ptr<Layer> read(const std::string& filename);
        //static void readDbf(const std::string& filename, AttributeDatabase& out_attribute_database);
    };
}