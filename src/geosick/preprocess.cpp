#include <algorithm>
#include "geosick/file_reader.hpp"
#include "geosick/file_writer.hpp"
#include "geosick/merge_reader.hpp"
#include "geosick/preprocess.hpp"

namespace geosick {

namespace {
    struct CompareRows {
        bool operator()(const GeoRow& r1, const GeoRow& r2) {
            if (r1.user_id < r2.user_id) { return true; }
            if (r1.user_id > r2.user_id) { return false; }
            return r1.timestamp_utc_s < r2.timestamp_utc_s;
        }
    };
}

Preprocess::Preprocess(std::unordered_set<uint32_t> infected_user_ids,
    std::filesystem::path temp_dir,
    size_t row_buffer_size
):
    m_infected_user_ids(std::move(infected_user_ids)),
    m_temp_dir(std::move(temp_dir)),
    m_row_buffer_size(row_buffer_size)
{}

void Preprocess::flush_buffer() {
    std::sort(m_row_buffer.begin(), m_row_buffer.end(), CompareRows());

    auto temp_name = "rows_" + std::to_string(m_temp_file_counter++) + ".bin";
    auto temp_path = m_temp_dir / temp_name;
    m_temp_files.push_back(temp_path);

    FileWriter writer(temp_path);
    writer.write(m_row_buffer.data(), m_row_buffer.size());
    writer.close();
    m_row_buffer.clear();
}

void Preprocess::process(GeoRowReader& reader) {
    m_row_buffer.reserve(m_row_buffer_size);
    while (auto row = reader.read()) {
        if (m_infected_user_ids.count(row->user_id)) {
            m_infected_rows.push_back(*row);
        }

        m_row_buffer.push_back(*row);
        if (m_row_buffer.size() >= m_row_buffer_size) {
            this->flush_buffer();
        }
    }
    if (m_row_buffer.size() > 0) {
        this->flush_buffer();
    }
}

std::unique_ptr<GeoRowReader> Preprocess::read_all_rows() {
    auto merger = std::make_unique<MergeReader<CompareRows>>(CompareRows());
    for (const auto& path: m_temp_files) {
        merger->add_reader(std::make_unique<FileReader>(path));
        std::filesystem::remove(path);
    }
    return merger;
}

std::vector<GeoRow> Preprocess::read_infected_rows() {
    return std::move(m_infected_rows);
}

}
