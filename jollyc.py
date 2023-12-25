import jmake

jmake.setupenv()

workspace = jmake.Workspace("jollyc")
workspace.lang = "c17"

lib = jmake.Project("jollyc", jmake.Target.STATIC_LIBRARY)
files = jmake.glob("src", "*.h") + jmake.glob("src", "*.c")
lib.add(files)

lib.export(includes=jmake.fullpath("src"))

libdebug = lib.filter("debug")
libdebug["debug"] = True

test = jmake.Project("test", jmake.Target.EXECUTABLE)
test.add("test/main.c")
test.include(jmake.fullpath("src"))
test.depend(lib)

testdebug = test.filter("debug")
testdebug["debug"] = True

host = jmake.Env()
if host.os == jmake.Platform.WIN32:
    lib.define("JOLLY_WIN32", 1)
    lib.define("WIN32_LEAN_AND_MEAN", 1)
    lib.compile("/experimental:c11atomics")

workspace.add(lib)
workspace.add(test)
jmake.generate(workspace)
