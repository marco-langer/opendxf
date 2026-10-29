// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Marco Langer

#pragma once

#include "opendxf/document.hpp"
#include "opendxf/entities.hpp"
#include "opendxf/error.hpp"
#include "opendxf/header.hpp"
#include "opendxf/tables.hpp"

#include <expected>
#include <filesystem>
#include <fstream>

namespace odxf {

class WriteStream;

std::expected<void, Error> write(const std::filesystem::path& filePath, const Document& document);

// Writes the header, the tables and an empty BLOCKS section, then returns a stream for the
// entities. The file is complete after WriteStream::close() or when the stream is destroyed.
std::expected<WriteStream, Error>
write(const std::filesystem::path& filePath, const Header& header, const Tables& tables);

class WriteStream final
{
public:
    WriteStream(const WriteStream&) = delete;
    WriteStream(WriteStream&& other) noexcept;
    WriteStream& operator=(const WriteStream&) = delete;
    WriteStream& operator=(WriteStream&& other) noexcept;
    // Closes the file if close() wasn't called. Errors are lost, call close() to get them.
    ~WriteStream();

    // Writes the entity at the end of the ENTITIES section, so entities keep the order of the
    // calls.
    std::expected<void, Error> entity(const Entity& entity);
    // Ends the ENTITIES section and the file.
    std::expected<void, Error> close();

private:
    explicit WriteStream(std::ofstream stream);

    friend std::expected<WriteStream, Error>
    write(const std::filesystem::path& filePath, const Header& header, const Tables& tables);

    std::ofstream m_stream;
    bool m_isClosed{ false };
};

}   // namespace odxf
