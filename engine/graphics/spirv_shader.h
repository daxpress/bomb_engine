#pragma once

namespace BE_NAMESPACE
{
struct UniformBufferData
{
    std::string name;
    uint32_t binding;
};

enum class E_SHADER_STAGE : uint8_t
{
    VERTEX = 0,
    FRAGMENT,
    COMPUTE,
    TASK,
    MESH
};

class SPIRVShader
{
public:
    SPIRVShader() = default;
    explicit SPIRVShader(const std::vector<char>& spirv_binary);

    [[nodiscard]] inline auto get_data() const -> const std::vector<uint32_t>& { return m_data; }
    [[nodiscard]] inline auto get_bytes_count() const -> const size_t
    {
        return m_data.size() * sizeof(uint32_t);
    }
    [[nodiscard]] inline auto get_stage() const -> const E_SHADER_STAGE { return m_stage; }

private:
    std::vector<uint32_t> m_data;
    E_SHADER_STAGE m_stage;
    std::vector<UniformBufferData> m_uniform_buffers;
};
}  // namespace BE_NAMESPACE