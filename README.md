# Backup

Scripts to backup personal data to an external disk.

Everything should be use under Linux or WSL (=Windows Sub Linux)
(even if cygwin should be compatible)

# Requirements (for python scripts)

- Python >= 3.6
- `pip3 install colorama`
- `pip3 install unireedsolomon` (to have the cython version)

# Bud (BackUp Disk) prepare

- plug bud
- go to bud's root: `cd /media/bud`
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
- check eec for all files: `./backup/ecc.py check -i ./path/to/ecc` (or see the [no-dep README](./no-dep/README.md))
- wait...
- copy the \*ring.gpg files somewhere (but not on bud): /path/to/gnupg/directory
- execute `./backup/bud-restore.sh [-d] -g /path/to/gnupg/directory -i ./path/of/backup -o /path/where/to/restore/backup` (or see the [no-dep README](./no-dep/README.md))
- wait...

# Bud check

- check hash for all files: `./backup/hash-check.sh ./path/to/check`
- wait...
- eec for all files: `./backup/ecc.py {check|fix} -i ./path/to/ecc` (or see the [no-dep README](./no-dep/README.md))
- wait...

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
./backup/ecc.py check -i ./mnt/c/backup/portable/
./backup/bud-restore.sh -g /mnt/d/gnupg/ -i ./mnt/c/backup/portable/ -o /mnt/c/restored/portable/
```
