#pragma once

#include "core/graphics/graphics_pipeline_type.h"

#include <vulkan/vulkan.hpp>
#include <vector>

namespace core::graphics::api
{
	class VulkanPipelineBuilder
	{
	public:
		VulkanPipelineBuilder(vk::Device device);

		vk::ResultValue<vk::Pipeline> build_graphics();
		vk::ResultValue<vk::Pipeline> build_compute();
		void clear();
		void set_default_values();
		VulkanPipelineBuilder& add_shader(const vk::ShaderModule shader, const vk::ShaderStageFlagBits stage, const char* entry_point = "main");
		VulkanPipelineBuilder& set_cache(const vk::PipelineCache cache);
		VulkanPipelineBuilder& set_layout(const vk::PipelineLayout layout);
		VulkanPipelineBuilder& set_create_flags(const vk::PipelineCreateFlagBits flags);
		VulkanPipelineBuilder& set_vertex_input_state(
			const std::vector<vk::VertexInputBindingDescription> input_bindings,
			const std::vector<vk::VertexInputAttributeDescription> input_attributes);
		VulkanPipelineBuilder& set_input_assembly_state(const vk::PrimitiveTopology topology, bool enable_primitive_restart = false);
		VulkanPipelineBuilder& set_depth(bool enable_depth_test, bool enable_depth_write);
		VulkanPipelineBuilder& set_depth_compare_operation(vk::CompareOp operation = vk::CompareOp::eLess);
		VulkanPipelineBuilder& set_depth_bounds(bool enable, const float min_bounds = 0.0f, const float max_bounds = 1.0f);
		VulkanPipelineBuilder& set_stencil(bool enable, vk::StencilOpState front, vk::StencilOpState back);
		VulkanPipelineBuilder& set_vertex_input(); // TODO: implement the shit after you figured out how to (due to bindless)
		VulkanPipelineBuilder& set_dynamic_states(std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor});
		VulkanPipelineBuilder& set_viewport_size(const float width, const float height);
		VulkanPipelineBuilder& set_viewport_position(const float x = 0.0f, const float y = 0.0f);
		VulkanPipelineBuilder& set_viewport_depth(const float min_depth = 0.0f, const float max_depth = 1.0f);
		VulkanPipelineBuilder& set_scissor_offset(const float offset_x, const float offset_y);
		VulkanPipelineBuilder& set_scissor_extent(const uint32_t width, const uint32_t height);
		VulkanPipelineBuilder& set_polygon_mode(vk::PolygonMode polygon_mode);
		VulkanPipelineBuilder& set_cull_mode(vk::CullModeFlagBits cull_mode);
		VulkanPipelineBuilder& set_front_face(vk::FrontFace front_face);
		VulkanPipelineBuilder& set_depth_bias(bool enable_depth_bias, const float constant_factor, const float clamp, const float slope_factor);
		VulkanPipelineBuilder& enable_depth_clamp(bool enable);
		VulkanPipelineBuilder& enable_rasterizer_discard(bool enable);
		VulkanPipelineBuilder& enable_sample_shading(bool enable);
		VulkanPipelineBuilder& set_rasterization_samples(const vk::SampleCountFlagBits samples);
		VulkanPipelineBuilder& enable_color_blend(bool enable);
		VulkanPipelineBuilder& set_color_write_mask(vk::ColorComponentFlags mask);
		VulkanPipelineBuilder& set_color_attachment_format(vk::Format format);
		VulkanPipelineBuilder& set_depth_attachment_format(vk::Format format);

	private:
		void default_vertex_input();
		void default_input_assembly();
		void default_tessellation();
		void default_rasterization();
		void default_color_blend_attachment();
		void default_dynamic_state();
		void default_multisample();
		void default_depth_stencil();
		void default_rendering();


		~VulkanPipelineBuilder();

	private:
		vk::Device m_device;
		vk::PipelineCache m_pipeline_cache = nullptr;
		vk::PipelineLayout m_pipeline_layout = nullptr;
		std::vector<vk::PipelineShaderStageCreateInfo> m_shaders;
		vk::PipelineCreateFlags m_pipeline_create_flags;

		vk::PipelineVertexInputStateCreateInfo m_vertex_input;
		vk::PipelineInputAssemblyStateCreateInfo m_input_assembly;
		vk::PipelineTessellationStateCreateInfo m_tessellation;
		vk::PipelineRasterizationStateCreateInfo m_rasterization;
		vk::PipelineColorBlendAttachmentState m_color_blend_attachment;
		vk::PipelineDynamicStateCreateInfo m_dynamic_state;
		vk::PipelineMultisampleStateCreateInfo m_multisample;
		vk::PipelineDepthStencilStateCreateInfo m_depth_stencil;
		vk::PipelineRenderingCreateInfo m_rendering_create_info;
		vk::Viewport m_viewport;
		vk::Rect2D m_scissor;
		vk::Format m_color_attachment_format;
		vk::RenderPass m_render_pass = nullptr;
		uint32_t m_subpass = 0;
	};
}