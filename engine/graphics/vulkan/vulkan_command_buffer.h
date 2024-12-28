#pragma once

namespace BE_NAMESPACE
{

class VulkanCommandBuffer
{
    template <uint32_t Count>
    using ReturnType = std::optional<
        std::conditional_t<Count == 1, VulkanCommandBuffer, std::vector<VulkanCommandBuffer>>>;

public:
    // shorthand to allocate multiple command buffers on the fly
    template <uint32_t Count>
    static auto create(
        const std::shared_ptr<vk::Device> device,
        const vk::CommandPool& pool,
        const vk::CommandBufferLevel level
    ) -> ReturnType<Count>
    {
        auto expected_buffers =
            device->allocateCommandBuffers(vk::CommandBufferAllocateInfo(pool, level, Count));

        if (expected_buffers.empty())
        {
            return std::nullopt;
        }

        if constexpr (Count == 1)
        {
            return ReturnType<Count>(VulkanCommandBuffer(device, pool, expected_buffers.at(0)));
        }
        else
        {
            std::vector<VulkanCommandBuffer> command_buffers{};
            command_buffers.reserve(expected_buffers.size());

            for (auto& buffer : expected_buffers)
            {
                command_buffers.emplace_back(device, pool, buffer);
            }

            return ReturnType<Count>(std::move(command_buffers));
        }
    }

    explicit VulkanCommandBuffer(
        std::shared_ptr<vk::Device> device, const vk::CommandPool& pool, vk::CommandBuffer buffer
    );

    void begin_record(
        const vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    ) const;
    void submit(const vk::Queue& queue) const;
    void end_record() const { m_command_buffer.end(); };

    [[nodiscard]] auto get() const -> const vk::CommandBuffer& { return m_command_buffer; }

    auto operator->() -> vk::CommandBuffer* { return &m_command_buffer; }

    //~VulkanCommandBuffer();

    void free_buffer() const { m_device->freeCommandBuffers(m_command_pool, m_command_buffer); }

private:
    vk::CommandBuffer m_command_buffer;
    const vk::CommandPool& m_command_pool;
    const std::shared_ptr<vk::Device> m_device;
};
}  // namespace BE_NAMESPACE