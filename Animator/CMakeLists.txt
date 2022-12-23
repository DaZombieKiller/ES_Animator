project(Ani_ik)

set(SOURCE_FILES
    animator.rc
    Animator.cpp
    Audio.cpp
    Draws.cpp
    Editors.cpp
    IK2.cpp
    Ik3.cpp
    Io_subs.cpp
    Mouse.cpp
    RenderD3D.cpp
    Tools.cpp
    Track.cpp
    Utils.cpp
    ikd.cpp
    mathem.cpp
)

set(HEADER_FILES
    Animator.h
    resource.h
    ik.h
    mathem.h
)

set(RESOURCE_FILES
    Hand.cur
    appicon.ICO
    ico00001.ico
    ico_0x0.ico
    ico_ani.ico
    ico_exit.ico
    ico_fet.ico
    ico_flat.ico
    ico_hold.ico
    ico_ikmo.ico
    ico_in.ico
    ico_maxa.ico
    ico_mov.ico
    ico_ms.ico
    ico_out.ico
    ico_rol.ico
    ico_rs.ico
    ico_sc2s.ico
    ico_sc3s.ico
    ico_sets.ico
    ico_text.ico
    ico_wf.ico
    ico_zoom.ico
    icon2.ico
    ikrot.ico
    movmode1.cur
    movmode2.cur
    movmode3.cur
    square.cur
)

add_executable(Ani_ik WIN32 ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})
target_link_libraries(Ani_ik PRIVATE winmm comctl32 ddraw dsound)
source_group("Resource Files" FILES ${RESOURCE_FILES})
