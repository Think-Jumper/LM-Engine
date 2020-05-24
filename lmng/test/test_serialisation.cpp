#include "hierarchy.h"
#include <catch2/catch.hpp>
#include <fstream>
#include <lmng/name.h>
#include <lmng/serialisation.h>
#include <lmng/transform.h>
#include <yaml-cpp/yaml.h>

TEST_CASE("Serialise hierarchies")
{
    entt::registry registry;

    create_test_hierarchy(registry);

    for (auto i = 0; i < 10; i++)
    {
        YAML::Node output_yaml;

        lmng::serialise(registry, output_yaml);

        registry.clear();

        lmng::deserialise(output_yaml, registry);
    }
}
