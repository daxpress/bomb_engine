#include "vulkan/api_vulkan.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#include "file_helper.h"
#include "vertex_data.h"
#include "vulkan/api_vulkan_internal.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vulkan_statics.h"

// TODO: Replace exceptions with logs (can also throw exceptions but they must
// be engine-breaking!)

#pragma region vulkan instance helpers

auto validation_layers_supported(const std::vector<const char*>& validation_layers) -> bool
{
    // same tricks used with the other check above
    uint32_t layer_count;
    if (vk::enumerateInstanceLayerProperties(&layer_count, nullptr) != vk::Result::eSuccess)
    {
        return false;
    }
    std::vector<vk::LayerProperties> available_layers(layer_count);
    if (vk::enumerateInstanceLayerProperties(&layer_count, available_layers.data()) !=
        vk::Result::eSuccess)
    {
        return false;
    }

    for (const auto& layer : validation_layers)
    {
        bool layer_found = false;

        for (const auto& availableLayer : available_layers)
        {
            if (strcmp(layer, availableLayer.layerName) == 0)
            {
                layer_found = true;
            }
        }

        if (!layer_found)
        {
            return false;
        }
    }
    return true;
}

auto extensions_supported(const char** required_extensions, uint32_t count) -> bool
{
    uint32_t extensions_count = 0;
    if (vk::enumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr) !=
        vk::Result::eSuccess)
    {
        return false;
    }
    std::vector<vk::ExtensionProperties> extensions(extensions_count);
    // not sure why it doesn't like the hpp version with data in
    if (vk::enumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions.data()) !=
        vk::Result::eSuccess)
    {
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        bool found = false;
        for (const auto& avExtension : extensions)
        {
            if (strcmp(avExtension.extensionName, required_extensions[i]) == 0)
            {
                found = true;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

auto get_required_instance_extensions(bool enable_validation_layers) -> std::vector<const char*>
{
    uint32_t extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + extension_count);

    if (enable_validation_layers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

// The debug callback for the validation layers: it has to be either a function
// or a static method (no context)
VKAPI_ATTR auto VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) -> VkBool32
{
    auto severity = LogSeverity::Log;
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        severity = LogSeverity::Warning;
    }
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        severity = LogSeverity::Error;
    }

    Log(VulkanAPICategory, severity, "validation layer: {}", pCallbackData->pMessage);
    return VK_FALSE;
}

void fill_debug_utils_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& messenger_info)
{
    messenger_info.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
    messenger_info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                     vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    messenger_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                 vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
    messenger_info.pfnUserCallback = &debug_callback;
    // at the moment we keep it to null ,when we have a bit of framework we
    // might pass in something
    messenger_info.pUserData = nullptr;
}

static void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    auto renderer = reinterpret_cast<bomb_engine::APIVulkan*>(glfwGetWindowUserPointer(window));
    renderer->framebuffer_resized();
}
#pragma endregion vulkan instance helpers

