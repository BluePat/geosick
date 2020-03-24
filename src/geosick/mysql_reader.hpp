#pragma once
#include <mysql++/mysql++.h>
#include "geosick/geo_row_reader.hpp"

namespace geosick {

class MysqlReader final: public GeoRowReader {
    mysqlpp::Connection _conn;
    mysqlpp::UseQueryResult _result;
public:
    MysqlReader(const std::string& db, const std::string& host, unsigned port,
        const std::string& user, const std::string& password);
    virtual std::optional<GeoRow> read() override;
};

}
