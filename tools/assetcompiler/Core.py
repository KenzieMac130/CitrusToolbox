from . import Build

def options(ctx):
	ctx.add_option('--final', default=False, action='store', help='is production build')
	ctx.add_option('--libs', default='', action='store', help='path to platform libs')
	ctx.add_option('--build', default='', action='store', help='path to build dir')
	ctx.add_option('--platform', default='shared', action='store', help='target platform')

def configure(ctx):
	outPathList = [ctx.options.libs,ctx.options.build+'/output/',ctx.options.build+'/output/Release',ctx.options.build+'/output/Debug',ctx.options.build]
	try:
		ctx.find_program("compressonatorcli", path_list=ctx.options.libs+ '/Compressonator/', var='Compressonator')
	except:
		raise ValueError("Could not find Compressonator!")
	try:
		ctx.find_program("CitrusShader", path_list=outPathList, var='CitrusShader')
	except:
		ctx.to_log("Could not find CitrusShader! Please build tools and reconfigure")
	try:
		ctx.find_program("CitrusModel", path_list=outPathList, var='CitrusModel')
	except:
		ctx.to_log("Could not find CitrusModel! Please build tools and reconfigure")
	ctx.env.platform = ctx.options.platform
	ctx.env.final = ctx.options.final

def build(ctx):
	print('Building: ' + ctx.path.abspath())
	ctx.add_pre_fun(Build.build_step_checkin_all_assets)
	Build.build_step_compile(ctx)
	ctx.add_post_fun(Build.build_step_generate_mapping)