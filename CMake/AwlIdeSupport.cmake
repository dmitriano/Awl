if (MSVC)
    set(AWL_VISUALIZERS ${AWL_ROOT_DIR}/Ide/VisualStudio/Awl.natvis)

    if (EXISTS ${AWL_VISUALIZERS})
        target_sources(${PROJECT_NAME} PRIVATE ${AWL_VISUALIZERS})
    else()
        message(WARNING "MS Visual Studio visualizer file not found: ${AWL_VISUALIZERS}")
    endif()
endif()
