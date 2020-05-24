#include "hierarchy.h"
#include <lmng/name.h>
#include <lmng/transform.h>
#include <range/v3/view/all.hpp>

std::pair<entt::entity, std::array<entt::entity, 4>>
  create_test_hierarchy(entt::registry &registry)
{
    auto parent = registry.create();

    registry.assign<lmng::name>(parent, "Parent");

    auto child = registry.create();
    registry.assign<lmng::transform_parent>(child, parent);
    registry.assign<lmng::name>(child, "Child");

    auto other_child = registry.create();
    registry.assign<lmng::transform_parent>(other_child, parent);
    registry.assign<lmng::name>(other_child, "Other Child");

    auto grandchild = registry.create();
    registry.assign<lmng::transform_parent>(grandchild, child);
    registry.assign<lmng::name>(grandchild, "Grandchild");

    auto great_grandchild = registry.create();
    registry.assign<lmng::transform_parent>(great_grandchild, grandchild);
    registry.assign<lmng::name>(great_grandchild, "Great Grandchild");

    return {
      parent, std::array{child, other_child, grandchild, great_grandchild}};
};
