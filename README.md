# backup

Scripts to backup some personal data on an external disk.

The script must be at the root of the disk where the backup will be made.
And the path to backup will be added to the current (root) directory.

(On windows, use cygwin)

# restore

Go to the root of the disk where the backup was made.

duplicity [--dry-run] --progress --gpg-options="--homedir GNUPG_PATH" "file://INPUT_PATH" "OUTPUT_PATH"
GNUPG_PATH: path where the *ring.gpp files are
INPUT_PATH: backup to restore
OUTPUT_PATH: path where to restore
example:
duplicity [--dry-run] --progress --gpg-options="--homedir ./gnupg" "file://./cygdrive/d/Users/Mike" "/cygdrive/d/Users/Mike"
