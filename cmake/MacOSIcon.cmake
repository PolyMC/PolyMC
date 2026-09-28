# Set up an executable target with a Liquid Glass icon.
# dir: The directory containing the .icns and Assets.car files, and the .icon directory.
# icon: The name (without extension) of the icns file.
# target: The target to apply this to.
function(MacOSIcon dir icon target)
    # Xcode wants the .icon directory, and will handle the rest
    if (CMAKE_GENERATOR MATCHES "Xcode")
        set(files ${dir}/${icon}.icon)
    else()
        set(icns ${dir}/${icon}.icns)
        set(assets ${dir}/Assets.car)

        set(files ${icns} ${assets})
    endif()

    set_source_files_properties(${files} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
        XCODE_FILE_ATTRIBUTES CodeSignOnCopy)

    target_sources(${target} PRIVATE ${files})

    set_target_properties(${target} PROPERTIES
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME ${icon}
        MACOSX_BUNDLE_ICON_FILE ${icon})
endfunction()
