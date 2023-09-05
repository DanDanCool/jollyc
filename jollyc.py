import jmake

jmake.setupenv()

workspace = jmake.Workspace("jollyc")
workspace.lang = "c17"

lib = jmake.Project("jollyc", jmake.Target.STATIC_LIBRARY)
files = jmake.glob("src", "*.h") + jmake.glob("src", "*.c")
lib.add(files)

lib.includes.extend(jmake.fullpath("src"))

host = jmake.Host()
if host.os == jmake.Platform.WIN32:
    lib.define("JOLLY_MSVC", 1)

debug = lib.filter("debug")
debug["debug"] = True

test = jmake.Project("test", jmake.Target.EXECUTABLE)
test.add("test/main.c")
test.include(jmake.fullpath("src"))
test.depend(lib)

debug = test.filter("debug")
debug["debug"] = True

workspace.add(lib)
workspace.add(test)
jmake.generate(workspace)
