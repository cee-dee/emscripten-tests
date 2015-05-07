Create glue-code:
python tools/webidl_binder.py test.idl glue

Compile:
emcc test.cpp my_glue_wrapper.cpp --post-js glue.js -o test.js -Oz

Open test.html in browser (e.g. Firefox)
