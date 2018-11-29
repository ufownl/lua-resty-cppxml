# lua-resty-cppxml

Convert xml string to lua table or reverse convert lua table to xml string.

## Install

```
cmake . && make && make install
```

## Usage

XML -> lua table:

```lua
local xml = require("resty.cppxml")
local str = [[
  <test one="two">
    <three four="five" four="six"/>
    <three>eight</three>
    <nine ten="eleven">twelve</nine>
  </test>
]]
local tbl, err = xml.decode(str)
```

If `tbl` is nil then `err.what` represent the error description and `err.where` represent where the error occurred. Otherwise, the value of `tbl` is:

```lua
{
  type = xml.node_type.document,
  children = {
    {
      type = xml.node_type.element,
      name = "test",
      attributes = {
        {
          name = "one",
          value = "two"
        }
      },
      children = {
        {
          type = xml.node_type.element,
          name = "three",
          attributes = {
            {
              name = "four",
              value = "five"
            },
            {
              name = "four",
              value = "six"
            }
          },
          children = {}
        },
        {
          type = xml.node_type.element,
          name = "three",
          attributes = {},
          children = {
            {
              type = xml.node_type.data,
              value = "eight"
            }
          }
        },
        {
          type = xml.node_type.element,
          name = "nine",
          attributes = {
            {
              name = "ten",
              value = "eleven"
            }
          },
          children = {
            {
              type = xml.node_type.data,
              value = "twelve"
            }
          }
        }
      }
    }
  }
}
```

lua table -> XML:

```lua
local xml = require("resty.cppxml")
local tbl = {
  -- Assume the value of `tbl` is equal the value above.
}
local str = xml.encode(tbl)
local str_without_indent = xml.encode(tbl, 1)
```
