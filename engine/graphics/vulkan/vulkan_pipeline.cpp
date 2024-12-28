// #include "vulkan_pipeline.h"
// #include "file_helper.h"
//
// namespace BE_NAMESPACE
// {
// Pipeline::Pipeline(
//     vk::Device device, vk::Extent2D swapchainExtent, vk::SampleCountFlagBits msaaSamples
// )
//     : m_device(device)
// {
//     m_renderPass = createRenderPass(vk::Format::eB8G8R8A8Srgb, vk::Format::eD32SfloatS8Uint);
//     m_pipelineLayout = createPipelineLayout();
//     m_pipeline = createGraphicsPipeline(swapchainExtent, msaaSamples);
// }
//
// Pipeline::~Pipeline()
// {
//     m_device.destroyPipeline(m_pipeline);
//     m_device.destroyPipelineLayout(m_pipelineLayout);
//     m_device.destroyRenderPass(m_renderPass);
// }
//
// vk::ShaderModule Pipeline::createShaderModule(const std::vector<char>& code)
// {
//     vk::ShaderModuleCreateInfo createInfo{};
//     createInfo.codeSize = code.size();
//     createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
//
//     return m_device.createShaderModule(createInfo);
// }
//
// vk::PipelineLayout Pipeline::createPipelineLayout()
// {
//     vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
//     return m_device.createPipelineLayout(pipelineLayoutInfo);
// }
//
// vk::RenderPass Pipeline::createRenderPass(vk::Format swapchainImageFormat, vk::Format depthFormat)
// {
//     vk::AttachmentDescription colorAttachment{};
//     colorAttachment.format = swapchainImageFormat;
//     colorAttachment.samples = vk::SampleCountFlagBits::e1;
//     colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
//     colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
//     colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
//     colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
//     colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
//     colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
//
//     vk::AttachmentReference colorAttachmentRef{};
//     colorAttachmentRef.attachment = 0;
//     colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
//
//     vk::SubpassDescription subpass{};
//     subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
//     subpass.colorAttachmentCount = 1;
//     subpass.pColorAttachments = &colorAttachmentRef;
//
//     vk::RenderPassCreateInfo renderPassInfo{};
//     renderPassInfo.attachmentCount = 1;
//     renderPassInfo.pAttachments = &colorAttachment;
//     renderPassInfo.subpassCount = 1;
//     renderPassInfo.pSubpasses = &subpass;
//
//     return m_device.createRenderPass(renderPassInfo);
// }
//
// vk::Pipeline Pipeline::createGraphicsPipeline(
//     vk::Extent2D swapchainExtent, vk::SampleCountFlagBits msaaSamples
// )
// {
//     auto vertShaderCode = file_helper::load_file("shaders/vertex.spv");
//     auto fragShaderCode = file_helper::load_file("shaders/fragment.spv");
//
//     vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
//     vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);
//
//     vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
//     vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
//     vertShaderStageInfo.module = vertShaderModule;
//     vertShaderStageInfo.pName = "main";
//
//     vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
//     fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
//     fragShaderStageInfo.module = fragShaderModule;
//     fragShaderStageInfo.pName = "main";
//
//     vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
//
//     auto bindingDescription = VertexData::get_binding_description();
//     auto attributeDescriptions = VertexData::get_attribute_descriptions();
//
//     vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
//     vertexInputInfo.vertexBindingDescriptionCount = 1;
//     vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//     vertexInputInfo.vertexAttributeDescriptionCount =
//         static_cast<uint32_t>(attributeDescriptions.size());
//     vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//     vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
//     inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
//     inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//     vk::Viewport viewport{};
//     viewport.x = 0.0f;
//     viewport.y = 0.0f;
//     viewport.width = static_cast<float>(swapchainExtent.width);
//     viewport.height = static_cast<float>(swapchainExtent.height);
//     viewport.minDepth = 0.0f;
//     viewport.maxDepth = 1.0f;
//
//     vk::Rect2D scissor{};
//     scissor.offset = {0, 0};
//     scissor.extent = swapchainExtent;
//
//     vk::PipelineViewportStateCreateInfo viewportState{};
//     viewportState.viewportCount = 1;
//     viewportState.pViewports = &viewport;
//     viewportState.scissorCount = 1;
//     viewportState.pScissors = &scissor;
//
//     vk::PipelineRasterizationStateCreateInfo rasterizer{};
//     rasterizer.depthClampEnable = VK_FALSE;
//     rasterizer.rasterizerDiscardEnable = VK_FALSE;
//     rasterizer.polygonMode = vk::PolygonMode::eFill;
//     rasterizer.lineWidth = 1.0f;
//     rasterizer.cullMode = vk::CullModeFlagBits::eBack;
//     rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
//     rasterizer.depthBiasEnable = VK_FALSE;
//
//     vk::PipelineMultisampleStateCreateInfo multisampling{};
//     multisampling.sampleShadingEnable = VK_FALSE;
//     multisampling.rasterizationSamples = msaaSamples;
//
//     vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
//     colorBlendAttachment.colorWriteMask =
//         vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
//         vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
//     colorBlendAttachment.blendEnable = VK_FALSE;
//
//     vk::PipelineColorBlendStateCreateInfo colorBlending{};
//     colorBlending.logicOpEnable = VK_FALSE;
//     colorBlending.logicOp = vk::LogicOp::eCopy;
//     colorBlending.attachmentCount = 1;
//     colorBlending.pAttachments = &colorBlendAttachment;
//     colorBlending.blendConstants[0] = 0.0f;
//     colorBlending.blendConstants[1] = 0.0f;
//     colorBlending.blendConstants[2] = 0.0f;
//     colorBlending.blendConstants[3] = 0.0f;
//
//     vk::GraphicsPipelineCreateInfo pipelineInfo{};
//     pipelineInfo.stageCount = 2;
//     pipelineInfo.pStages = shaderStages;
//     pipelineInfo.pVertexInputState = &vertexInputInfo;
//     pipelineInfo.pInputAssemblyState = &inputAssembly;
//     pipelineInfo.pViewportState = &viewportState;
//     pipelineInfo.pRasterizationState = &rasterizer;
//     pipelineInfo.pMultisampleState = &multisampling;
//     pipelineInfo.pColorBlendState = &colorBlending;
//     pipelineInfo.layout = m_pipelineLayout;
//     pipelineInfo.renderPass = m_renderPass;
//     pipelineInfo.subpass = 0;
//
//     vk::Pipeline pipeline = m_device.createGraphicsPipeline(nullptr, pipelineInfo).value;
//
//     m_device.destroyShaderModule(vertShaderModule);
//     m_device.destroyShaderModule(fragShaderModule);
//
//     return pipeline;
// }
// }  // namespace BE_NAMESPACE