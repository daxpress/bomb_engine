#include "core/graphics/vulkan/vulkan_renderpass_builder.h"

namespace bomb_engine
{
	VulkanRenderPassBuilder::VulkanRenderPassBuilder(vk::Device device) :
		m_device(device)
	{
	}
	vk::RenderPass VulkanRenderPassBuilder::build()
	{
		std::vector<vk::SubpassDescription> subpasses;
		for (const auto& info : m_subpass_infos)
		{
			vk::SubpassDescription desc{};
			desc.pipelineBindPoint = info.bind_point;
			desc.setColorAttachments(info.color_refs);
			desc.setPDepthStencilAttachment(&info.depth_stencil_ref);
			desc.setInputAttachments(info.input_refs);
			desc.setResolveAttachments(info.resolve_refs);
			desc.setPreserveAttachments(info.preserve_refs);
		}

		vk::RenderPassCreateInfo create_info{};
		create_info.setAttachments(m_attachments);
		create_info.setDependencies(m_dependencies);
		create_info.setSubpasses(subpasses);

		return m_device.createRenderPass(create_info);
	}
	void VulkanRenderPassBuilder::clear()
	{
		m_current_attachment = nullptr;
		m_current_dependency = nullptr;
		m_current_subpass = nullptr;

		m_attachments.clear();
		m_references.clear();
		m_dependencies.clear();
		m_subpass_infos.clear();
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::add_attachment(const uint32_t attachment_number, const vk::ImageLayout layout, const std::string name)
	{
		vk::AttachmentDescription attachment{};
		attachment.format = vk::Format::eUndefined;
		attachment.samples = vk::SampleCountFlagBits::e1;
		attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
		attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachment.initialLayout = vk::ImageLayout::eUndefined;
		attachment.finalLayout = vk::ImageLayout::eUndefined;

		m_current_attachment = &m_attachments.emplace_back(attachment);

		vk::AttachmentReference reference(attachment_number, layout);
		m_references.emplace(name, reference);
		
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_attachment_format(vk::Format format)
	{
		m_current_attachment->format = format;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_attachment_samples(vk::SampleCountFlagBits samples)
	{
		m_current_attachment->samples = samples;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_attachment_load_op(vk::AttachmentLoadOp load_op)
	{
		m_current_attachment->loadOp = load_op;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_attachment_store_op(vk::AttachmentStoreOp store_op)
	{
		m_current_attachment->storeOp = store_op;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_attachment_stencil_load_op(vk::AttachmentLoadOp load_op)
	{
		m_current_attachment->stencilLoadOp = load_op;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_attachment_stencil_store_op(vk::AttachmentStoreOp store_op)
	{
		m_current_attachment->stencilStoreOp = store_op;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_initial_layout(vk::ImageLayout layout)
	{
		m_current_attachment->initialLayout = layout;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_final_layout(vk::ImageLayout layout)
	{
		m_current_attachment->finalLayout = layout;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::add_subpass()
	{
		VulkanSubpassInfo info{};
		info.bind_point = vk::PipelineBindPoint::eGraphics;
		m_current_subpass = &m_subpass_infos.emplace_back(info);
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_subpass_bind_point(vk::PipelineBindPoint bind_point)
	{
		m_current_subpass->bind_point = bind_point;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_subpass_color_attachments(std::vector<std::string> names_ordered)
	{
		for (const auto& name : names_ordered)
		{
			m_current_subpass->color_refs.push_back(m_references[name]);
		}
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_subpass_input_attachments(std::vector<std::string> names_ordered)
	{
		for (const auto& name : names_ordered)
		{
			m_current_subpass->input_refs.push_back(m_references[name]);
		}
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_subpass_resolve_attachments(std::vector<std::string> names_ordered)
	{
		for (const auto& name : names_ordered)
		{
			m_current_subpass->resolve_refs.push_back(m_references[name]);
		} 
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_subpass_preserve_attachments(std::vector<uint32_t> indices)
	{
		for (const auto& index : indices)
		{
			m_current_subpass->preserve_refs.push_back(index);
		}
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_subpass_depth_stencil_attachment(std::string name)
	{
		m_current_subpass->depth_stencil_ref = m_references[name];
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::add_subpass_dependency()
	{
		vk::SubpassDependency dep{};
		dep.srcSubpass = vk::SubpassExternal;
		dep.dstSubpass = 0;
		dep.srcStageMask = vk::PipelineStageFlagBits::eNone;
		dep.dstStageMask = vk::PipelineStageFlagBits::eNone;
		dep.srcAccessMask = vk::AccessFlagBits::eNone;
		dep.dstAccessMask = vk::AccessFlagBits::eNone;
		m_current_dependency = &m_dependencies.emplace_back(dep);
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_dependency_subpasses(uint32_t src_subpass, uint32_t dst_subpass)
	{
		m_current_dependency->srcSubpass = src_subpass;
		m_current_dependency->dstSubpass = dst_subpass;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_dependency_stage_masks(vk::PipelineStageFlagBits src, vk::PipelineStageFlagBits dst)
	{
		m_current_dependency->srcStageMask = src;
		m_current_dependency->dstStageMask = dst;
		return *this;
	}
	VulkanRenderPassBuilder& VulkanRenderPassBuilder::set_dependency_access_masks(vk::AccessFlagBits src, vk::AccessFlagBits dst)
	{
		m_current_dependency->srcAccessMask = src;
		m_current_dependency->dstAccessMask = dst;
		return *this;
	}
}