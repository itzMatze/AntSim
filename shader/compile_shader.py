import sys
sys.dont_write_bytecode = True
import os, subprocess

WHITE = "\033[0m"
RED = "\033[91m"
GREEN = "\033[92m"

class CompileTask(object):
	def __init__(self, name, subprocess):
		self.name = name
		self.subprocess = subprocess

def output_task_result(task):
	stdout, stderr = task.subprocess.communicate()
	if task.subprocess.returncode != 0:
		if stdout:
			print(f"{RED}{task.name} compilation failed:{WHITE}\n{stdout}")
		if stderr:
			print(f"{RED}{task.name} compilation failed:{WHITE}\n{stderr}")
	else:
		print(f"{GREEN}{task.name} compiled successfully!{WHITE}")

if __name__ == '__main__':
	print("Compiling shaders")
	script_directory = os.path.dirname(os.path.realpath(__file__))
	os.chdir(script_directory)
	if not os.path.exists("bin/"):
		os.mkdir("bin/")
	dirs = [d for d in os.listdir(".") if not d.endswith('.glsl') and not d.endswith('.py') and os.path.isfile(d)]
	compile_tasks = []
	while len(dirs) > 0:
		cpu_cores = os.cpu_count() or 1
		while len(compile_tasks) > cpu_cores:
			for compile_task in compile_tasks:
				if compile_task.subprocess.poll() is not None:
					output_task_result(compile_task)
					compile_tasks.remove(compile_task)
					break
		d = dirs.pop()
		glslc_args = "--target-env=vulkan1.4 -O -o bin/{0}.spv {0}".format(d)
		if os.name == 'nt':
			compile_tasks.append(CompileTask(d, subprocess.Popen(f".\\..\\dependencies\\glslc.exe {glslc_args}", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)))
		else:
			compile_tasks.append(CompileTask(d, subprocess.Popen(f"glslc {glslc_args}", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)))

	for compile_task in compile_tasks:
		compile_task.subprocess.wait()
		output_task_result(compile_task)
	print("Done!")
