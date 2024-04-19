#pragma once

#include "core/bomb_engine.h"

#include <vector>
#include <string>

namespace bomb_engine
{
	struct BOMB_ENGINE_API UniformBufferData
	{
		std::string name;
		uint32_t binding;
	};

	enum class E_SHADER_STAGE
	{
		VERTEX = 0,
		FRAGMENT,
		COMPUTE,
		TASK,
		MESH
	};

	class BOMB_ENGINE_API SPIRVShader
	{
	public:
		SPIRVShader(const std::vector<unsigned char>& spirv_binary);

		inline const std::vector<uint32_t>& get_data() const { return m_data; }
		inline const E_SHADER_STAGE get_stage() const { return m_stage; }
	private:

		std::vector<uint32_t> m_data;
		E_SHADER_STAGE m_stage;
		std::vector<UniformBufferData> m_uniform_buffers;
	};
}