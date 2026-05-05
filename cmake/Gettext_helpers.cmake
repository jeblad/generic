find_program(GETTEXT_XGETTEXT_COMMAND xgettext)
find_program(GETTEXT_MSGFMT_COMMAND msgfmt)
find_program(GETTEXT_MSGINIT_COMMAND msginit)
find_program(GETTEXT_MSGMERGE_COMMAND msgmerge)

function(configure_gettext)
    # Ensure the utility programs are available
    if(NOT GETTEXT_XGETTEXT_COMMAND OR NOT GETTEXT_MSGFMT_COMMAND
            OR NOT GETTEXT_MSGMERGE_COMMAND OR NOT GETTEXT_MSGINIT_COMMAND)
        message(FATAL_ERROR "Could not find required programs!")
        message(WARNING "Gettext utilities not found. Translations will not be updated.")
        return()
    endif()

    set(options ALL)
    set(one_value_args 
        DOMAIN INSTALL_DESTINATION INSTALL_COMPONENT TARGET_NAME
        POTFILE_DESTINATION POFILE_DESTINATION GMOFILE_DESTINATION
        BUILD_DESTINATION
    )
    set(multi_args SOURCES LANGUAGES XGETTEXT_ARGS MSGFMT_ARGS MSGINIT_ARGS MSGMERGE_ARGS)
    cmake_parse_arguments(GETTEXT
        "${options}" "${one_value_args}" "${multi_args}" ${ARGV})

    if(NOT GETTEXT_DOMAIN)
        message(FATAL_ERROR "Must supply a DOMAIN!")
    elseif(NOT GETTEXT_POTFILE_DESTINATION)
        message(FATAL_ERROR "Must supply a POTFILE_DESTINATION!")
    elseif(NOT GETTEXT_LANGUAGES)
        message(FATAL_ERROR "No LANGUAGES specified!")
    elseif(NOT GETTEXT_TARGET_NAME)
        message(FATAL_ERROR "No TARGET_NAME specified!")
    elseif(NOT GETTEXT_SOURCES)
        message(FATAL_ERROR "No SOURCES supplied!")
    elseif(GETTEXT_INSTALL_COMPONENT AND NOT GETTEXT_INSTALL_DESTINATION)
        message(FATAL_ERROR "INSTALL_COMPONENT relies on INSTALL_DESTINATION")
    endif()

    if(NOT GETTEXT_POFILE_DESTINATION)
        set(GETTEXT_POFILE_DESTINATION "${GETTEXT_POTFILE_DESTINATION}")
    endif()
    if(NOT GETTEXT_GMOFILE_DESTINATION)
        set(GETTEXT_GMOFILE_DESTINATION "${GETTEXT_POFILE_DESTINATION}")
    endif()

    # Make input directories absolute
    foreach(dir POTFILE POFILE GMOFILE)
        if(NOT IS_ABSOLUTE "${GETTEXT_${dir}_DESTINATION}")
            set(GETTEXT_${dir}_DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/${GETTEXT_${dir}_DESTINATION}")
        endif()
        file(TO_CMAKE_PATH "${GETTEXT_${dir}_DESTINATION}" GETTEXT_${dir}_DESTINATION)
    endforeach()
    
    # Safety check: Ensure we are not writing outside the project source directory
    file(RELATIVE_PATH _rel_pot "${CMAKE_CURRENT_SOURCE_DIR}" "${GETTEXT_POTFILE_DESTINATION}")
    if(_rel_pot MATCHES "^\\.\\.")
        message(FATAL_ERROR "POTFILE_DESTINATION (${GETTEXT_POTFILE_DESTINATION}) is outside project source!")
    endif()

    # Create needed directories
    file(MAKE_DIRECTORY "${GETTEXT_POTFILE_DESTINATION}")
    file(MAKE_DIRECTORY "${GETTEXT_POFILE_DESTINATION}")
    file(MAKE_DIRECTORY "${GETTEXT_GMOFILE_DESTINATION}")

    set(POT_PATH "${GETTEXT_POTFILE_DESTINATION}/${GETTEXT_DOMAIN}.pot")

    if(GETTEXT_ALL)
        add_custom_target("${GETTEXT_TARGET_NAME}" ALL DEPENDS "${POT_PATH}")
    else()
        add_custom_target("${GETTEXT_TARGET_NAME}" DEPENDS "${POT_PATH}")
    endif()

    # Generate the .pot file
    add_custom_command(
        OUTPUT "${POT_PATH}"
        COMMAND "${GETTEXT_XGETTEXT_COMMAND}" ${GETTEXT_XGETTEXT_ARGS}
            ${GETTEXT_SOURCES}
            "--output=${POT_PATH}"
        DEPENDS ${GETTEXT_SOURCES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Extracting translatable strings to ${GETTEXT_DOMAIN}.pot"
        VERBATIM)

    foreach(lang IN LISTS GETTEXT_LANGUAGES)
        # Create needed directories
        file(MAKE_DIRECTORY "${GETTEXT_POFILE_DESTINATION}/${lang}")
        file(MAKE_DIRECTORY "${GETTEXT_GMOFILE_DESTINATION}/${lang}")
        
        set(PO_PATH "${GETTEXT_POFILE_DESTINATION}/${lang}/${GETTEXT_DOMAIN}.po")
        set(GMO_PATH "${GETTEXT_GMOFILE_DESTINATION}/${lang}/${GETTEXT_DOMAIN}.gmo")

        # Initialize .po file if it doesn't exist
        if(NOT EXISTS "${PO_PATH}")
            message(STATUS "Creating initial .po file for ${lang}")
            if(NOT EXISTS "${POT_PATH}")
                 file(WRITE "${POT_PATH}" "") # Create dummy if it doesn't exist yet for msginit
            endif()
            execute_process(
                COMMAND "${GETTEXT_MSGINIT_COMMAND}" ${GETTEXT_MSGINIT_ARGS}
                    "--input=${POT_PATH}"
                    "--no-translator"
                    "--output-file=${PO_PATH}"
                    "--locale=${lang}"
                    "--no-translator"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
        endif()

        # Update .po file from .pot
        add_custom_command(
            OUTPUT "${PO_PATH}"
            COMMAND "${GETTEXT_MSGMERGE_COMMAND}" ${GETTEXT_MSGMERGE_ARGS}
                "--update" "--backup=none"
                "${PO_PATH}"
                "${POT_PATH}"
            DEPENDS "${POT_PATH}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Merging changes into ${lang}.po"
            VERBATIM)

        # Compile .po to .gmo
        add_custom_command(
            OUTPUT "${GMO_PATH}"
            COMMAND "${GETTEXT_MSGFMT_COMMAND}" ${GETTEXT_MSGFMT_ARGS}
                "${PO_PATH}"
                "--output-file=${GMO_PATH}"
            DEPENDS "${PO_PATH}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Compiling ${lang}.po to ${lang}.gmo"
            VERBATIM)

        add_custom_target("${GETTEXT_TARGET_NAME}-${lang}" DEPENDS "${GMO_PATH}")
        add_dependencies("${GETTEXT_TARGET_NAME}" "${GETTEXT_TARGET_NAME}-${lang}")

        # Installation
        if(GETTEXT_INSTALL_DESTINATION)
            install(FILES "${GMO_PATH}"
                DESTINATION "${GETTEXT_INSTALL_DESTINATION}/${lang}/LC_MESSAGES"
                RENAME "${GETTEXT_DOMAIN}.mo"
                COMPONENT Translations)
        endif()

        if(GETTEXT_BUILD_DESTINATION)
            add_custom_command(
                TARGET "${GETTEXT_TARGET_NAME}-${lang}" PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    "${GMO_PATH}"
                    "${GETTEXT_BUILD_DESTINATION}/${lang}/LC_MESSAGES/${GETTEXT_DOMAIN}.mo"
            )
        endif()
    endforeach() # lang IN LISTS GETTEXT_LANGUAGES
endfunction()
