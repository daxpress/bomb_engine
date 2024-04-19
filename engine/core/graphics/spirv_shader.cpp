#include "core/graphics/spirv_shader.h"

#include <spirv_cross/spirv_cross.hpp>

namespace bomb_engine
{
	SPIRVShader::SPIRVShader(const std::vector<unsigned char>& spirv_binary)
	{
		m_data = std::vector<uint32_t>(spirv_binary.begin(), spirv_binary.end());
		spirv_cross::Compiler compiler(m_data);
		
		auto resources = compiler.get_shader_resources();
		for (const auto& ubo : resources.uniform_buffers)
		{
			UniformBufferData data{};
			data.name = ubo.name;
			data.binding = compiler.get_decoration(ubo.id, spv::DecorationBinding);
			auto set = compiler.get_decoration(ubo.id, spv::DecorationDescriptorSet);
			auto location = compiler.get_decoration(ubo.id, spv::DecorationLocation);
		}

		for (const auto& constant : resources.push_constant_buffers)
		{
			auto name = constant.name;
		}

		//for (const auto& input : resources.stage_inputs)
		//{

		//}

		//for (const auto& output : resources.stage_outputs)
		//{

		//}
	}
}