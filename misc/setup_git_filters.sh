git config filter.stripoutput.clean "$(git rev-parse --show-toplevel)/misc/nbstripout.py"
git config filter.stripoutput.smudge cat
git config filter.stripoutput.required true
