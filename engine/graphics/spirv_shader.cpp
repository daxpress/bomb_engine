#include "spirv_shader.h"

#include <spirv_glsl.hpp>

namespace bomb_engine
{
	SPIRVShader::SPIRVShader(const std::vector<char>& spirv_binary)
	{
		m_data = std::vector<uint32_t>(spirv_binary.size() / sizeof(uint32_t));
		memcpy(m_data.data(), spirv_binary.data(), spirv_binary.size());

		spirv_cross::CompilerGLSL compiler(m_data);
		auto resources = compiler.get_shader_resources();
		m_uniform_buffers.reserve(resources.uniform_buffers.size());
		for (const auto& ubo : resources.uniform_buffers)
		{
			UniformBufferData data{};
			data.name = ubo.name;
			data.binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
			auto set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
			auto location = compiler.get_decoration(ubo.id, spv::DecorationLocation);
			m_uniform_buffers.push_back(data);
		}

		//for (const auto& constant : resources.push_constant_buffers)
		//{
		//	auto name = constant.name;
		//}

		//for (const auto& input : resources.stage_inputs)
		//{

		//}

		//for (const auto& output : resources.stage_outputs)
		//{

		//}
	}
}