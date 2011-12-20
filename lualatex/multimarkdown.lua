
module('multimarkdown', package.seeall)

multimarkdown.module = {
    name          = "multimarkdown",
    version       =  0.1,
    date          = "2011/12/20",
    description   = "Input filter for Peg-MultiMarkdown blocks.",
    author        = "Marko Mahnič",
    copyright     = "Marko Mahnič",
    license       = "",
}

luatexbase.provides_module(multimarkdown.module)

require "multimarkdownlualib"
local inputblock = {}
local outputblock = {}

luatexbase.add_to_callback('find_read_file',
   function(id_number,asked_name)
      if string.match(asked_name, "^multimarkdowninput:") then
         return asked_name
      end
      -- TODO: how can we get the default behaviour for other files?
      -- For now we just check if it exists
      f = io.open(asked_name,"r")
      if f then
         f:close()
         return asked_name
      end
      return nil
   end,
  'multimarkdown.find_read_file'
)

luatexbase.add_to_callback('open_read_file',
  function(asked_name)
     --texio.write_nl("open_read_file " .. asked_name .. "\n")
     local tab = { }
     if string.match(asked_name, "^multimarkdowninput:") then
        tab.file = outputblock
        tab.i = 0
        tab.reader = function (t)
                        if t.i > table.maxn(t.file) then
                           return nil
                        end
                        local rv = t.file[t.i]
                        t.i = t.i + 1
                        return rv
                     end
     else
        --print(asked_name)
        tab.file = assert(io.open(asked_name))
        tab.reader = function (t)
                        local f = t.file
                        return f:read('*l')
                     end
     end
     return tab
  end,
  'multimarkdown.open_read_file'
)

function multimarkdown.addmkdline(line)
    -- TODO: split into head | \stopmarkdown | tail
    if string.match(line,"^[ 	]*\\stopmarkdown") then
        luatexbase.remove_from_callback('process_input_buffer', 'multimarkdown.addmkdline')
        local mkd = table.concat(inputblock, "\n")
        inputblock = {}
        --latex = [[It still works.
        --Now in multiple lines, too.]]

        local latex = multimarkdownlualib.tolatex(mkd)
        outputblock = string.explode(latex,"\n")
        return "\\input{multimarkdowninput:outputblock}\\directlua{multimarkdown.cleanup('')}"

        --[ A variant using temporary files instead of XXX_read_file callbacks]
        --local tmpf = os.tmpname() -- the file is created, but input needs one with .tex
        --local ftex = io.open(tmpf .. ".tex", "w")
        --ftex:write(latex)
        --ftex:close()
        --return "\\input{" .. tmpf .. ".tex}\\directlua{multimarkdown.cleanup('" .. tmpf .. "')}" -- good
    end
    table.insert(inputblock, line)
    return "" -- this removes the line from LuaTeX's input buffer
end

function multimarkdown.cleanup(tmpf)
    inputblock = {}
    outputblock = {}

    --[ A variant using temporary files instead of XXX_read_file callbacks]
    --os.remove(tmpf)
    --os.remove(tmpf .. ".tex")
end

function multimarkdown.parse_options(params)
    local t = {}
    
    -- TODO: OPTION: Output Format [latex/tex/context]
    --
    -- OPTION: Base Header Level
    -- multimarkdown generates these levels with latex output (1=part, 2=chapter, ...):
    local hlvls = { "part", "chapter",
        "section", "subsection", "subsubsection",
        "paragraph", "subparagraph", "subsubparagraph" }
    local bhl = string.match(params, "baseheaderlevel=([0-9a-z]+)")
    if bhl then
        local level = nil
        if string.match(bhl, "^[0-9]") then
            level = tonumber(bhl)
        else
            for i,v in ipairs(hlvls) do
                if v == bhl then
                    level = i
                    break
                end
            end
        end 
        if level and (level >= 1) and (level <= table.maxn(hlvls)) then
            t['baseheaderlevel'] = level
        else
            tex.error("Invalid Base Header Level: " .. bhl)
        end 
    end
    return t
end

function multimarkdown.startmarkdown(params)
    inputblock = {}
    opts = parse_options(params)
    if opts['baseheaderlevel'] then
        table.insert(inputblock, "Base Header Level: " .. opts['baseheaderlevel'])
    end
    luatexbase.add_to_callback("process_input_buffer", multimarkdown.addmkdline, 'multimarkdown.addmkdline')
end

function multimarkdown.includemarkdown(filename, params)
    inputblock = {}
    opts = parse_options(params)
    if opts['baseheaderlevel'] then
        table.insert(inputblock, "Base Header Level: " .. opts['baseheaderlevel'])
    end

    f = assert(io.open(filename))
    for s in f:lines() do
        -- TODO: remove (some) headers, otherwise it could be treated as a standalone document
        table.insert(inputblock, s)
    end
    f:close()

    local mkd = table.concat(inputblock, "\n")
    inputblock = {}
    local latex = multimarkdownlualib.tolatex(mkd)
    outputblock = string.explode(latex,"\n")
    tex.print("\\input{multimarkdowninput:outputblock}\\directlua{multimarkdown.cleanup('')}")
end
