# backup

Scripts to backup personal data to an external disk.

# bud (BackUp Disk) prepare

- plug bud
- go to bud's root: `cd /media/bud`
- if backup directory doesn't exist: `git clone https://github.com/m-ll/backup.git`
- OR `git pull backup`

# bud backup

- copy the \*ring.gpg files somewhere (but not on bud): /path/to/gnupg/directory
- plug bud
- go to bud's root: `cd /media/bud`
- execute `./backup/bud-backup.sh [-f] ... -g /path/to/gnupg/directory -i /path/to/backup`
- the input path will be added to the current directory to create the backup directory: /media/bud//path/to/backup
- wait...
- create hash (if not already exists) for all files (MUST be relative path): `./backup/hash-create.sh ./relative/path/to/hash`
- wait...
- create eec (if not already exists) for all files: `./backup/ecc.py create -i ./path/to/ecc`
- wait...

# bud restore

- plug bud
- go to bud's root: `cd /media/bud`
- check hash for all files: `./backup/hash-check.sh ./path/to/check`
- wait...
- check eec for all files: `./backup/eec.py check -i ./path/to/check`
- wait...
- copy the \*ring.gpg files somewhere (but not on bud): /path/to/gnupg/directory
- execute `./backup/bud-restore.sh [-d] -g /path/to/gnupg/directory -i /media/bud/path/to/backup -o /path/where/to/restore/backup`
- wait...
- OR if duplicity is not available, execute a no-dependencies gpg on Windows: 
    `.\backup\gpg4win-x.x.x\bin\gpg.exe --homedir X:\path\to\gnupg\directory -d .\path\to\file\to\decrypt.tar.gpg > X:\path\to\output\decoded\file.tar`

# bud check

- check hash for all files: `./backup/hash-check.sh ./path/to/check`
- wait...
- eec for all files: `./backup/ecc.py {check|fix} -i ./path/or/file/to/check`
- wait...

# examples

## backup
- `cd /cygdrive/f/` # root of bud
- `./backup/bud-backup.sh -f -g /cygdrive/d/gnupg/ -i /cygdrive/c/backup/portable/`
- `./backup/hash-create.sh ./cygdrive/c/backup/portable/`
- `./backup/ecc.py create -i ./cygdrive/c/backup/portable/`

## restore
- `cd /cygdrive/f/` # root of bud
- `./backup/hash-check.sh ./cygdrive/c/backup/portable/`
- `./backup/ecc.py check -i ./cygdrive/c/backup/portable/`
- `./backup/bud-restore.sh -g /cygdrive/d/gnupg/ -i ./cygdrive/c/backup/portable/ -o /cygdrive/c/restored/portable/`
