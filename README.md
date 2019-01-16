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
- create hash for all files without its corresponding hash file (MUST be relative path): `./backup/hash-create.sh ./path/to/backup`
- wait...

# bud restore

- plug bud
- go to bud's root: `cd /media/bud`
- check hash for all files: `./backup/hash-check.sh ./path/to/check`
- wait...
- copy the \*ring.gpg files somewhere (but not on bud): /path/to/gnupg/directory
- execute `/media/bud/backup/bud-restore.sh [-d] -g /path/to/gnupg/directory -i /media/bud/path/to/backup -o /path/where/to/restore/backup`
- wait...
- OR if duplicity is not available, execute a no-dependencies gpg on Windows: 
    `.\backup\gpg4win-x.x.x\bin\gpg.exe --homedir X:\path\to\gnupg\directory -d .\path\to\file\to\decrypt.tar.gpg > X:\path\to\output\decoded\file.tar`

# bud check

- check hash for all files: `./backup/hash-check.sh ./path/to/check`
- wait...
