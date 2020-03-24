#pragma once
#include "sampler.hpp"

namespace geosick {

class GeoSearch {
private:
    struct UserGeoPoint {
        GeoSample::UserID user_id;
        int32_t lat, lon;
        uint16_t accuracy_m;
    };
    std::vector<UserGeoPoint> m_points;
public:

    explicit GeoSearch(const std::vector<GeoSample>& samples);

    std::vector<GeoSample::UserID>
    find_users_within_circle(int32_t lat, int32_t lon, unsigned radius_m) const;
};

}
