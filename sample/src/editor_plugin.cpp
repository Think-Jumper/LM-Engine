#include <vector>

#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION
#include <boost/dll.hpp>
#include <entt/entt.hpp>
#include <range/v3/view.hpp>

#include <lmeditor/simulation_plugin.h>

#include "reflect_types.h"
#include "simulations/character_movement.h"
#include "simulations/rigid_bodies_battle.h"

LMEDITOR_SIMULATION_PLUGIN(rigid_bodies_battle);
LMEDITOR_SIMULATION_PLUGIN(character_movement);

using simulation_creation_fn =
  std::function<lmeditor::psimulation(lmng::simulation_init const &)>;

std::map<std::string, simulation_creation_fn> simulation_creation_map{
  std::pair{
    "Rigid Bodies Battle",
    [](auto &init) {
        return lm::reference{
          std::make_unique<rigid_bodies_battle_plugin>(init)};
    },
  },
  std::pair{
    "Character Movement",
    [](auto &init) {
        return lm::reference{std::make_unique<character_movement_plugin>(init)};
    },
  },
};

std::vector<std::string> list_simulations_cpp()
{
    return ranges::views::keys(simulation_creation_map) |
           ranges::to<std::vector>();
}

BOOST_DLL_ALIAS(list_simulations_cpp, list_simulations);

lmeditor::psimulation create_simulation_cpp(
  std::string const &name,
  lmng::simulation_init const &init)
{
    return simulation_creation_map[name](init);
}

BOOST_DLL_ALIAS(create_simulation_cpp, create_simulation);

void set_meta_context_cpp(entt::meta_ctx const &ctx)
{
    entt::meta_ctx::bind(ctx);
}

BOOST_DLL_ALIAS(set_meta_context_cpp, set_meta_context);

BOOST_DLL_ALIAS(reflect_types_cpp, reflect_types);
