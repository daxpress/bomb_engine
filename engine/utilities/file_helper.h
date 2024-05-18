#pragma once

namespace BE_NAMESPACE::file_helper
{
	enum class file_error
	{
		file_not_found = 0,
		invalid_filesize,
		read_error,
	};

	std::expected<std::vector<char>, file_error> load_file(const std::filesystem::path& filepath);
}