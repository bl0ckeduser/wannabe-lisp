make prefix.txt

#emcc -DJS_GUI `ls *.c | grep -v '^main\.c'` -O2 -s ASM_JS=0 --embed-file prefix.txt --shell-file gui/emcc-template.html -s ALLOW_MEMORY_GROWTH=1 -o gui/main.html -s EXPORTED_FUNCTIONS="['_handle_gui_line', '_do_setup']"
emcc -O2 -DJS_GUI -s ASM_JS=0 `ls *.c | grep -v '^main\.c'` --embed-file prefix.txt --shell-file gui/emcc-template.html -s ALLOW_MEMORY_GROWTH=1 -o gui/main.html -s EXPORTED_FUNCTIONS="['_handle_gui_line', '_do_setup', '_jsgui_error_handler']"

cp gui/main.html gui/final/code/chrome/content/
cp gui/frame.html gui/final/code/chrome/content/index.html
