#include "map_editor_controller.h"
#include <lmeditor/component/map_editor.h>
#include <lmeditor/model/selection.h>
#include <lmlib/eigen.h>
#include <lmlib/variant_visitor.h>
#include <lmng/name.h>
#include <lmng/physics.h>
#include <lmng/reflection.h>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/join.hpp>

namespace lmeditor
{
map_editor_controller::map_editor_controller(
  entt::registry &map,
  orbital_camera_init const &camera_init)
    : state{select_state{*this}},
      state_text{select_state::label},
      viewport{{camera_init}},
      map{&map}
{
}

bool map_editor_controller::handle(lmtk::input_event const &event)
{
    return std::visit(
      lm::variant_visitor{
        [&](
          lmtk::key_down_event const &key_down_event, auto &state_alternative) {
            auto itfound = state_alternative.key_command_map.find(
              key_down_event.input_state.key_state);

            if (itfound == state_alternative.key_command_map.end())
            {
                itfound =
                  state_alternative.key_command_map.find(key_down_event.key);

                if (itfound == state_alternative.key_command_map.end())
                    return false;
            }

            return itfound->second.fn(command_args{
              *this,
              *map,
              key_down_event,
            });
        },
        [](auto &, auto &) { return false; },
      },
      event,
      state);
}

entt::entity map_editor_controller::add_cube(
  entt::registry &map,
  Eigen::Vector3f const &position,
  float extent)
{
    auto new_cube = map.create();

    auto &transform = map.assign<lmng::transform>(
      new_cube, position, Eigen::Quaternionf::Identity());

    auto &box = map.assign<lmng::box_render>(
      new_cube,
      Eigen::Vector3f{extent, extent, extent},
      std::array{0.5f, 0.5f, 0.5f});

    map.assign<lmng::box_collider>(
      new_cube, Eigen::Vector3f{extent, extent, extent});

    auto &rigid_body =
      map.assign<lmng::rigid_body>(new_cube, 1.f, 0.25f, 0.75f);

    map.assign<lmng::name>(new_cube, get_unique_name(map, new_cube, "Box"));

    return new_cube;
}

std::string map_editor_controller::get_unique_name(
  entt::registry const &map,
  entt::entity for_entity,
  const std::string &name)
{
    unsigned postfix{0};

    std::string found_name{name};

    if (auto maybe_parent = map.try_get<lmng::transform_parent>(for_entity))
    {
        while (true)
        {
            bool collides{false};

            lmng::visit_transform_children(
              map, maybe_parent->entity, [&](entt::entity child) {
                  if (lmng::get_name(map, child) == name)
                      collides = true;
              });

            if (!collides)
                break;

            found_name = name + std::to_string(++postfix);
        }

        return name;
    }

    auto name_view =
      map.view<lmng::name const>(entt::exclude<lmng::transform_parent>);

    while (ranges::find_if(name_view, [&](auto entity) {
               return name_view.get(entity).string == found_name;
           }) != name_view.end())
    {
        found_name += std::to_string(++postfix);
    }

    return found_name;
}

entt::entity map_editor_controller::add_adjacent(
  entt::registry &map,
  Eigen::Vector3f const &direction)
{
    auto add_pos = get_selection_position(map);
    auto extents = get_selection_extents(map);
    extents += Eigen::Vector3f::Ones();
    add_pos += std::abs(extents.dot(direction)) * direction;
    return add_cube(map, add_pos, 1.f);
}

entt::entity map_editor_controller::copy_entity(
  entt::registry &map,
  Eigen::Vector3f const &direction)
{
    auto selected_box = get_selection();

    auto new_box = map.create(selected_box);

    auto new_name =
      get_unique_name(map, new_box, lmng::get_name(map, selected_box).c_str());
    lmng::reflect_components(
      map, selected_box, [&](lmng::any_component component) {
          component.assign(map, new_box);
      });
    auto &transform = map.get<lmng::transform>(new_box);
    transform.position +=
      2 *
      Eigen::Vector3f{direction.array() * get_selection_extents(map).array()};
    map.assign<lmng::name>(new_box, lmng::name{new_name});
    return new_box;
}

void map_editor_controller::translate(
  entt::registry &map,
  entt::entity entity,
  Eigen::Vector3f const &vector)
{
    auto new_transform = map.get<lmng::transform>(entity);
    new_transform.position += vector;
    map.replace<lmng::transform>(entity, new_transform);
}

std::vector<command_description>
  map_editor_controller::get_command_descriptions()
{
    return state >> lm::variant_visitor{[this](auto &state_alternative) {
               return get_command_descriptions(
                 state_alternative.commands,
                 std::string{state_alternative.label}.empty()
                   ? "Map Editor"
                   : std::string{"Map Editor -> "} + state_alternative.label);
           }};
}

std::vector<command_description>
  map_editor_controller::get_command_descriptions(
    map_editor_controller::command_list const &list,
    std::string const &context)
{
    std::vector<command_description> command_lines;
    for (auto &[keys, command] : list)
    {
        std::vector<std::string> key_names;
        for (auto key : keys)
            key_names.emplace_back(lmpl::get_keycode_string(key));

        std::string keystroke_string;
        command_lines.emplace_back(command_description{
          command.description,
          ranges::views::join(key_names, ' ') | ranges::to<std::string>(),
          context,
        });
    }
    return std::move(command_lines);
}

Eigen::Vector3f
  map_editor_controller::view_to_axis(Eigen::Vector3f const &view_vector)
{
    return lm::snap_to_axis(view_to_world(view_vector));
}

Eigen::Vector3f
  map_editor_controller::get_selection_extents(entt::registry const &map) const
{
    auto selected_box = get_selection();

    auto maybe_box_render = map.try_get<lmng::box_render>(selected_box);
    if (maybe_box_render)
        return maybe_box_render->extents;

    auto maybe_box_collider = map.try_get<lmng::box_collider>(selected_box);
    if (maybe_box_collider)
        return maybe_box_collider->extents;

    return Eigen::Vector3f{0.f, 0.f, 0.f};
}

void map_editor_controller::select(entt::entity entity)
{
    clear_selection();
    map->assign<selected>(entity);
}

void map_editor_controller::clear_selection() const { map->clear<selected>(); }

entt::registry *map_editor_controller::get_registry() const { return map; }

bool map_editor_controller::have_selection() const
{
    return !map->view<selected>().empty();
}

entt::entity map_editor_controller::get_selection() const
{
    return lmeditor::get_selection(*map);
}

lm::camera map_editor_controller::get_camera() const { return camera; }

Eigen::Vector3f
  map_editor_controller::view_to_world(const Eigen::Vector3f &view_vector)
{
    return camera.rotation * view_vector;
}

Eigen::Vector3f
  map_editor_controller::get_selection_position(const entt::registry &map) const
{
    return lmng::resolve_transform(map, get_selection()).position;
}

} // namespace lmeditor
