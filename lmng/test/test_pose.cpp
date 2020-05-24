#include "hierarchy.h"
#include <catch2/catch.hpp>
#include <iostream>
#include <lmlib/range.h>
#include <lmng/name.h>
#include <lmng/pose.h>
#include <lmng/reflection.h>
#include <lmng/serialisation.h>
#include <lmng/transform.h>
#include <range/v3/algorithm/generate.hpp>
#include <range/v3/view/zip.hpp>

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

    registry.assign<lmng::transform>(
      children.begin(), children.end(), transforms.begin());

    auto yaml = lmng::save_pose(registry, parent);

    registry = entt::registry{};

    std::tie(parent, children) = create_test_hierarchy(registry);

    registry.assign<lmng::transform>(children.begin(), children.end(), {});

    lmng::load_pose(registry, parent, yaml);

    registry.assign<lmng::transform>(parent);

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
