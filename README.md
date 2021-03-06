# Backup

Scripts to backup personal data to an external disk.

Everything should be use under Linux or WSL (=Windows Sub Linux)
(even if cygwin should be compatible)

# Requirements (for python scripts)

- Python >= 3.6
- `pip3 install colorama`

# TODO

- Create new file inside a tmp directory, then move it in its right directory (if the execution is stopped during a hash or ecc, no need to find and remove the not finish one)
- maybe improve python ecc script to enable multi entries with the -i argument (certainly with nargs=+)

# Bud (BackUp Disk) prepare

- plug bud
- go to bud's root: `cd /media/bud` (or `cd /mnt/f` on WSL)
- if backup directory doesn't exist: `git clone https://github.com/m-ll/backup.git`
- OR `git pull backup`

# Bud backup

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

# Bud restore

- plug bud
- go to bud's root: `cd /media/bud`
- check hash for all files: `./backup/hash-check.sh ./path/to/check`
- wait...
- check eec for all files: `./backup/ecc.py {check-size|fix-and-compare} -i ./path/to/ecc` (or see the [no-dep README](./no-dep/README.md))
- wait...
- copy the \*ring.gpg files somewhere (but not on bud): /path/to/gnupg/directory
- execute `./backup/bud-restore.sh [-d] -g /path/to/gnupg/directory -i ./path/of/backup -o /path/where/to/restore/backup` (or see the [no-dep README](./no-dep/README.md))
- wait...

# Bud check

- check hash for all files: `./backup/hash-check.sh ./path/to/check`
- wait...
- eec for all files: `./backup/ecc.py {check-size|fix|fix-and-compare} -i ./path/to/ecc` (or see the [no-dep README](./no-dep/README.md))
- wait...

# Bud shake

This will copy/paste files at the same place to move the files to another sectors/blocks.
This should be done every 1/2/3(/?) years.

- plug bud
- go to bud's root: `cd /media/bud`
- execute `./backup/shake.sh ./path/to/shake`
- (and maybe make a hash check)

# Examples

## Backup
```shell
cd /mnt/f/ # root of bud
./backup/bud-backup.sh -f -g /mnt/d/gnupg/ -i /mnt/c/backup/portable/
./backup/hash-create.sh ./mnt/c/backup/portable/
./backup/ecc.py create -i ./mnt/c/backup/portable/
```

## Restore
```shell
cd /mnt/f/ # root of bud
./backup/hash-check.sh ./mnt/c/backup/portable/
./backup/ecc.py check-size -i ./mnt/c/backup/portable/ && ./backup/ecc.py check -i ./mnt/c/backup/portable/
./backup/bud-restore.sh -g /mnt/d/gnupg/ -i ./mnt/c/backup/portable/ -o /mnt/c/restored/portable/
```
