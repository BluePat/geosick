#include <iostream>
#include <rapidjson/writer.h>
#include "geosick/notify_process.hpp"
#include "geosick/sampler.hpp"

namespace geosick {

using JsonWriter = rapidjson::Writer<rapidjson::StringBuffer>;

static void check_bz(int bzerror) {
    if (bzerror != BZ_OK) {
        throw std::runtime_error("Error from libbzip2: " + std::to_string(bzerror));
    }
}

NotifyProcess::NotifyProcess(const Sampler* sampler,
    const std::filesystem::path& matches_path,
    const std::filesystem::path& selected_matches_path)
{
    m_sampler = sampler;
    m_matches_output.open(matches_path);
    if (!m_matches_output) {
        throw std::runtime_error("Could not open file for writing: " +
            matches_path.string());
    }

    m_selected_matches_file = std::fopen(selected_matches_path.c_str(), "wb");
    if (!m_selected_matches_file) {
        throw std::runtime_error("Could not open file for writing: " +
            selected_matches_path.string());
    }

    int bzerror = BZ_OK;
    m_selected_matches_bzfile = BZ2_bzWriteOpen(&bzerror, m_selected_matches_file,
        9, 0, 30);
    check_bz(bzerror);
    assert(m_selected_matches_bzfile != nullptr);
}

NotifyProcess::~NotifyProcess() {
    if (m_selected_matches_bzfile) {
        int bzerror = BZ_OK;
        BZ2_bzWriteClose(&bzerror, m_selected_matches_bzfile, 0, nullptr, nullptr);
        m_selected_matches_bzfile = nullptr;
    }

    if (m_selected_matches_file) {
        std::fclose(m_selected_matches_file);
        m_selected_matches_file = nullptr;
    }
}


static void row_to_json(JsonWriter& w, const GeoRow& row) {
    w.StartObject();
    w.Key("user_id"); w.Uint(row.user_id);
    w.Key("timestamp_utc_s"); w.Int(row.timestamp_utc_s);
    w.Key("lat_e7"); w.Int(row.lat);
    w.Key("lon_e7"); w.Int(row.lon);
    w.Key("accuracy_m"); w.Uint(row.accuracy_m);
    if (row.altitude_m != UINT16_MAX) {
        w.Key("altitude_m"); w.Uint(row.altitude_m);
    }
    if (row.heading_deg != UINT16_MAX) {
        w.Key("heading_deg"); w.Uint(row.heading_deg);
    }
    w.Key("velocity_mps"), w.Double(row.velocity_mps);
    w.EndObject();
}

static void sample_to_json(JsonWriter& w,
    const Sampler& sampler, const GeoSample& sample)
{
    w.StartObject();
    w.Key("time_index"); w.Int(sample.time_index);
    w.Key("timestamp_utc_s"); w.Int(sampler.time_index_to_timestamp(sample.time_index));
    w.Key("user_id"); w.Uint(sample.user_id);
    w.Key("lat_e7"); w.Int(sample.lat);
    w.Key("lon_e7"); w.Int(sample.lon);
    w.Key("accuracy_m"); w.Uint(sample.accuracy_m);
    w.EndObject();
}

static void step_to_json(JsonWriter& w,
    const Sampler& sampler, const MatchStep& step)
{
    w.StartObject();
    w.Key("time_index"); w.Int(step.time_index);
    w.Key("timestamp_utc_s"); w.Int(sampler.time_index_to_timestamp(step.time_index));
    w.Key("infect_rate"); w.Double(step.infect_rate);
    w.Key("distance_m"); w.Double(step.distance_m);
    w.EndObject();
}

static void match_to_json(JsonWriter& w, const Sampler& sampler,
    const MatchInput& mi, const MatchOutput& mo)
{
    w.StartObject();
    w.Key("query_user_id"); w.Uint(mi.query_user_id);
    w.Key("sick_user_id"); w.Uint(mi.sick_user_id);

    w.Key("query_rows");
    w.StartArray();
    for (const auto& query_row: mi.query_rows) {
        row_to_json(w, query_row);
    }
    w.EndArray();

    w.Key("sick_rows");
    w.StartArray();
    for (const auto& sick_row: mi.sick_rows) {
        row_to_json(w, sick_row);
    }
    w.EndArray();

    w.Key("query_samples");
    w.StartArray();
    for (const auto& query_sample: mi.query_samples) {
        sample_to_json(w, sampler, query_sample);
    }
    w.EndArray();

    w.Key("sick_samples");
    w.StartArray();
    for (const auto& sick_sample: mi.sick_samples) {
        sample_to_json(w, sampler, sick_sample);
    }
    w.EndArray();

    w.Key("steps");
    w.StartArray();
    for (const auto& step: mo.steps) {
        step_to_json(w, sampler, step);
    }
    w.EndArray();

    w.Key("score"); w.Double(mo.score);
    w.Key("min_distance_m"); w.Double(mo.min_distance_m);
    w.EndObject();
}

void NotifyProcess::notify(const MatchInput& mi, const MatchOutput& mo) {
    rapidjson::Writer<rapidjson::StringBuffer> w(m_match_buffer);
    match_to_json(w, *m_sampler, mi, mo);
    m_match_buffer.Put('\n');
    m_matches_output << m_match_buffer.GetString();

    if (std::bernoulli_distribution(0.1)(m_rng)) {
        int bzerror = BZ_OK;
        BZ2_bzWrite(&bzerror, m_selected_matches_bzfile,
            (void*)m_match_buffer.GetString(), (int)m_match_buffer.GetSize());
        check_bz(bzerror);
    }

    m_match_buffer.Clear();
    m_match_count += 1;
}

void NotifyProcess::close() {
    std::cout << "Reported " << m_match_count << " matches" << std::endl;
    m_matches_output.close();

    if (m_selected_matches_bzfile) {
        int bzerror = BZ_OK;
        BZ2_bzWriteClose(&bzerror, m_selected_matches_bzfile, 0, nullptr, nullptr);
        check_bz(bzerror);
        m_selected_matches_bzfile = nullptr;
    }

    if (m_selected_matches_file) {
        std::fclose(m_selected_matches_file);
        m_selected_matches_file = nullptr;
    }
}

}
