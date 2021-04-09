from platform import system


def host_platform():
	if system() == "Windows":
		return "win"
	if system() == "Darwin":
		return "osx"
	if system() == "Linux":
		return "linux"