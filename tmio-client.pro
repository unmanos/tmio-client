TEMPLATE = subdirs

SUBDIRS += \
    core \
    editor \
    cryptolib

# where to find the sub projects - give the folders
# lib2.subdir = src/lib2
# lib.subdir  = src/lib
# app.subdir  = src/app

# what subproject depends on others
editor.depends = core cryptolib

TRANSLATIONS = editor/editor_fr.ts  \
               editor/editor_de.ts
