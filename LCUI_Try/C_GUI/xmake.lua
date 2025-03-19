add_rules("mode.debug", "mode.release")

target("mylcuiapp")
    set_kind("binary")
    add_files("src/*.c")
    add_includedirs("src/include") -- Ensure this path is correct
    add_linkdirs("src/lib") -- Ensure this path is correct
    add_links("lcui")