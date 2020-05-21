#include <catch2/catch.hpp>
#include <iostream>
#include <lmengine/name.h>
#include <lmengine/pose.h>
#include <lmengine/reflection.h>
#include <lmengine/serialisation.h>
#include <lmengine/transform.h>
#include <lmlib/range.h>
#include <range/v3/algorithm/generate.hpp>
#include <range/v3/view/zip.hpp>

std::pair<entt::entity, std::array<entt::entity, 4>>
  create_test_hierarchy(entt::registry &registry)
{
    auto parent = registry.create();

    std::array<entt::entity, 4> children{entt::null};

    registry.create(children.begin(), children.end());

    std::array<lmng::name, 4> names{"1", "11", "111", "2"};

    registry.insert<lmng::name>(
      children.begin(), children.end(), names.begin(), names.end());

    std::array<lmng::transform_parent, 4> parents{
      parent, children[0], children[1], parent};

    registry.insert<lmng::transform_parent>(
      children.begin(), children.end(), parents.begin(), parents.end());

    return {parent, children};
};

TEST_CASE("Save/load pose")
{
    entt::registry registry;

    auto [parent, children] = create_test_hierarchy(registry);

    std::array transforms{
      lmng::transform{{1.f, 0.f, 0.f}},
      lmng::transform{{1.f, 0.f, 0.f}},
      lmng::transform{{1.f, 0.f, 0.f}},
      lmng::transform{{2.f, 0.f, 0.f}},
    };

    registry.insert<lmng::transform>(
      children.begin(), children.end(), transforms.begin(), transforms.end());

    auto yaml = lmng::save_pose(registry, parent);

    registry = entt::registry{};

    std::tie(parent, children) = create_test_hierarchy(registry);

    registry.insert<lmng::transform>(children.begin(), children.end(), {});

    lmng::load_pose(registry, parent, yaml);

    registry.emplace<lmng::transform>(parent);

    for (auto i : lm::range(4))
    {
        CHECK(registry.get<lmng::transform>(children[i]) == transforms[i]);
    }

    auto resolved_child = lmng::resolve_transform(registry, children[2]);

    auto composed_transform = lmng::compose_transforms(
      lmng::compose_transforms(transforms[0], transforms[1]), transforms[2]);

    CAPTURE(
      resolved_child.position,
      composed_transform.position,
      resolved_child.rotation,
      composed_transform.rotation);

    CHECK(resolved_child == composed_transform);
}
