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
- execute `./backup/bud-backup.sh [-f] [-d] -g /path/to/gnupg/directory -i /path/to/backup`
- the input path will be added to the current directory to create the backup directory: /media/bud/path/to/backup
- wait...

# bud restore

- plug bud
- copy the \*ring.gpg files somewhere (but not on bud): /path/to/gnupg/directory
- execute `/media/bud/backup/bud-restore.sh [-d] -g /path/to/gnupg/directory -i /media/bud/path/to/backup -o /path/where/to/restore/backup`
- wait...
