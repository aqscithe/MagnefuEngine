function add_common_defines()
    filter "system:macosx"
        defines {
            "MF_PLATFORM_MACOS",
            "MF_ARCH_ARM"
        }
    
    filter "system::windows"
end
