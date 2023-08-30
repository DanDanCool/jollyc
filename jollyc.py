import jmake

workspace = jmake.Workspace("jollyc")
workspace.lang = "c17"

lib = jmake.Project("jollyc", jmake.Target.STATIC_LIBRARY)
files = jmake.glob("src", "*.h") + jmake.glob("src", "*.c")
files = [ file for file in files if file != "main.c" ]
lib.add(files)

host = jmake.Host()
if host.os == jmake.Platform.WIN32:
    lib.define("JOLLY_MSVC", 1)

debug = lib.filter("debug")
debug["debug"] = True

test = jmake.Project("test", jmake.Target.EXECUTABLE)
test.add("main.c")
test.depend(lib)

debug = test.filter("debug")
debug["debug"] = True

workspace.add(lib)
workspace.add(test)
jmake.generate(workspace)
