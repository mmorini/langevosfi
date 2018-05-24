cd ../..

# Move code out to other servers
rsync -avz --delete langevosfi serrano:
rsync -avz --delete langevosfi pequin:
rsync -avz --delete langevosfi jalapeno:

# Bring data in from other servers
rsync -avz serrano:ldata .
rsync -avz pequin:ldata .
rsync -avz jalapeno:ldata .

