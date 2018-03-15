import os
import subprocess

# Returns short git commit id
# To see the state of the code for that commit, go to https://github.com/mmorini/langevosfi/commit/{id}
def get_git_id():
    def is_git_directory(path = '.'):
        return subprocess.call(['git', '-C', path, 'status'], stderr=subprocess.STDOUT, stdout = open(os.devnull, 'w')) == 0

    def get_repo_sha(repo = '.'):
        sha = subprocess.check_output(['git', 'rev-parse', '--short=4', 'HEAD'], cwd=repo).decode('ascii').strip()
        return sha

    if not is_git_directory('.'):
        return 'NO_GIT_REPO_FOUND'
    else:
        return 'commit ' + get_repo_sha()
