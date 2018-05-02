#!/bin/bash
DIR='bm_sfi_v003'
DIR='bm_sfi_scaling002'
ssh -t artemy@santafe.santafe.edu "mkdir -p $DIR ; rsync -avz --delete habanero:langevosfi/benchmarks_py/$DIR ."
mkdir -p $DIR
rsync -avz --delete santafe.santafe.edu:$DIR .

