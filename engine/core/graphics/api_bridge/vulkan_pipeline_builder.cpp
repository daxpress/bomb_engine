#include "core/graphics/api_bridge/vulkan_pipeline_builder.h"

namespace core::graphics::api
{
	VulkanPipelineBuilder::VulkanPipelineBuilder(vk::Device device)
		: m_device(device)
	{
		set_default_values();
	}

	vk::ResultValue<vk::Pipeline> VulkanPipelineBuilder::build_graphics()
	{
		vk::PipelineViewportStateCreateInfo viewport(vk::PipelineViewportStateCreateFlagBits(), m_viewport, m_scissor);
		vk::PipelineColorBlendStateCreateInfo color_blend(vk::PipelineColorBlendStateCreateFlags(), false, vk::LogicOp::eClear, m_color_blend_attachment);

		vk::GraphicsPipelineCreateInfo create_info(
			m_pipeline_create_flags,
			m_shaders,
			&m_vertex_input,
			&m_input_assembly,
			&m_tessellation,
			&viewport,
			&m_rasterization,
			&m_multisample,
			&m_depth_stencil,
			&color_blend,
			&m_dynamic_state,
			m_pipeline_layout,
			m_render_pass,
			m_subpass,
			nullptr,
			0,
			&m_rendering_create_info
			);

		return m_device.createGraphicsPipeline(m_pipeline_cache, create_info);
		
	}

