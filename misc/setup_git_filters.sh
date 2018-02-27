git config filter.nbstripout.clean '"python" "$(git rev-parse --show-toplevel)/misc/nbstripout.py"'
git config filter.nbstripout.smudge cat
git config filter.nbstripout.required true
