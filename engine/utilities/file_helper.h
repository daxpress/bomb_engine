#pragma once

namespace BE_NAMESPACE::file_helper
{
enum class file_error : uint8_t
{
    file_not_found = 0,
    invalid_filesize,
    read_error,
};

auto load_file(const std::filesystem::path& filepath
) -> std::expected<std::vector<char>, file_error>;
}  // namespace BE_NAMESPACE::file_helper