namespace BE_NAMESPACE
{
APIVulkan::APIVulkan(Window& window, bool enable_validation_layers)
    : b_use_validation_layers(enable_validation_layers), m_window_ref(window)
{
    create_instance(window, enable_validation_layers);
    create_surface(window, m_surface);

    glfwSetWindowUserPointer(window.get_raw_window(), this);
    glfwSetFramebufferSizeCallback(window.get_raw_window(), framebuffer_resize_callback);

    m_physical_device = std::make_shared<vk::PhysicalDevice>(select_physical_device());
    m_msaa_samples = get_sample_count();

    m_device = std::make_shared<vk::Device>(create_logical_device(*m_physical_device));

    auto families = get_queue_families(*m_physical_device);
    m_graphics_queue =
        std::make_shared<vk::Queue>(m_device->getQueue(families.graphics.value(), 0));
    m_present_queue = std::make_shared<vk::Queue>(m_device->getQueue(families.present.value(), 0));
    m_transfer_queue =
        std::make_shared<vk::Queue>(m_device->getQueue(families.transfer.value(), 0));
    m_compute_queue = std::make_shared<vk::Queue>(m_device->getQueue(families.compute.value(), 0));

    m_swapchain_info = create_swapchain(*m_physical_device, m_surface, *m_device);

    // example pipeline related (we will create a sample scene rendered directly
    // from here to drive abstractions for scene and renderer's APIs)
    m_example_pipeline = create_example_pipeline();
    m_example_command_pool = std::make_shared<vk::CommandPool>(vulkan_statics::create_command_pool(
        *m_device, vk::CommandPoolCreateFlagBits::eResetCommandBuffer, families.graphics.value()
    ));
    create_color_resources(m_swapchain_info);
    create_depth_resources(m_swapchain_info);
    m_frame_buffers = create_frame_buffers(m_swapchain_info, m_example_renderpass);

    m_model = std::make_shared<Mesh>("assets/models/viking_room.obj");

    create_example_vb();
    create_example_ib();

    create_example_texture();

    m_example_sampler = vulkan_statics::image::create_image_sampler(
        *m_physical_device, *m_device, vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat
    );

    create_example_uniform_buffers();
    m_example_desc_pool =
        create_descriptor_pool(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT);
    m_example_desc_sets = create_descriptor_sets(MAX_FRAMES_IN_FLIGHT);
    populate_example_desc_sets();

    m_example_command_buffers = vulkan_statics::command_buffer::create_command_buffers(
        *m_device, *m_example_command_pool, vk::CommandBufferLevel::ePrimary, MAX_FRAMES_IN_FLIGHT
    );
    create_sync_objects();
}

APIVulkan::~APIVulkan()
{
    m_device->waitIdle();

    m_example_image.reset();
    m_device->destroySampler(m_example_sampler);

    // drop the buffers to destroy before the device is (current use, it WILL change)
    m_model_ib.reset();
    m_model_vb.reset();

    for (auto& ub : m_uniform_buffers)
    {
        ub.reset();
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_device->destroySemaphore(m_render_finished[i]);
        m_device->destroySemaphore(m_swapchain_image_available[i]);
        m_device->destroyFence(m_in_flight[i]);
    }

    m_device->freeCommandBuffers(*m_example_command_pool, m_example_command_buffers);

    m_device->destroyCommandPool(*m_example_command_pool);

    cleanup_swapchain(m_swapchain_info);

    m_device->destroyPipeline(m_example_pipeline);
    m_device->destroyRenderPass(m_example_renderpass);
    m_device->destroyPipelineLayout(m_example_layout);
    m_device->destroyDescriptorPool(m_example_desc_pool);
    m_device->destroyDescriptorSetLayout(m_example_descriptor_set_layout);
    m_device->destroy();
    vkDestroySurfaceKHR(*m_vulkan_instance, m_surface, nullptr);
}

void APIVulkan::draw_frame() { draw_example_frame(); }

void APIVulkan::create_instance(const Window& window, bool enable_validation_layers)
{
    vk::ApplicationInfo appInfo(
        window.get_title().c_str(),
        VK_MAKE_API_VERSION(0, 0, 1, 0),
        "Bomb Engine",
        VK_MAKE_API_VERSION(0, 0, 1, 0),
        vk::ApiVersion13,
        nullptr
    );

    // not using constructor here to allow flexibility for the validation layers
    vk::InstanceCreateInfo createinfo{};
    createinfo.sType = vk::StructureType::eInstanceCreateInfo;
    createinfo.pApplicationInfo = &appInfo;
    createinfo.pNext = nullptr;

    // get the glfw extensions required for Vulkan
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    if (!extensions_supported(glfw_extensions, glfw_extension_count))
    {
        throw std::runtime_error("required glfw extensions are not supported!");
    }

    // get the final list of all the extensions (conditionally adds in the
    // validation layers)
    auto extensions = get_required_instance_extensions(enable_validation_layers);
    createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createinfo.ppEnabledExtensionNames = extensions.data();

    if (enable_validation_layers && !validation_layers_supported(m_validation_layers))
    {
        throw std::runtime_error("requested validation layers are not supported!");
    }

    vk::DebugUtilsMessengerCreateInfoEXT messenger_info{};
    if (enable_validation_layers)
    {
        createinfo.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
        createinfo.ppEnabledLayerNames = m_validation_layers.data();
        fill_debug_utils_messenger_create_info(messenger_info);
        createinfo.pNext = &messenger_info;
    }
    else
    {
        createinfo.enabledLayerCount = 0;
        createinfo.ppEnabledLayerNames = nullptr;
    }

    m_vulkan_instance = vk::createInstanceUnique(createinfo);

    if (enable_validation_layers)
    {
        setup_debug_messenger(m_vulkan_instance, messenger_info);
    }
}

void APIVulkan::setup_debug_messenger(
    vk::UniqueInstance& instance, vk::DebugUtilsMessengerCreateInfoEXT& messenger_info
)
{
    // Vulkan hpp gives us a DispatchLoaderDynamic that basically loads the
    // requiired method for us
    static vk::DispatchLoaderDynamic dldi;
    dldi.init(*instance, vkGetInstanceProcAddr);
    auto messenger = instance->createDebugUtilsMessengerEXTUnique(messenger_info, nullptr, dldi);
}

void APIVulkan::create_surface(const Window& window, VkSurfaceKHR& surface)
{
    if (glfwCreateWindowSurface(*m_vulkan_instance, window.get_raw_window(), nullptr, &surface) !=
        VK_SUCCESS)
    {
        throw ::std::runtime_error("failed to create window surface!");
    }
}

auto APIVulkan::select_physical_device() -> vk::PhysicalDevice
{
    auto physical_devices = m_vulkan_instance->enumeratePhysicalDevices();
    if (physical_devices.empty())
    {
        throw std::runtime_error("failed to find a supported GPU!");
    }

    std::map<uint32_t, vk::PhysicalDevice, std::greater<>> device_ratings;

    for (const auto& p_device : physical_devices)
    {
        if (physical_device_is_suitable(p_device))
        {
            auto rating = rate_physical_device(p_device);
            device_ratings.emplace(rating, p_device);
        }
    }

    if (physical_devices.empty())
    {
        throw std::runtime_error("failed to find a supported GPU!");
    }

    Log(VulkanAPICategory,
        LogSeverity::Log,
        "Picked Device: {:s}",
        device_ratings.begin()->second.getProperties().deviceName);

    return device_ratings.begin()->second;
}

auto APIVulkan::physical_device_is_suitable(vk::PhysicalDevice physical_device) -> bool
{
    auto families = get_queue_families(physical_device);
    if (!families.is_complete())
    {
        return false;
    }

    if (!check_extensions_support(physical_device))
    {
        return false;
    }

    auto swapchain_details = VkSwapchainDetails(physical_device, m_surface);
    if (swapchain_details.formats.empty() || swapchain_details.present_modes.empty())
    {
        return false;
    }

    return true;
}

auto APIVulkan::rate_physical_device(vk::PhysicalDevice physical_device) -> uint32_t
{
    uint32_t score = 0;

    auto properties = physical_device.getProperties();
    auto memory_properties = physical_device.getMemoryProperties();
    // auto features = physical_device.getFeatures();

    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
    {
        score += 1000;
    }

    score += properties.limits.maxComputeSharedMemorySize;
    score += properties.limits.maxImageDimension2D + properties.limits.maxImageDimension3D;

    for (const auto& heap : memory_properties.memoryHeaps)
    {
        if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
        {
            // heap size is in bytes, we want it in the order of Mb to keep it
            // somewhat proportionate
            score += static_cast<uint32_t>(std::round(heap.size * 1e-6f));
        }
    }

    return score;
}

auto APIVulkan::get_queue_families(const vk::PhysicalDevice& physical_device) const
    -> VkQueueFamilyIndices
{
    static VkQueueFamilyIndices families{};

    if (families.is_complete())
    {
        // caching families as they will effectively not change
        return families;
    }

    auto const queue_families = physical_device.getQueueFamilyProperties();

    for (uint32_t family_index = 0; const auto& queue_family : queue_families)
    {
        if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            families.graphics = family_index;
        }

        if (physical_device.getSurfaceSupportKHR(family_index, m_surface))
        {
            families.present = family_index;
        }

        if (!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics) &&
            queue_family.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            families.transfer = family_index;
        }

