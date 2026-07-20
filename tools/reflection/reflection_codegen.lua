local M = {}

local function push(t, value)
    t[#t + 1] = value
end

local function trim(s)
    return (s or ""):gsub("^%s+", ""):gsub("%s+$", "")
end

local function strip_quotes(s)
    s = trim(s)
    if #s >= 2 and s:sub(1, 1) == '"' and s:sub(-1) == '"' then
        return s:sub(2, -2)
    end
    return s
end

local function split_csv(s)
    local out = {}
    local current = {}
    local in_quote = false
    for i = 1, #s do
        local ch = s:sub(i, i)
        if ch == '"' then
            in_quote = not in_quote
            push(current, ch)
        elseif ch == "," and not in_quote then
            push(out, trim(table.concat(current)))
            current = {}
        else
            push(current, ch)
        end
    end
    local tail = trim(table.concat(current))
    if tail ~= "" then
        push(out, tail)
    end
    return out
end

local function parse_attrs(s)
    local attrs = {}
    for _, token in ipairs(split_csv(s or "")) do
        local key, value = token:match("^([%w_]+)%s*=%s*(.+)$")
        if key then
            attrs[key] = strip_quotes(value)
        elseif token ~= "" then
            attrs[token] = true
        end
    end
    return attrs
end

local function escape_cpp_string(s)
    return (s or ""):gsub("\\", "\\\\"):gsub('"', '\\"')
end

local function read_lines(pathname)
    local f = assert(io.open(pathname, "r"))
    local lines = {}
    for line in f:lines() do
        push(lines, line)
    end
    f:close()
    return lines
end

local function parse_field_decl(line)
    local decl = trim(line:gsub("//.*$", ""))
    decl = decl:gsub("%b{}", "")
    decl = decl:gsub("=.*$", ";")
    decl = decl:gsub(";%s*$", "")
    decl = trim(decl)
    if decl == "" or decl:find("%(") then
        return nil
    end
    local cpp_type, name = decl:match("^(.-)%s+([%w_]+)%s*$")
    if not cpp_type or not name then
        return nil
    end
    return trim(cpp_type), name
end

local function parse_function_decl(lines, index)
    local sig = trim(lines[index]:gsub("//.*$", ""))
    local i = index
    while not sig:find("[;{]") and i < #lines do
        i = i + 1
        sig = sig .. " " .. trim(lines[i]:gsub("//.*$", ""))
    end
    local clean = trim(sig:gsub("%s*{.*$", ""):gsub(";%s*$", ""))
    local prefix, args, suffix = clean:match("^(.-)%((.*)%)%s*(.*)$")
    if not prefix then
        return nil, index
    end
    local ret, name = trim(prefix):match("^(.-)%s+([~%w_]+)$")
    if not ret or not name then
        return nil, index
    end
    return {
        name = name,
        return_type = trim(ret),
        args = trim(args),
        suffix = trim(suffix),
        signature = clean,
    }, i
end

local function find_next_decl_line(lines, index)
    local i = index + 1
    while i <= #lines do
        local line = trim(lines[i])
        if line ~= "" and not line:match("^//") and line ~= "public:" and line ~= "private:" and line ~= "protected:" then
            return i
        end
        i = i + 1
    end
    return nil
end

local function parse_header(pathname, include_path)
    local lines = read_lines(pathname)
    local component = nil
    local i = 1
    while i <= #lines do
        local line = lines[i]
        local comp_args = line:match("CYBER_REFLECT_COMPONENT%s*%((.*)%)")
        if comp_args then
            local parts = split_csv(comp_args)
            component = {
                type = trim(parts[1] or ""),
                base = trim(parts[2] or ""),
                attrs = {},
                properties = {},
                functions = {},
                include = include_path,
            }
            local attr_parts = {}
            for p = 3, #parts do
                push(attr_parts, parts[p])
            end
            component.attrs = parse_attrs(table.concat(attr_parts, ","))
        end

        local prop_args = line:match("CYBER_PROPERTY%s*%((.*)%)")
        if component and prop_args then
            local decl_index = find_next_decl_line(lines, i)
            if decl_index then
                local cpp_type, name = parse_field_decl(lines[decl_index])
                if cpp_type and name then
                    push(component.properties, {
                        name = name,
                        cpp_type = cpp_type,
                        attrs = parse_attrs(prop_args),
                    })
                    i = decl_index
                end
            end
        end

        local func_args = line:match("CYBER_FUNCTION%s*%((.*)%)")
        if component and func_args then
            local decl_index = find_next_decl_line(lines, i)
            if decl_index then
                local fn, end_index = parse_function_decl(lines, decl_index)
                if fn then
                    fn.attrs = parse_attrs(func_args)
                    push(component.functions, fn)
                    i = end_index
                end
            end
        end

        i = i + 1
    end
    return component
end

local function emit_property_chain(lines, component, prop)
    local type_name = "Cyber::Component::" .. component.type
    push(lines, ("            .field<%s>(\"%s\", &%s::%s)"):format(
        type_name, escape_cpp_string(prop.name), type_name, prop.name))

    local attrs = prop.attrs or {}
    if attrs.Display then
        push(lines, ("                .display(\"%s\")"):format(escape_cpp_string(attrs.Display)))
    end
    if attrs.Serializable then
        push(lines, "                .serializable()")
    end
    if attrs.ReadOnly then
        push(lines, "                .readonly()")
    end
    if attrs.Asset then
        push(lines, ("                .asset(Cyber::Editor::PropertyAssetKind::%s)"):format(attrs.Asset))
    end
    if attrs.Color then
        push(lines, "                .as_color()")
    end
    if attrs.Euler then
        push(lines, "                .as_euler()")
    end
    if attrs.UniformScale then
        push(lines, "                .uniform_scale()")
    end
    if attrs.Speed then
        push(lines, ("                .speed(%sf)"):format(attrs.Speed))
    end
    if attrs.Min then
        push(lines, ("                .min(%sf)"):format(attrs.Min))
    end
    if attrs.Max then
        push(lines, ("                .max(%sf)"):format(attrs.Max))
    end
    if attrs.Range then
        local lo, hi = attrs.Range:match("^([^:]+):(.+)$")
        if lo and hi then
            push(lines, ("                .range(%sf, %sf)"):format(trim(lo), trim(hi)))
        end
    end
end

local function emit_function_chain(lines, fn)
    local attrs = fn.attrs or {}
    push(lines, ("            .function(\"%s\", \"%s\")"):format(
        escape_cpp_string(fn.name), escape_cpp_string(fn.signature)))
    if attrs.Display then
        push(lines, ("                .display(\"%s\")"):format(escape_cpp_string(attrs.Display)))
    end
    if attrs.ScriptCallable then
        push(lines, "                .script_callable()")
    end
    if fn.suffix and fn.suffix:find("const") then
        push(lines, "                .const_function()")
    end
end

local function build_generated_cpp(components)
    local lines = {}
    push(lines, "// Generated by tools/reflection/reflection_codegen.lua. Do not edit manually.")
    push(lines, "#include \"component_reflection.gen.h\"")
    push(lines, "#include \"editor/property_registry.h\"")
    for _, component in ipairs(components) do
        push(lines, ("#include \"%s\""):format(component.include))
    end
    push(lines, "")
    push(lines, "namespace Cyber::Generated")
    push(lines, "{")
    push(lines, "    void register_component_reflection()")
    push(lines, "    {")
    push(lines, "        auto& registry = Cyber::Editor::PropertyRegistry::get();")
    push(lines, "")

    for _, component in ipairs(components) do
        local type_name = "Cyber::Component::" .. component.type
        local attrs = component.attrs or {}
        push(lines, ("        registry.register_component(\"%s\")"):format(escape_cpp_string(component.type)))
        if attrs.Display then
            push(lines, ("            .display(\"%s\")"):format(escape_cpp_string(attrs.Display)))
        end
        if component.base and component.base ~= "" and component.base ~= "None" then
            push(lines, ("            .inherits(\"%s\")"):format(escape_cpp_string(component.base)))
        end
        if attrs.Abstract then
            push(lines, "            .abstract()")
        else
            push(lines, ("            .factory<%s>()"):format(type_name))
        end
        for _, prop in ipairs(component.properties) do
            emit_property_chain(lines, component, prop)
        end
        for _, fn in ipairs(component.functions) do
            emit_function_chain(lines, fn)
        end
        push(lines, "            ;")
        push(lines, "")
    end

    push(lines, "    }")
    push(lines, "}")
    push(lines, "")
    return table.concat(lines, "\n")
end

local function write_if_changed(pathname, content)
    local old = nil
    local f = io.open(pathname, "r")
    if f then
        old = f:read("*a")
        f:close()
    end
    if old == content then
        return
    end
    if io.writefile then
        io.writefile(pathname, content)
    else
        local out = assert(io.open(pathname, "w"))
        out:write(content)
        out:close()
    end
end

function M.generate(projectdir)
    local include_root = path.join(projectdir, "Engine/modules/runtime/include/component")
    local out_dir = path.join(projectdir, "Engine/modules/runtime/generated")
    os.mkdir(out_dir)

    local components = {}
    local headers = os.files(path.join(include_root, "*.h"))
    table.sort(headers)
    for _, header in ipairs(headers) do
        local filename = path.filename(header)
        local include_path = "component/" .. filename
        local component = parse_header(header, include_path)
        if component and component.type ~= "" then
            push(components, component)
        end
    end

    table.sort(components, function(a, b)
        if a.type == "Primitive" then return true end
        if b.type == "Primitive" then return false end
        return a.type < b.type
    end)

    local header = table.concat({
        "// Generated by tools/reflection/reflection_codegen.lua. Do not edit manually.",
        "#pragma once",
        "#include \"cyber_runtime.config.h\"",
        "",
        "namespace Cyber::Generated",
        "{",
        "    CYBER_RUNTIME_API void register_component_reflection();",
        "}",
        "",
    }, "\n")

    write_if_changed(path.join(out_dir, "component_reflection.gen.h"), header)
    write_if_changed(path.join(out_dir, "component_reflection.gen.cpp"), build_generated_cpp(components))
end

function generate(projectdir)
    M.generate(projectdir or os.projectdir())
end

function main(projectdir)
    M.generate(projectdir or os.projectdir())
end

return M
