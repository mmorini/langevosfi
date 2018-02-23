import os
import subprocess

git_strings = [];
git_strings.append("@(#)config.py: $Id$")

def get_git_id():
    def is_git_directory(path = '.'):
        return subprocess.call(['git', '-C', path, 'status'], stderr=subprocess.STDOUT, stdout = open(os.devnull, 'w')) == 0    

    def get_repo_sha(repo = '.'):
        sha = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo).decode('ascii').strip()
        return sha

    if not is_git_directory('.'):
        return 'NO_GIT_REPO_FOUND'
    else:
        return 'commit ' + get_repo_sha()

