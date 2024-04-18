#pragma once

#include <vulkan/vulkan.hpp>
#include <map>
#include <unordered_map>

namespace bomb_engine
{

	class VulkanRenderPassBuilder
	{
	public:

		enum class E_AttachmentType
		{
			E_COLOR,
			E_DEPTH_STENCIL,
			E_RESOLVE,
			E_INPUT,
			E_PRESERVE
		};

		struct VulkanSubpassInfo
		{
			vk::PipelineBindPoint bind_point;
			std::vector<vk::AttachmentReference> color_refs;
			std::vector<vk::AttachmentReference> input_refs;
			std::vector<vk::AttachmentReference> resolve_refs;
			std::vector<uint32_t> preserve_refs;
			vk::AttachmentReference depth_stencil_ref;
		};

	public:

		VulkanRenderPassBuilder(vk::Device device);

		vk::RenderPass build();
		void clear();
		VulkanRenderPassBuilder& add_attachment(const uint32_t attachment_number, const vk::ImageLayout layout, const std::string name);
		VulkanRenderPassBuilder& set_attachment_format(vk::Format format);
		VulkanRenderPassBuilder& set_attachment_samples(vk::SampleCountFlagBits samples);
		VulkanRenderPassBuilder& set_attachment_load_op(vk::AttachmentLoadOp load_op);
		VulkanRenderPassBuilder& set_attachment_store_op(vk::AttachmentStoreOp store_op);
		VulkanRenderPassBuilder& set_attachment_stencil_load_op(vk::AttachmentLoadOp load_op);
		VulkanRenderPassBuilder& set_attachment_stencil_store_op(vk::AttachmentStoreOp store_op);
		VulkanRenderPassBuilder& set_initial_layout(vk::ImageLayout layout);
		VulkanRenderPassBuilder& set_final_layout(vk::ImageLayout layout);

		VulkanRenderPassBuilder& add_subpass();
		VulkanRenderPassBuilder& set_subpass_bind_point(vk::PipelineBindPoint bind_point);
		VulkanRenderPassBuilder& set_subpass_color_attachments(std::vector<std::string> names_ordered);
		VulkanRenderPassBuilder& set_subpass_input_attachments(std::vector<std::string> names_ordered);
		VulkanRenderPassBuilder& set_subpass_resolve_attachments(std::vector<std::string> names_ordered);
		VulkanRenderPassBuilder& set_subpass_preserve_attachments(std::vector<uint32_t> indices);
		VulkanRenderPassBuilder& set_subpass_depth_stencil_attachment(std::string name);

		VulkanRenderPassBuilder& add_subpass_dependency();
		VulkanRenderPassBuilder& set_dependency_subpasses(uint32_t src_subpass, uint32_t dst_subpass);
		VulkanRenderPassBuilder& set_dependency_stage_masks(vk::PipelineStageFlagBits src, vk::PipelineStageFlagBits dst);
		VulkanRenderPassBuilder& set_dependency_access_masks(vk::AccessFlagBits src, vk::AccessFlagBits dst);

	private:
		vk::Device m_device;

		vk::AttachmentDescription* m_current_attachment = nullptr;
		VulkanSubpassInfo* m_current_subpass = nullptr;
		vk::SubpassDependency* m_current_dependency = nullptr;

		std::vector<vk::AttachmentDescription> m_attachments;

		std::map<std::string, vk::AttachmentReference> m_references;

		std::vector<VulkanSubpassInfo> m_subpass_infos;
		std::vector<vk::SubpassDependency> m_dependencies;
	};
}