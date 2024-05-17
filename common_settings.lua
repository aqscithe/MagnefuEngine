function add_common_defines()
    filter "system:macosx"
        defines {
            "MF_PLATFORM_MACOS",
            "MF_ARCH_ARM"
        }
    
    filter "system::windows"
end

function macosx_build_options()
    buildoptions {
        "-fmodules",  -- Example: Enabling Clang modules
        "-fcxx-modules",
        "-fprofile-arcs", 
        "-ftest-coverage",
        "-Wall",        -- Enables all the warnings about constructions
        "-Wextra",      -- Enables some extra warning flags not enabled by -Wall
        --"-Werror",      -- Make all warnings into errors.
        "-Wshadow",     -- Warn the user if a variable declaration shadows one from a parent context.
        "-Wnon-virtual-dtor", -- Warn when a class with a virtual function has a non-virtual destructor.
        "-Wold-style-cast", -- Warn for c-style casts.
        "-Wunused",     -- Warn on anything being unused.
        "-Woverloaded-virtual", -- Warn if you overload (not override) a virtual function.
        "-Wpedantic",   -- (or -Wpendantic) Warn if non-standard C++ is used.
        "-Wconversion", -- Warn on type conversions that may lose data.
        "-Wsign-conversion", -- Warn on sign conversions.
        "-Wmisleading-indentation", -- Warn if indentation implies blocks where blocks do not exist.
        "-Wduplicated-cond", -- Warn if if / else chain has duplicated conditions.
        "-Wduplicated-branches", -- Warn if if / else branches have duplicated code.
        "-Wlogical-op", -- Warn about suspicious uses of logical operators in expressions.
        "-Wnull-dereference", -- Warn if a null dereference is detected.
        "-Wdouble-promotion", -- Warn on implicit conversion from float to double.
        "-Wformat=2"   -- Warn on security issues around functions that format output (like printf).
    }



end
