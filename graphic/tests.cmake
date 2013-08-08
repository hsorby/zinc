
SET(CURRENT_TEST graphic)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
    ${CURRENT_TEST}/contours.cpp
    ${CURRENT_TEST}/convert_to_finite_elements.cpp
    ${CURRENT_TEST}/graphic.cpp
    ${CURRENT_TEST}/graphics_filter.cpp
    ${CURRENT_TEST}/font.cpp
    ${CURRENT_TEST}/scene.cpp
    ${CURRENT_TEST}/scene_picker.cpp
    ${CURRENT_TEST}/scene_viewer.cpp
    ${CURRENT_TEST}/streamlines.cpp
    ${CURRENT_TEST}/tessellation.cpp
    )

