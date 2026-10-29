// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/document.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/header.hpp"
#include "opendxf/io/error.hpp"
#include "opendxf/tables.hpp"

#include <expected>
#include <filesystem>
#include <fstream>

namespace odxf {

class write_stream final
{
public:
    // Writes the header, the tables and an empty BLOCKS section, then returns a stream for the
    // entities. The file is complete after close() or when the stream is destroyed.
    static std::expected<write_stream, error> open(
        const std::filesystem::path& filePath,
        const odxf::header& header,
        const odxf::tables& tables);

    write_stream(const write_stream&) = delete;
    write_stream(write_stream&& other) noexcept;
    write_stream& operator=(const write_stream&) = delete;
    write_stream& operator=(write_stream&& other) noexcept;
    // Closes the file if close() wasn't called. Errors are lost, call close() to get them.
    ~write_stream();

    // Writes the entity at the end of the ENTITIES section, so entities keep the order of the
    // calls.
    std::expected<void, error> entity(const odxf::entity& entity);
    // Ends the ENTITIES section and the file.
    std::expected<void, error> close();

private:
    explicit write_stream(std::ofstream stream);

    std::ofstream m_stream;
    bool m_isClosed{ false };
};

std::expected<void, error>
write(const std::filesystem::path& filePath, const odxf::document& document);

}   // namespace odxf
