#include "cppxml.hpp"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <vector>
#include <iterator>
#include <algorithm>

namespace {

void to_xml(lua_State* L, int index, rapidxml::xml_document<>& doc,
            rapidxml::xml_node<>* root = nullptr) {
  if (index <= 0 || !lua_istable(L, index)) {
    return;
  }
  lua_pushstring(L, "type");
  lua_gettable(L, index);
  auto type = lua_tointeger(L, -1);
  lua_pop(L, 1);
  auto visit_children = [&](rapidxml::xml_node<>* node) {
    lua_pushstring(L, "children");
    lua_gettable(L, index);
    if (lua_istable(L, -1)) {
      lua_pushnil(L);
      while (lua_next(L, -2)) {
        to_xml(L, lua_gettop(L), doc, node);
        lua_pop(L, 1);
      }
    }
    lua_pop(L, 1);
  };
  auto visit_attributes = [&](rapidxml::xml_node<>* node) {
    lua_pushstring(L, "attributes");
    lua_gettable(L, index);
    if (lua_istable(L, -1)) {
      lua_pushnil(L);
      while (lua_next(L, -2)) {
        if (lua_istable(L, -1)) {
          lua_pushstring(L, "name");
          lua_gettable(L, -2);
          size_t name_len = 0;
          auto name_src = lua_tolstring(L, -1, &name_len);
          auto name = doc.allocate_string(name_src, name_len);
          lua_pop(L, 1);
          lua_pushstring(L, "value");
          lua_gettable(L, -2);
          size_t value_len = 0;
          auto value_src = lua_tolstring(L, -1, &value_len);
          auto value = doc.allocate_string(value_src, value_len);
          lua_pop(L, 1);
          auto attr = doc.allocate_attribute(name, value, name_len, value_len);
          node->append_attribute(attr);
        }
        lua_pop(L, 1);
      }
    }
    lua_pop(L, 1);
  };
  switch (type) {
  case rapidxml::node_document:
    visit_children(&doc);
    break;
  case rapidxml::node_element:
    if (root) {
      lua_pushstring(L, "name");
      lua_gettable(L, index);
      size_t name_len = 0;
      auto name_src = lua_tolstring(L, -1, &name_len);
      auto name = doc.allocate_string(name_src, name_len);
      lua_pop(L, 1);
      auto node = doc.allocate_node(static_cast<rapidxml::node_type>(type),
                                    name, nullptr, name_len, 0);
      visit_attributes(node);
      visit_children(node);
      root->append_node(node);
    }
    break;
  case rapidxml::node_data:
  case rapidxml::node_cdata:
  case rapidxml::node_comment:
  case rapidxml::node_doctype:
    if (root) {
      lua_pushstring(L, "value");
      lua_gettable(L, index);
      size_t value_len = 0;
      auto value_src = lua_tolstring(L, -1, &value_len);
      auto value = doc.allocate_string(value_src, value_len);
      lua_pop(L, 1);
      auto node = doc.allocate_node(static_cast<rapidxml::node_type>(type),
                                    nullptr, value, 0, value_len);
      root->append_node(node);
    }
    break;
  case rapidxml::node_declaration:
    if (root) {
      auto node = doc.allocate_node(static_cast<rapidxml::node_type>(type));
      visit_attributes(node);
      root->append_node(node);
    }
    break;
  case rapidxml::node_pi:
    if (root) {
      lua_pushstring(L, "name");
      lua_gettable(L, index);
      size_t name_len = 0;
      auto name_src = lua_tolstring(L, -1, &name_len);
      auto name = doc.allocate_string(name_src, name_len);
      lua_pop(L, 1);
      lua_pushstring(L, "value");
      lua_gettable(L, index);
      size_t value_len = 0;
      auto value_src = lua_tolstring(L, -1, &value_len);
      auto value = doc.allocate_string(value_src, value_len);
      lua_pop(L, 1);
      auto node = doc.allocate_node(static_cast<rapidxml::node_type>(type),
                                    name, value, name_len, value_len);
      root->append_node(node);
    }
    break;
  default:
    break;
  }
}

void to_lua(lua_State* L, rapidxml::xml_node<>* node) {
  lua_newtable(L); {
    lua_pushstring(L, "type");
    lua_pushinteger(L, node->type());
    lua_settable(L, -3);
    switch (node->type()) {
    case rapidxml::node_element:
    case rapidxml::node_pi:
      lua_pushstring(L, "name");
      lua_pushlstring(L, node->name(), node->name_size());
      lua_settable(L, -3);
      break;
    default:
      break;
    }
    switch (node->type()) {
    case rapidxml::node_data:
    case rapidxml::node_cdata:
    case rapidxml::node_comment:
    case rapidxml::node_doctype:
    case rapidxml::node_pi:
      lua_pushstring(L, "value");
      lua_pushlstring(L, node->value(), node->value_size());
      lua_settable(L, -3);
      break;
    default:
      break;
    }
    switch (node->type()) {
    case rapidxml::node_element:
    case rapidxml::node_declaration:
      lua_pushstring(L, "attributes");
      lua_newtable(L); {
        lua_Integer idx = 0;
        for (auto i = node->first_attribute(); i; i = i->next_attribute()) {
          lua_pushinteger(L, ++idx);
          lua_createtable(L, 0, 2); {
            lua_pushstring(L, "name");
            lua_pushlstring(L, i->name(), i->name_size());
            lua_settable(L, -3);
            lua_pushstring(L, "value");
            lua_pushlstring(L, i->value(), i->value_size());
            lua_settable(L, -3);
          }
          lua_settable(L, -3);
        }
      }
      lua_settable(L, -3);
      break;
    default:
      break;
    }
    switch (node->type()) {
    case rapidxml::node_document:
    case rapidxml::node_element:
      lua_pushstring(L, "children");
      lua_newtable(L); {
        lua_Integer idx = 0;
        for (auto i = node->first_node(); i; i = i->next_sibling()) {
          lua_pushinteger(L, ++idx);
          to_lua(L, i);
          lua_settable(L, -3);
        }
      }
      lua_settable(L, -3);
      break;
    default:
      break;
    }
  }
}

int cppxml_encode(lua_State* L) {
  rapidxml::xml_document<> doc;
  to_xml(L, 1, doc);
  std::vector<char> buf;
  rapidxml::print(std::back_inserter(buf), doc);
  lua_pushlstring(L, buf.data(), buf.size());
  return 1;
}

int cppxml_decode(lua_State* L) {
  size_t len = 0;
  auto str = lua_tolstring(L, 1, &len);
  try {
    std::vector<char> buf;
    buf.reserve(len + 1);
    std::copy(str, str + len, std::back_inserter(buf));
    buf.push_back(0);
    rapidxml::xml_document<> doc;
    doc.parse<0>(buf.data());
    to_lua(L, &doc);
    lua_pushnil(L);
  } catch (const rapidxml::parse_error& e) {
    lua_pushnil(L);
    lua_createtable(L, 0, 2); {
      lua_pushstring(L, "what");
      lua_pushstring(L, e.what());
      lua_settable(L, -3);
      lua_pushstring(L, "where");
      lua_pushstring(L, e.where<char>());
      lua_settable(L, -3);
    }
  }
  return 2;
}

}

int luaopen_resty_cppxml(lua_State* L) {
  constexpr const luaL_Reg reg[] = {
    {"encode", cppxml_encode},
    {"decode", cppxml_decode},
    {nullptr, nullptr}
  };
  luaL_register(L, "cppxml", reg);
  return 1;
}