        if (!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics) &&
            queue_family.queueFlags & vk::QueueFlagBits::eCompute)
        {
            families.compute = family_index;
        }

        if (families.is_complete())
        {
            break;
        }

        family_index++;
    }

    return families;
}

auto APIVulkan::check_extensions_support(vk::PhysicalDevice physical_device) -> bool
{
    auto available_extensions = physical_device.enumerateDeviceExtensionProperties();

    std::set<std::string> required_extensions{
        m_required_device_extensions.begin(), m_required_device_extensions.end()
    };

    for (const auto& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

auto APIVulkan::get_sample_count() -> vk::SampleCountFlagBits
{
    auto properties = m_physical_device->getProperties();
    auto counts =
        properties.limits.framebufferColorSampleCounts &
        properties.limits
            .framebufferDepthSampleCounts;  // consider checking stencil in addition to these

    if (counts & vk::SampleCountFlagBits::e64) return vk::SampleCountFlagBits::e64;
    if (counts & vk::SampleCountFlagBits::e32) return vk::SampleCountFlagBits::e32;
    if (counts & vk::SampleCountFlagBits::e16) return vk::SampleCountFlagBits::e16;
    if (counts & vk::SampleCountFlagBits::e8) return vk::SampleCountFlagBits::e8;
    if (counts & vk::SampleCountFlagBits::e4) return vk::SampleCountFlagBits::e4;
    if (counts & vk::SampleCountFlagBits::e2) return vk::SampleCountFlagBits::e2;
    return vk::SampleCountFlagBits::e1;
}

auto APIVulkan::create_logical_device(vk::PhysicalDevice physical_device) -> vk::Device
{
    auto families = get_queue_families(physical_device);

    std::set<uint32_t> unique_families = {
        families.graphics.value(),
        families.present.value(),
        families.transfer.value(),
        families.compute.value(),
    };

    // atm same priority for every queue, when we have a more complex rendering
    // solution I might change it
    std::array<float, 4> priorities{1.0f};

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};

    for (const auto& queue_family : unique_families)
    {
        vk::DeviceQueueCreateInfo queue_create_info(
            vk::DeviceQueueCreateFlags(), queue_family, priorities
        );
        queue_create_infos.push_back(queue_create_info);
    }

    // TODO: have a better features selection
    vk::PhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = vk::True;
    device_features.sampleRateShading = vk::True;

    vk::DeviceCreateInfo create_info(
        vk::DeviceCreateFlags(),
        queue_create_infos,
        nullptr,
        m_required_device_extensions,
        &device_features
    );

    if (b_use_validation_layers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
        create_info.ppEnabledLayerNames = m_validation_layers.data();
    }

    return physical_device.createDevice(create_info);
}

auto APIVulkan::create_swapchain(
    vk::PhysicalDevice physical_device,
    vk::SurfaceKHR surface,
    vk::Device device,
    vk::SwapchainKHR old_swapchain
) -> VkSwapchainInfo
{
    auto swapchain_details = VkSwapchainDetails(physical_device, surface);
    auto [format, color_space] = choose_swapchain_surface_format(swapchain_details.formats);
    auto present_mode = choose_swapchain_present_mode(swapchain_details.present_modes);
    auto extent = choose_swapchain_extent(swapchain_details.capabilities);

    uint32_t image_count = swapchain_details.capabilities.minImageCount + 1;
    if (swapchain_details.capabilities.maxImageCount > 0 &&
        image_count > swapchain_details.capabilities.maxImageCount)
    {
        image_count = swapchain_details.capabilities.maxImageCount;
    }

    auto indices = get_queue_families(physical_device);
    std::vector<uint32_t> swapchain_sharing_indicies{};
    uint32_t sharing_index_count = 0;
    auto sharing_mode = vk::SharingMode::eExclusive;
    if (indices.graphics != indices.present)
    {
        sharing_index_count = 2;
        swapchain_sharing_indicies = {indices.graphics.value(), indices.present.value()};
        sharing_mode = vk::SharingMode::eConcurrent;
    }

    vk::SwapchainCreateInfoKHR create_info(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        image_count,
        format,
        color_space,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        sharing_mode,
        sharing_index_count,
        swapchain_sharing_indicies.data(),
        swapchain_details.capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        present_mode,
        true,
        old_swapchain,
        nullptr
    );

    VkSwapchainInfo swapchain_info{};
    swapchain_info.swapchain = device.createSwapchainKHR(create_info);
    swapchain_info.extent = extent;
    swapchain_info.format = format;
    swapchain_info.images = device.getSwapchainImagesKHR(swapchain_info.swapchain);

    swapchain_info.image_views.reserve(swapchain_info.images.size());
    vk::ImageViewCreateInfo image_view_create_info(
        vk::ImageViewCreateFlags(),
        {},
        vk::ImageViewType::e2D,
        format,
        {},
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    );

    for (const auto& image : swapchain_info.images)
    {
        image_view_create_info.image = image;
        swapchain_info.image_views.push_back(device.createImageView(image_view_create_info));
    }

    return swapchain_info;
}

auto APIVulkan::choose_swapchain_surface_format(std::vector<vk::SurfaceFormatKHR> formats)
    -> vk::SurfaceFormatKHR
{
    for (const auto& format : formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return format;
        }
    }
    return formats[0];
}

