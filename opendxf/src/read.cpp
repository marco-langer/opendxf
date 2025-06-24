#include "opendxf/read.hpp"

#include "opendxf/header.hpp"
#include "opendxf/ireadstream.hpp"

#include <fmt/format.h>

#include <charconv>
#include <fstream>
#include <optional>

namespace odxf {

namespace {
    struct Data
    {
        int statusCode;
        std::string value;
    };

    std::optional<Data> readNext(std::ifstream& stream)
    {
        Data data;
        if (!std::getline(stream, data.value)) {
            return std::nullopt;
        }

        const auto [_, errorCode]{ std::from_chars(
            data.value.c_str(), data.value.c_str() + data.value.size(), data.statusCode) };
        if (errorCode != std::errc()) {
            return std::nullopt;
        }

        if (!std::getline(stream, data.value)) {
            return std::nullopt;
        }

        return data;
    }

    tl::expected<void, Error> readSectionBegin(std::ifstream& stream)
    {
        std::optional<Data> maybeData{ readNext(stream) };
        if (!maybeData) {
            return tl::make_unexpected(Error{ .type = Error::Type::InvalidFile });
        }
        const Data& data{ *maybeData };
        if (data.statusCode != 0 || data.value != "SECTION") {
            return tl::make_unexpected(Error{ .type = Error::Type::InvalidFile });
        }

        return {};
    }

    tl::expected<void, Error> readHeaderBegin(std::ifstream& stream)
    {
        tl::expected<void, Error> sectionBeginResult{ readSectionBegin(stream) };
        if (!sectionBeginResult) {
            return tl::make_unexpected(sectionBeginResult.error());
        }

        std::optional<Data> maybeData{ readNext(stream) };
        if (!maybeData) {
            return tl::make_unexpected(Error{ .type = Error::Type::InvalidFile });
        }
        const Data& data{ *maybeData };
        if (data.statusCode != 2 || data.value != "HEADER") {
            return tl::make_unexpected(Error{ .type = Error::Type::InvalidFile });
        }

        return {};
    }

    bool isEndSection(const Data& data) { return data.statusCode == 0 && data.value == "ENDSEC"; }

    tl::expected<std::optional<std::pair<HeaderKey, HeaderValue>>, Error>
    readHeaderEntry(std::ifstream& stream)
    {
        std::optional<Data> maybeData{ readNext(stream) };
        if (!maybeData) {
            return tl::make_unexpected(Error{ .type = Error::Type::InvalidFile });
        }

        const Data& data{ *maybeData };
        if (isEndSection(data)) {
            return std::nullopt;
        }

        if (data.statusCode != 9) {
            return tl::make_unexpected(Error{ .type = Error::Type::InvalidFile });
        }

        std::pair<HeaderKey, HeaderValue> headerEntry;
        headerEntry.first = data.value;

        return headerEntry;
    }

    tl::expected<Header, Error> readHeader(std::ifstream& stream)
    {
        Header header;

        tl::expected<void, Error> headerBeginResult{ readHeaderBegin(stream) };
        if (!headerBeginResult) {
            return tl::make_unexpected(headerBeginResult.error());
        }

        while (true) {
            tl::expected<std::optional<std::pair<HeaderKey, HeaderValue>>, Error> maybeHeaderEntry;
            if (!maybeHeaderEntry) {
                return tl::make_unexpected(maybeHeaderEntry.error());
            }

            if (!*maybeHeaderEntry) {
                return header;
            }

            const auto [_, success]{ header.entries.insert(std::move(**maybeHeaderEntry)) };
            if (!success) {
                return tl::make_unexpected(
                    Error{ .type = Error::Type::InvalidFile, .what = "duplicate header entry" });
            }
        }

        return header;
    }
}   // namespace

tl::expected<void, Error> read(IReadStream& stream, const std::filesystem::path& filePath)
{
    std::ifstream fileStream{ filePath };
    if (!fileStream.is_open()) {
        return tl::make_unexpected(
            Error{ .type = Error::Type::FileOpenError,
                   .what = fmt::format(
                       "unable to open file {}",
                       reinterpret_cast<const char*>(filePath.u8string().c_str())) });
    }

    tl::expected<Header, Error> maybeHeader{ readHeader(fileStream) };
    if (!maybeHeader) {
        return tl::make_unexpected(maybeHeader.error());
    }

    stream.header(*maybeHeader);

    return {};
}

}   // namespace odxf
