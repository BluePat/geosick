#include "geo_search.hpp"
#include <cassert>

namespace geosick {

GeoSearch::GeoSearch(const std::vector<GeoSample>& samples)
{
    m_points.reserve(samples.size());
    for (const auto& sample: samples) {
        m_points.push_back(UserGeoPoint{
            .user_id = sample.user_id,
            .time_index = sample.time_index,
            .lat = sample.lat,
            .lon = sample.lon,
            .accuracy_m = sample.accuracy_m,
        });
    }
}

std::vector<GeoSample::UserID>
GeoSearch::find_users_within_circle(int32_t lat, int32_t lon, unsigned radius, uint32_t time_index) const
{
    auto pow2 = [](const int64_t i) {
        return i*i;
    };

    std::vector<GeoSample::UserID> users;
    for (const auto& p: m_points) {
        if (p.time_index != time_index) { continue; }
        if (pow2(p.lat - lat) + pow2(p.lon - lon) <= pow2(p.accuracy_m + radius)) {
            users.push_back(p.user_id);
        }
    }
    return users;
}


}
