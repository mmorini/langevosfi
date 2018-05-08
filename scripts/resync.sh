#!/bin/bash
DIR=ldata
ssh -t artemy@santafe.santafe.edu "ssh -t artemy@habanero 'cd langevosfi; ./resync_langevo.sh ' ; mkdir -p $DIR ; rsync -avz --delete habanero:$DIR ."
mkdir -p $DIR
rsync -avz --delete santafe.santafe.edu:$DIR .
mv ldata/* benchmarks_py/
