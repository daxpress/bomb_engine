#pragma warning(disable : 5030)
#define expose clang::annotate("expose")
#define hide clang::annotate("hide")

#include <optional>
#include <string>
#include "included.h"

namespace exposition
{
    /// @brief this is a brief description.
    class [[expose]] ThisIsClass
    {
    public:
        int pub_int = 0;

        /// @brief method brief.
        void pub_met() {};

        [[hide]]
        void hidden() {};

    private:
        /// @brief member brief.
        const int priv_int = -1;
        static bool is_static;
        std::string string;

        int priv_met(char argument) { return 1; };
        static float static_method() { return .0f; };
    };

    struct [[expose]] ThisIsStruct
    {
        ThisIsClass *ptr;
        ThisIsClass &reference;
        char achar;

        ThisIsClass* returns_ptr() const { return ptr; };
    };
    /// @brief enum brief.
    enum class [[expose]] E_123
    {
        E1,
        E2,
        E3
    };

    [[expose]]
    std::optional<int> thisisfunc()
    {
        return std::nullopt;
    };

    // simple comment
    class [[expose]] OOPs 
    {
        public:
        OOPs();
        OOPs(OOPs& other);
        OOPs(OOPs&& other);
        OOPs& operator=(OOPs& other); 
        OOPs& operator=(OOPs&& other);

        ~OOPs();
    };
}

class [[expose]] AbstractClass 
{
    public: 

    virtual void i_am_abstract() = 0;
    virtual void virtual_with_default_impl(){};
};

/// @brief function brief.
[[expose]]
int outofnamespace(int c)
{
    return 1;
}

/// @brief variable brief.
[[expose]]
std::string iamvar = "";

[[expose]]
static int static_variable;