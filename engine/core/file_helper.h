#pragma once

#include "bomb_engine.h"

#include <vector>
#include <filesystem>
#include <expected>

namespace bomb_engine::file_helper
{
	enum class file_error
	{
		file_not_found = 0,
		invalid_filesize,
		read_error,
	};

	using filedata = std::vector<std::byte>;

	BOMB_ENGINE_API std::expected<filedata, file_error> load_file(const std::filesystem::path& filepath);
}