auto APIVulkan::choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR>& present_modes)
    -> vk::PresentModeKHR
{
    for (const auto& present_mode : present_modes)
    {
        if (present_mode == vk::PresentModeKHR::eMailbox)  // to render frames as fast as
                                                           // possible
        {
            return present_mode;
        }
    }
    // guaranteed by spec
    return vk::PresentModeKHR::eFifo;
}

auto APIVulkan::choose_swapchain_extent(vk::SurfaceCapabilitiesKHR capabilities) -> vk::Extent2D
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(m_window_ref.get_raw_window(), &width, &height);

    vk::Extent2D extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    return std::clamp(extent, capabilities.minImageExtent, capabilities.maxImageExtent);
}

auto APIVulkan::create_shader_module(SPIRVShader& shader) -> vk::ShaderModule
{
    vk::ShaderModuleCreateInfo create_info(
        vk::ShaderModuleCreateFlagBits(), shader.get_bytes_count(), shader.get_data().data()
    );
    return m_device->createShaderModule(create_info);
}

auto APIVulkan::create_example_pipeline() -> vk::Pipeline
{
    SPIRVShader vertex, fragment;
    if (auto vertex_file = file_helper::load_file("shaders/vertex.spv"); vertex_file.has_value())
    {
        vertex = SPIRVShader(vertex_file.value());
    }
    else
    {
        throw std::runtime_error("error parsing file");
    }
    if (auto fragment_file = file_helper::load_file("shaders/fragment.spv");
        fragment_file.has_value())
    {
        fragment = SPIRVShader(fragment_file.value());
    }
    else
    {
        throw std::runtime_error("error parsing file");
    }
    auto vertex_shader = create_shader_module(vertex);
    auto fragment_shader = create_shader_module(fragment);

    vk::PipelineShaderStageCreateInfo vert_stage(
        vk::PipelineShaderStageCreateFlagBits(),
        vk::ShaderStageFlagBits::eVertex,
        vertex_shader,
        "main"
    );
    vk::PipelineShaderStageCreateInfo frag_stage(
        vk::PipelineShaderStageCreateFlagBits(),
        vk::ShaderStageFlagBits::eFragment,
        fragment_shader,
        "main"
    );

    std::array<vk::PipelineShaderStageCreateInfo, 2> stages{vert_stage, frag_stage};

    vk::PipelineDepthStencilStateCreateInfo depth_stencil(
        vk::PipelineDepthStencilStateCreateFlagBits(), true, true, vk::CompareOp::eLess
    );
    depth_stencil.depthBoundsTestEnable = false;
    depth_stencil.stencilTestEnable = false;

    auto binding_desc = vulkan_statics::mesh::get_binding_description();
    auto attrib_desc = vulkan_statics::mesh::get_attribute_descriptions();
    vk::PipelineVertexInputStateCreateInfo vert_input(
        vk::PipelineVertexInputStateCreateFlagBits(), binding_desc, attrib_desc
    );

    std::array<vk::DynamicState, 2> dynamic_states{
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamic_state_info(
        vk::PipelineDynamicStateCreateFlagBits(), dynamic_states
    );

    vk::Viewport viewport(
        0, 0, m_swapchain_info.extent.width, m_swapchain_info.extent.height, 0, 1
    );
    vk::Rect2D scissor({0, 0}, m_swapchain_info.extent);
    vk::PipelineViewportStateCreateInfo viewport_state(
        vk::PipelineViewportStateCreateFlagBits(), viewport, scissor
    );

    vk::PipelineInputAssemblyStateCreateInfo input_assembly(
        vk::PipelineInputAssemblyStateCreateFlagBits(), vk::PrimitiveTopology::eTriangleList, false
    );

    vk::PipelineRasterizationStateCreateInfo raster(
        vk::PipelineRasterizationStateCreateFlagBits(), false, false
    );                        // default is ok
    raster.lineWidth = 1.0f;  // it was complaining, probably default is 0
    raster.cullMode = vk::CullModeFlagBits::eBack;
    raster.frontFace = vk::FrontFace::eCounterClockwise;

    vk::PipelineMultisampleStateCreateInfo multisampling(
        vk::PipelineMultisampleStateCreateFlagBits(), m_msaa_samples, true, 0.20f
    );

    vk::PipelineColorBlendAttachmentState color_attachment(
        true,
        vk::BlendFactor::eSrcAlpha,
        vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );

    vk::PipelineColorBlendStateCreateInfo color_blend(
        vk::PipelineColorBlendStateCreateFlagBits(), false, vk::LogicOp::eClear, color_attachment
    );

    m_example_layout = create_example_pipeline_layout();

    m_example_renderpass = create_example_render_pass();

    vk::GraphicsPipelineCreateInfo pipeline(
        vk::PipelineCreateFlagBits(),
        stages,
        &vert_input,
        &input_assembly,
        nullptr,
        &viewport_state,
        &raster,
        &multisampling,
        &depth_stencil,
        &color_blend,
        &dynamic_state_info,
        m_example_layout,
        m_example_renderpass,
        0
    );

    auto pipeline_res = m_device->createGraphicsPipeline(nullptr, pipeline);
    if (pipeline_res.result != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    m_device->destroyShaderModule(vertex_shader);
    m_device->destroyShaderModule(fragment_shader);

    return pipeline_res.value;
}
auto APIVulkan::create_example_render_pass() -> vk::RenderPass
{
    vk::AttachmentDescription color_attachment(
        vk::AttachmentDescriptionFlagBits(),
        m_swapchain_info.format,
        m_msaa_samples,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    );

    vk::AttachmentDescription depth_attachment(
        vk::AttachmentDescriptionFlagBits(),
        DEPTH_FORMAT,
        m_msaa_samples,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    );

    vk::AttachmentDescription color_attachment_resolve(
        vk::AttachmentDescriptionFlagBits(),
        m_swapchain_info.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    );

    vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depth_stencil_ref(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    vk::AttachmentReference color_resolve_ref(2, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass(
        vk::SubpassDescriptionFlagBits(), vk::PipelineBindPoint::eGraphics
    );
    subpass.setColorAttachments(color_ref);
    subpass.setPDepthStencilAttachment(&depth_stencil_ref);
    subpass.setResolveAttachments(color_resolve_ref);

    vk::SubpassDependency dependency(
        vk::SubpassExternal,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eLateFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
    );

    std::array<vk::AttachmentDescription, 3> attachments{
        color_attachment, depth_attachment, color_attachment_resolve
    };
    vk::RenderPassCreateInfo render_pass_info(
        vk::RenderPassCreateFlagBits(), attachments, subpass, dependency
    );

    return m_device->createRenderPass(render_pass_info);
}
auto APIVulkan::create_example_pipeline_layout() -> vk::PipelineLayout
{
    vk::DescriptorSetLayoutBinding projection(
        0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex
    );
    vk::DescriptorSetLayoutBinding sampler(
        1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment
    );

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings{projection, sampler};

    vk::DescriptorSetLayoutCreateInfo layout_info(
        vk::DescriptorSetLayoutCreateFlagBits(), bindings
    );
    m_example_descriptor_set_layout = m_device->createDescriptorSetLayout(layout_info);

    vk::PipelineLayoutCreateInfo pipeline_layout(
        vk::PipelineLayoutCreateFlagBits(), m_example_descriptor_set_layout
    );

    return m_device->createPipelineLayout(pipeline_layout);
}

void APIVulkan::create_example_vb()
{
    const auto size = m_model->m_vertices.size() * sizeof(VertexData);

    const auto families = get_queue_families(*m_physical_device);

    VulkanBufferFactory buff_factory(
        {families.graphics.value(), families.transfer.value(), families.compute.value()},
        m_physical_device,
        m_device,
        m_example_command_pool
    );

    const auto buffer = buff_factory.create(
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
    );

    buffer->set_data<VertexData>(m_model->m_vertices);

    m_model_vb = buff_factory.create(
        size,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    if (!buffer->copy_to(*m_model_vb, *m_graphics_queue))
    {
        Log(VulkanAPICategory, LogSeverity::Fatal, "Failed to copy buffer");
    }
}

void APIVulkan::create_example_ib()
{
    const auto size = m_model->m_indices.size() * sizeof(uint32_t);

    const auto families = get_queue_families(*m_physical_device);

    VulkanBufferFactory buff_factory(
        {families.graphics.value(), families.transfer.value(), families.compute.value()},
        m_physical_device,
        m_device,
        m_example_command_pool
    );

    const auto buffer = buff_factory.create(
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
    );

    buffer->set_data<uint32_t>(m_model->m_indices);

    m_model_ib = buff_factory.create(
        size,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    if (!buffer->copy_to(*m_model_ib, *m_graphics_queue))
    {
        Log(VulkanAPICategory, LogSeverity::Fatal, "Failed to copy buffer");
    }
}

void APIVulkan::create_example_uniform_buffers()
{
    const size_t size = sizeof(UniformBufferObject);

    const auto families = get_queue_families(*m_physical_device);
    VulkanBufferFactory buff_factory(
        {families.graphics.value(), families.transfer.value(), families.compute.value()},
        m_physical_device,
        m_device,
        m_example_command_pool
    );

    m_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_uniform_buffers[i] = buff_factory.create(
            size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::SharingMode::eExclusive,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        m_uniform_buffers_mapped[i] =
            m_device->mapMemory(m_uniform_buffers[i]->memory(), 0, size, vk::MemoryMapFlags());
    }
}

void APIVulkan::update_uniform_buffer(uint32_t image_index)
{
    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    float time =
        std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time)
            .count();

    UniformBufferObject ubo{};
    ubo.model =
        glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(m_swapchain_info.extent.width) /
            static_cast<float>(m_swapchain_info.extent.height),
        0.1f,
        10.0f
    );
    ubo.projection[1][1] *= -1;

    memcpy(
        std::any_cast<void*>(m_uniform_buffers_mapped[image_index]),
        &ubo,
        sizeof(UniformBufferObject)
    );
}

void APIVulkan::populate_example_desc_sets()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        std::array<vk::WriteDescriptorSet, 2> writes{};

        vk::DescriptorBufferInfo buffer_info(
            m_uniform_buffers[i]->buffer(), 0, sizeof(UniformBufferObject)
        );
        writes[0] = vk::WriteDescriptorSet(
            m_example_desc_sets[i],
            0,
            0,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            buffer_info,
            nullptr
        );
        vk::DescriptorImageInfo img_info(
            m_example_sampler, m_example_image->view(), vk::ImageLayout::eShaderReadOnlyOptimal
        );
        writes[1] = vk::WriteDescriptorSet(
            m_example_desc_sets[i],
            1,
            0,
            vk::DescriptorType::eCombinedImageSampler,
            img_info,
            nullptr,
            nullptr
        );
        m_device->updateDescriptorSets(writes, nullptr);
    }
}

void APIVulkan::create_example_texture()
{
    int width, height, channels;
    auto pixels =
        stbi_load("assets/textures/viking_room.png", &width, &height, &channels, STBI_rgb_alpha);
    size_t buffer_size = width * height * 4;

    auto pixels_array = std::span(pixels, pixels + buffer_size);

    uint32_t miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    m_example_mips = miplevels;

    const auto families = get_queue_families(*m_physical_device);

    auto factory = VulkanBufferFactory(
        {families.graphics.value(), families.transfer.value(), families.compute.value()},
        m_physical_device,
        m_device,
        m_example_command_pool
    );
    auto buffer = factory.create(
        buffer_size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
    );

    buffer->set_data<stbi_uc>(pixels_array);

    stbi_image_free(pixels);

    const auto img_factory = VulkanImageFactory(
        families, m_physical_device, m_device, m_example_command_pool, m_graphics_queue
    );

    m_example_image = img_factory.create(
        VulkanImageInfo{
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
            .mips = miplevels,
            .num_samples = vk::SampleCountFlagBits::e1,
            .format = vk::Format::eR8G8B8A8Srgb,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
                     vk::ImageUsageFlagBits::eSampled,
            .layout = vk::ImageLayout::eUndefined,
            .aspect = vk::ImageAspectFlagBits::eColor
        },
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    m_example_image->transition_layout(vk::ImageLayout::eTransferDstOptimal);
    m_example_image->copy_from_buffer(*buffer);
}

void APIVulkan::draw_example_frame()
{
    auto result = m_device->waitForFences(
        m_in_flight[m_current_frame], true, std::numeric_limits<uint64_t>().max()
    );

    auto [image_index_result, image_index] = m_device->acquireNextImageKHR(
        m_swapchain_info.swapchain,
        std::numeric_limits<uint64_t>().max(),
        m_swapchain_image_available[m_current_frame],
        nullptr
    );

    if (image_index_result == vk::Result::eErrorOutOfDateKHR)
    {
        std::tie(m_swapchain_info, m_frame_buffers) = recreate_swapchain_and_framebuffers(
            *m_physical_device, m_surface, *m_device, nullptr, m_example_renderpass
        );
        return;
    }
    else if (image_index_result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    m_device->resetFences(m_in_flight[m_current_frame]);

    m_example_command_buffers[m_current_frame].reset();

    record_example_command_buffer(m_example_command_buffers[m_current_frame], image_index);

    const std::array<vk::PipelineStageFlags, 1> wait_stages = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    };

    update_uniform_buffer(m_current_frame);

    m_graphics_queue->submit(
        vk::SubmitInfo(
            m_swapchain_image_available[m_current_frame],
            wait_stages,
            m_example_command_buffers[m_current_frame],
            m_render_finished[m_current_frame]
        ),
        m_in_flight[m_current_frame]
    );

    // TODO: remove exceptions and deal with error out of date assertion.
    try
    {
        auto present_result = m_present_queue->presentKHR(
            vk::PresentInfoKHR(
                m_render_finished[m_current_frame], m_swapchain_info.swapchain, image_index
            )
        );
        if (present_result == vk::Result::eSuboptimalKHR || m_frame_resized)
        {
            m_frame_resized = false;
            std::tie(m_swapchain_info, m_frame_buffers) = recreate_swapchain_and_framebuffers(
                *m_physical_device, m_surface, *m_device, nullptr, m_example_renderpass
            );
        }
    }
    catch (const vk::OutOfDateKHRError)
    {
        m_frame_resized = false;
        std::tie(m_swapchain_info, m_frame_buffers) = recreate_swapchain_and_framebuffers(
            *m_physical_device, m_surface, *m_device, nullptr, m_example_renderpass
        );
    }

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void APIVulkan::create_color_resources(const VkSwapchainInfo& swapchain)
{
    const auto families = get_queue_families(*m_physical_device);
    const auto img_factory = VulkanImageFactory(
        families, m_physical_device, m_device, m_example_command_pool, m_graphics_queue
    );

    m_color_image = img_factory.create(
        VulkanImageInfo{
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .mips = 1,
            .num_samples = m_msaa_samples,
            .format = swapchain.format,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eColorAttachment |
                     vk::ImageUsageFlagBits::eTransientAttachment,
            .layout = vk::ImageLayout::eUndefined,
            .aspect = vk::ImageAspectFlagBits::eColor,
        },
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
}

void APIVulkan::create_depth_resources(const VkSwapchainInfo& swapchain)
{
    const auto img_factory = VulkanImageFactory(
        get_queue_families(*m_physical_device),
        m_physical_device,
        m_device,
        m_example_command_pool,
        m_graphics_queue
    );

    m_depth_image = img_factory.create(
        VulkanImageInfo{
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .mips = 1,
            .num_samples = m_msaa_samples,
            .format = DEPTH_FORMAT,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
            .layout = vk::ImageLayout::eUndefined,
            .aspect = vk::ImageAspectFlagBits::eDepth,
        },
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    m_depth_image->transition_layout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

auto APIVulkan::create_frame_buffers(VkSwapchainInfo swapchain, vk::RenderPass render_pass)
    -> std::vector<vk::Framebuffer>
{
    std::vector<vk::Framebuffer> frame_buffers(swapchain.images.size());

    for (int i = 0; i < frame_buffers.size(); i++)
    {
        const std::array<vk::ImageView, 3> attachments{
            m_color_image->view(), m_depth_image->view(), swapchain.image_views[i]
        };

        auto create_info = vk::FramebufferCreateInfo(
            vk::FramebufferCreateFlags(),
            render_pass,
            attachments,
            swapchain.extent.width,
            swapchain.extent.height,
            1,
            nullptr
        );
        frame_buffers[i] = m_device->createFramebuffer(create_info);
    }

    return frame_buffers;
}

void APIVulkan::record_example_command_buffer(vk::CommandBuffer& buffer, uint32_t image_index)
{
    buffer.begin(vk::CommandBufferBeginInfo());

    buffer.beginRenderPass(
        vk::RenderPassBeginInfo(
            m_example_renderpass,
            m_frame_buffers[image_index],
            vk::Rect2D({0, 0}, m_swapchain_info.extent),
            CLEAR_VALUES
        ),
        vk::SubpassContents::eInline
    );

    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_example_pipeline);
    std::array<size_t, 1> offsets = {0};
    buffer.bindVertexBuffers(0, m_model_vb->buffer(), offsets);
    buffer.bindIndexBuffer(m_model_ib->buffer(), 0, vk::IndexType::eUint32);

    auto viewport = vk::Viewport(
        0.0f, 0.0f, m_swapchain_info.extent.width, m_swapchain_info.extent.height, 0.0f, 1.0f
    );
    buffer.setViewport(0, viewport);

    auto scissor = vk::Rect2D({0, 0}, m_swapchain_info.extent);
    buffer.setScissor(0, scissor);

    buffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_example_layout,
        0,
        m_example_desc_sets[m_current_frame],
        nullptr
    );
    buffer.drawIndexed(m_model->m_indices.size(), 1, 0, 0, 0);

    buffer.endRenderPass();
    buffer.end();
}

void APIVulkan::create_sync_objects()
{
    m_swapchain_image_available.resize(MAX_FRAMES_IN_FLIGHT);
    m_render_finished.resize(MAX_FRAMES_IN_FLIGHT);
    m_in_flight.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_swapchain_image_available[i] = m_device->createSemaphore(vk::SemaphoreCreateInfo());
        m_render_finished[i] = m_device->createSemaphore(vk::SemaphoreCreateInfo());
        m_in_flight[i] =
            m_device->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }
}

auto APIVulkan::create_descriptor_pool(vk::DescriptorType type, uint32_t size) -> vk::DescriptorPool
{
    auto pool_size = vk::DescriptorPoolSize(type, size);
    return m_device->createDescriptorPool(
        vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlags(), size, pool_size)
    );
}

auto APIVulkan::create_descriptor_sets(uint32_t count) -> std::vector<vk::DescriptorSet>
{
    std::vector<vk::DescriptorSetLayout> layouts(count, m_example_descriptor_set_layout);
    return m_device->allocateDescriptorSets(
        vk::DescriptorSetAllocateInfo(m_example_desc_pool, layouts)
    );
}

void APIVulkan::cleanup_swapchain(VkSwapchainInfo& swapchain)
{
    m_color_image.reset();
    m_depth_image.reset();

    for (const auto& framebuffer : m_frame_buffers)
    {
        m_device->destroyFramebuffer(framebuffer);
    }
    for (const auto& image_view : m_swapchain_info.image_views)
    {
        m_device->destroyImageView(image_view);
    }
    m_device->destroySwapchainKHR(m_swapchain_info.swapchain);
}

auto APIVulkan::recreate_swapchain_and_framebuffers(
    vk::PhysicalDevice physical_device,
    vk::SurfaceKHR surface,
    vk::Device device,
    vk::SwapchainKHR old_swapchain,
    vk::RenderPass render_pass
) -> std::tuple<VkSwapchainInfo, std::vector<vk::Framebuffer>>
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window_ref.get_raw_window(), &width, &height);
    while (width == 0 && height == 0)
    {
        glfwGetFramebufferSize(m_window_ref.get_raw_window(), &width, &height);
        glfwWaitEvents();
    }
    m_device->waitIdle();
    cleanup_swapchain(m_swapchain_info);
    auto swapchain_info = create_swapchain(physical_device, surface, device, nullptr);
    create_color_resources(swapchain_info);
    create_depth_resources(swapchain_info);
    auto frame_buffers = create_frame_buffers(swapchain_info, render_pass);
    return {swapchain_info, frame_buffers};
}
}  // namespace BE_NAMESPACE