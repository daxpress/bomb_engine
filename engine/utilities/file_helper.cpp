#include "file_helper.h"

namespace bomb_engine::file_helper
{
    std::expected<std::vector<char>, file_error> load_file(const std::filesystem::path& filepath)
	{
		std::ifstream file(filepath.c_str(), std::ios::binary | std::ios::ate);

		if (!file.is_open())
		{
			return std::unexpected(file_error::file_not_found);
		}

		size_t size = file.tellg();
		file.seekg(0);
		if (size <= 0)
		{
			return std::unexpected(file_error::invalid_filesize);
		}

		std::vector<char> file_buffer(size);
		file.read(file_buffer.data(), file_buffer.size());

		file.close();

		return file_buffer;
	}
}
