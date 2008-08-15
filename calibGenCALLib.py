# $Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['calibGenCAL'])
    env.Tool('addLibrary', library = env['rootLibs'])
    env.Tool('addLibrary', library = env['rootGuiLibs'])
    env.Tool('addLibrary', library = env['minuitLibs'])
#If the Minuit library above causes any problems, try replacing the previous statement with env.Tool('minuitLibs') because it contains a Minuit2 library.


def exists(env):
    return 1;