	vk::ResultValue<vk::Pipeline> VulkanPipelineBuilder::build_compute()
	{
		vk::ComputePipelineCreateInfo create_info(m_pipeline_create_flags, m_shaders[0], m_pipeline_layout);
		return m_device.createComputePipeline(m_pipeline_cache, create_info);
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::add_shader(const vk::ShaderModule shader, const vk::ShaderStageFlagBits stage, const char* entry_point)
	{
		vk::PipelineShaderStageCreateInfo create_info{};
		create_info.module = shader;
		create_info.stage = stage;
		create_info.pName = entry_point;
		m_shaders.push_back(create_info);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_cache(const vk::PipelineCache cache)
	{
		m_pipeline_cache = cache;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_layout(const vk::PipelineLayout layout)
	{
		m_pipeline_layout = layout;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_create_flags(const vk::PipelineCreateFlagBits flags)
	{
		m_pipeline_create_flags = flags;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_vertex_input_state(
		const std::vector<vk::VertexInputBindingDescription> input_bindings,
		const std::vector<vk::VertexInputAttributeDescription> input_attributes)
	{
		m_vertex_input = vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlagBits(), input_bindings, input_attributes);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_input_assembly_state(const vk::PrimitiveTopology topology, bool enable_primitive_restart)
	{
		m_input_assembly = vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlagBits(), topology, enable_primitive_restart);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_depth(bool depth_test, bool depth_write)
	{
		m_depth_stencil.depthTestEnable = depth_test;
		m_depth_stencil.depthWriteEnable = depth_write;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_depth_compare_operation(vk::CompareOp operation)
	{
		m_depth_stencil.depthCompareOp = operation;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_depth_bounds(bool enable, const float min_bounds, const float max_bounds)
	{
		m_depth_stencil.depthBoundsTestEnable = enable;
		m_depth_stencil.minDepthBounds = min_bounds;
		m_depth_stencil.maxDepthBounds = max_bounds;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_stencil(bool enable, vk::StencilOpState front, vk::StencilOpState back)
	{
		m_depth_stencil.stencilTestEnable = enable;
		m_depth_stencil.front = front;
		m_depth_stencil.back = back;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_vertex_input()
	{
		// TODO: implementation
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_dynamic_states(std::vector<vk::DynamicState> dynamic_states)
	{
		m_dynamic_state = vk::PipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlagBits(), dynamic_states);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_viewport_size(const float width, const float height)
	{
		m_viewport.width = width;
		m_viewport.height = height;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_viewport_position(const float x, const float y)
	{
		m_viewport.x = x;
		m_viewport.y = y;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_viewport_depth(const float min_depth, const float max_depth)
	{
		m_viewport.minDepth = min_depth;
		m_viewport.maxDepth = max_depth;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_scissor_offset(const float offset_x, const float offset_y)
	{
		m_scissor.offset = vk::Offset2D(offset_x, offset_y);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_scissor_extent(const uint32_t width, const uint32_t height)
	{
		m_scissor.extent = vk::Extent2D{ width, height };
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_polygon_mode(vk::PolygonMode polygon_mode)
	{
		m_rasterization.polygonMode = polygon_mode;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_cull_mode(vk::CullModeFlagBits cull_mode)
	{
		m_rasterization.cullMode = cull_mode;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_front_face(vk::FrontFace front_face)
	{
		m_rasterization.frontFace = front_face;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_depth_bias(bool enable_depth_bias, const float constant_factor, const float clamp, const float slope_factor)
	{
		m_rasterization.depthBiasEnable = enable_depth_bias;
		m_rasterization.depthBiasConstantFactor = constant_factor;
		m_rasterization.depthBiasClamp = clamp;
		m_rasterization.depthBiasSlopeFactor = slope_factor;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::enable_depth_clamp(bool enable)
	{
		m_rasterization.depthClampEnable = enable;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::enable_rasterizer_discard(bool enable)
	{
		m_rasterization.rasterizerDiscardEnable = enable;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::enable_sample_shading(bool enable)
	{
		m_multisample.sampleShadingEnable = enable;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_rasterization_samples(const vk::SampleCountFlagBits samples)
	{
		m_multisample.rasterizationSamples = samples;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::enable_color_blend(bool enable)
	{
		m_color_blend_attachment.blendEnable = enable;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_color_write_mask(vk::ColorComponentFlags mask)
	{
		m_color_blend_attachment.colorWriteMask = mask;
		return *this;
	}

	VulkanPipelineBuilder::~VulkanPipelineBuilder()
	{
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_color_attachment_format(vk::Format format)
	{
		m_color_attachment_format = format;
		m_rendering_create_info.setColorAttachmentFormats(m_color_attachment_format);
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_render_pass(vk::RenderPass render_pass)
	{
		m_render_pass = render_pass;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_render_pass(vk::RenderPass render_pass)
	{
		// TODO: insert return statement here
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::set_depth_attachment_format(vk::Format format)
	{
		m_rendering_create_info.setDepthAttachmentFormat(format);
		return *this;
	}

	void VulkanPipelineBuilder::clear()
	{
		m_pipeline_cache = nullptr;
		m_pipeline_layout = nullptr;
		m_shaders.clear();
		m_pipeline_create_flags = vk::PipelineCreateFlagBits();
		m_vertex_input = vk::PipelineVertexInputStateCreateInfo{};
		m_input_assembly = vk::PipelineInputAssemblyStateCreateInfo{};
		m_tessellation = vk::PipelineTessellationStateCreateInfo{};
		m_rasterization = vk::PipelineRasterizationStateCreateInfo{};
		m_color_blend_attachment = vk::PipelineColorBlendAttachmentState{};
		m_dynamic_state = vk::PipelineDynamicStateCreateInfo{};
		m_multisample = vk::PipelineMultisampleStateCreateInfo{};
		m_depth_stencil = vk::PipelineDepthStencilStateCreateInfo{};
		m_viewport = vk::Viewport{};
		m_scissor = vk::Rect2D{};
		m_color_attachment_format = vk::Format::eUndefined;
		m_render_pass = nullptr;
		m_subpass = 0;
		m_rendering_create_info = vk::PipelineRenderingCreateInfo{};
	}

	void VulkanPipelineBuilder::set_default_values()
	{
		m_pipeline_cache = nullptr;
		m_pipeline_layout = nullptr;
		m_shaders.clear();
		m_pipeline_create_flags = vk::PipelineCreateFlagBits();
		default_vertex_input();
		default_input_assembly();
		default_tessellation();
		default_rasterization();
		default_color_blend_attachment();
		default_dynamic_state();
		default_multisample();
		default_depth_stencil();
		default_rendering();
		m_viewport = vk::Viewport{};
		m_scissor = vk::Rect2D{};
		m_color_attachment_format = vk::Format::eUndefined;
		m_render_pass = nullptr;
		m_subpass = 0;
	}

	void VulkanPipelineBuilder::default_tessellation()
	{
		m_tessellation = vk::PipelineTessellationStateCreateInfo{};
	}

	void VulkanPipelineBuilder::default_input_assembly()
	{
		m_input_assembly = vk::PipelineInputAssemblyStateCreateInfo{};
		m_input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
		m_input_assembly.primitiveRestartEnable = false;
	}

	void VulkanPipelineBuilder::default_vertex_input()
	{
		m_vertex_input = vk::PipelineVertexInputStateCreateInfo{};
	}

	void VulkanPipelineBuilder::default_rasterization()
	{
		m_rasterization = vk::PipelineRasterizationStateCreateInfo{};
		m_rasterization.depthClampEnable = false;
		m_rasterization.rasterizerDiscardEnable = false;
		m_rasterization.polygonMode = vk::PolygonMode::eFill;
		m_rasterization.lineWidth = 1.0f;
		m_rasterization.cullMode = vk::CullModeFlagBits::eBack;
		m_rasterization.frontFace = vk::FrontFace::eCounterClockwise;
		m_rasterization.depthBiasEnable = false;
	}

	void VulkanPipelineBuilder::default_color_blend_attachment()
	{
		m_color_blend_attachment = vk::PipelineColorBlendAttachmentState{};
		m_color_blend_attachment.colorWriteMask = 
			vk::ColorComponentFlagBits::eA | 
			vk::ColorComponentFlagBits::eR | 
			vk::ColorComponentFlagBits::eG | 
			vk::ColorComponentFlagBits::eB;
		m_color_blend_attachment.blendEnable = true;
		m_color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		m_color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		m_color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		m_color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		m_color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;
	}

	void VulkanPipelineBuilder::default_dynamic_state()
	{
		m_dynamic_state = vk::PipelineDynamicStateCreateInfo{};
	}

	void VulkanPipelineBuilder::default_multisample()
	{
		m_multisample = vk::PipelineMultisampleStateCreateInfo{};
		m_multisample.sampleShadingEnable = false;
		m_multisample.rasterizationSamples = vk::SampleCountFlagBits::e1;
	}

	void VulkanPipelineBuilder::default_depth_stencil()
	{
		m_depth_stencil = vk::PipelineDepthStencilStateCreateInfo{};
		m_depth_stencil.depthTestEnable = true;
		m_depth_stencil.depthWriteEnable = true;
		m_depth_stencil.depthCompareOp = vk::CompareOp::eLess;
		m_depth_stencil.depthBoundsTestEnable = false;
		
		m_depth_stencil.stencilTestEnable = false;
	}

	void VulkanPipelineBuilder::default_rendering()
	{
		m_rendering_create_info = vk::PipelineRenderingCreateInfo{};
		m_color_attachment_format = vk::Format::eUndefined;
		m_rendering_create_info.setColorAttachmentFormats(m_color_attachment_format);
		m_rendering_create_info.setDepthAttachmentFormat(vk::Format::eUndefined);
	}
}