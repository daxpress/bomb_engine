// #pragma once
//
// namespace BE_NAMESPACE {
//
// class Pipeline {
// public:
//     Pipeline(vk::Device device, vk::Extent2D swapchainExtent, vk::SampleCountFlagBits msaaSamples);
//     ~Pipeline();
//
//     vk::Pipeline getPipeline() const { return m_pipeline; }
//     vk::PipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
//     vk::RenderPass getRenderPass() const { return m_renderPass; }
//
// private:
//     vk::Device m_device;
//     vk::Pipeline m_pipeline;
//     vk::PipelineLayout m_pipelineLayout;
//     vk::RenderPass m_renderPass;
//
//     vk::ShaderModule createShaderModule(const std::vector<char>& code);
//     vk::PipelineLayout createPipelineLayout();
//     vk::RenderPass createRenderPass(vk::Format swapchainImageFormat, vk::Format depthFormat);
//     vk::Pipeline createGraphicsPipeline(vk::Extent2D swapchainExtent, vk::SampleCountFlagBits msaaSamples);
// };
//
// } // BE_NAMESPACE
