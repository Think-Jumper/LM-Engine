#include <entt/entt.hpp>
#include <lmengine/reflection.h>

namespace lmng
{
entt::registry clone(const entt::registry &other)
{
    entt::registry new_registry;

    entt::resolve([&](entt::meta_type const &meta_type) {
        if (is_component_type(meta_type))
        {
            meta_type.func("clone"_hs).invoke({}, &other, &new_registry);
        }
    });

    return std::move(new_registry);
}
} // namespace lmng