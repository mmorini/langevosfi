#!/bin/bash
DIR=ldata
ssh -t santafe.santafe.edu "ssh -t habanero 'cd langevosfi/scripts; ./cluster_rsync.sh ' ; mkdir -p $DIR ; rsync -avz --delete habanero:$DIR ."
mkdir -p $DIR
rsync -avz --delete santafe.santafe.edu:$DIR/* ../benchmarks_py/
