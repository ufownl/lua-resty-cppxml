#include "cppxml.hpp"
#include <rapidxml.hpp>
#include <vector>

namespace {

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
    if (node->type() == rapidxml::node_element) {
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

int cppxml_decode(lua_State* L) {
  size_t len = 0;
  auto str = lua_tolstring(L, 1, &len);
  try {
    std::vector<char> buf{str, str + len};
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
    {"decode", cppxml_decode},
    {nullptr, nullptr}
  };
  luaL_register(L, "cppxml", reg);
  return 1;
}
