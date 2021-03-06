#include "asset_cache.h"
#include <Eigen/Eigen>
#include <catch2/catch.hpp>
#include <entt/entity/registry.hpp>
#include <lmlib/math_constants.h>
#include <lmng/archetype.h>
#include <lmng/hierarchy.h>
#include <lmng/logging.h>
#include <lmng/name.h>
#include <lmng/transform.h>
#include <lmng/yaml_save_load.h>

lmng::archetype_data create_test_archetype()
{
    lmng::archetype_data archetype_data;

    YAML::Node components_yaml;

    YAML::Node transform_yaml;

    transform_yaml["Position"] = "1.0 2.0 3.0";
    transform_yaml["Rotation"] = "0.0 0.0 0.0 1.0";

    components_yaml["Transform"] = transform_yaml;

    YAML::Node children_yaml;

    YAML::Node child_yaml;

    YAML::Node child_components_yaml;
    YAML::Node child_children_yaml;

    child_yaml["components"] = child_components_yaml;
    child_yaml["children"] = child_children_yaml;

    children_yaml["Child"] = child_yaml;

    archetype_data.yaml["components"] = components_yaml;
    archetype_data.yaml["children"] = children_yaml;
    return archetype_data;
}

TEST_CASE("Map serialisation with archetypes")
{
    test_asset_cache asset_cache;

    asset_cache.emplace<lmng::archetype_data>(
      "archetype", create_test_archetype());

    entt::registry registry;
    lmng::hierarchy_system hierarchy_system{registry};

    auto parent = registry.create();
    auto child = registry.create();

    registry.emplace<lmng::parent>(child, parent);
    registry.emplace<lmng::name>(parent, "Parent");
    registry.emplace<lmng::transform>(
      parent, Eigen::Vector3f{1.f, 2.f, 3.f}, Eigen::Quaternionf::Identity());
    registry.emplace<lmng::name>(child, "Child");
    registry.emplace<lmng::archetype>(parent, "archetype");

    YAML::Node expected_yaml;

    YAML::Node expected_parent_yaml;

    expected_parent_yaml["archetype"] = "archetype";
    expected_parent_yaml["components"] = YAML::Node{};
    expected_parent_yaml["children"] = YAML::Node{};

    expected_yaml["Parent"] = expected_parent_yaml;

    YAML::Node actual_yaml;
    lmng::save_registry_as_yaml(registry, asset_cache, actual_yaml);

    std::ostringstream expected_str{}, actual_str{};
    expected_str << expected_yaml;
    actual_str << actual_yaml["entities"];

    REQUIRE(actual_str.str() == expected_str.str());
}

TEST_CASE("Map deserialisation with archetypes")
{
    test_asset_cache asset_cache;

    asset_cache.emplace<lmng::archetype_data>(
      "archetype", create_test_archetype());

    YAML::Node map_yaml;

    YAML::Node parent_yaml;
    parent_yaml["archetype"] = "archetype";

    YAML::Node parent_transform_yaml;

    parent_transform_yaml["Position"] = "4.0 5.0 6.0";

    YAML::Node parent_components_yaml;
    parent_components_yaml["Transform"] = parent_transform_yaml;

    parent_yaml["components"] = parent_components_yaml;
    parent_yaml["children"] = YAML::Node{};

    map_yaml["entities"]["Parent"] = parent_yaml;
    map_yaml["schema_version"] = "1";

    entt::registry registry;
    lmng::hierarchy_system hierarchy_system{registry};
    lmng::connect_component_logging(registry);

    lmng::populate_registry_from_yaml(map_yaml, registry, asset_cache);

    auto parent = lmng::find_entity(registry, "Parent");

    REQUIRE((parent != entt::null) == true);

    auto child = lmng::find_entity(registry, "Child");

    REQUIRE((child != entt::null) == true);

    auto parent_transform = registry.get<lmng::transform>(parent);

    REQUIRE(parent_transform.position == Eigen::Vector3f{4.f, 5.f, 6.f});
}
