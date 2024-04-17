#include "file_helper.h"

#include <fstream>


namespace bomb_engine::file_helper
{
	std::expected<filedata, file_error> load_file(const std::filesystem::path& filepath)
	{
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		if (!file)
		{
			return std::unexpected(file_error::file_not_found);
		}

		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);
		size -= file.tellg();
		if (size <= 0)
		{
			return std::unexpected(file_error::invalid_filesize);
		}

		filedata file_buffer(size);
		if (!file.read(reinterpret_cast<char*>(file_buffer.data()), file_buffer.size()))
		{
			return std::unexpected(file_error::read_error);
		}

		return file_buffer;
	}
}
