# PLY
PLY a script that monitors your clipboard and uses mpv to play playlists and videos in the background if any Youtube URL is found, and notify the user with the title of the currently playing video

# Install:
Clone the repo:

`git clone https://github.com/hakivvi/PLY.git`

Install dependencies

`cd PLY && make install-deps`

then compile ply-notify:

`gcc ply-notify.c $(pkg-config --libs --cflags dbus-1) -o ply-notify`

**Install PLY (auto-start after booting):**

`make install`

**OR Run PLY:**

`make run`

then:

`ply-start` to start PLY

`ply-stop` to stop PLY

# Uninstall:
`make uninstall`
# Usage:
run or install PLY then just copy URL of a youtube video / playlist.

PLY will take the URL and check if it is a valid Youtube URL, if so it will play it using mpv, and will send a notification that contains the name of the video, if the video reached its end it will send a notification telling you that the video is ended, and it will play the new URL that is on the clipboard (loop).

if the URL is a playlist it will send a notification every time mpv starts a new video (this is the main reason i created this script for), once it recieves a INT (^C) or TERM (kill) SIGNALL it will first stop mpv (and thus its subprocesses) do some cleanup then it will exit.

Added: Error checking, if mpv failed to start, or if a video in the playlist is a `[Deleted video]`

Added: Using our custom binary to send notification to the desktop using DBUS plain C library instead of `notify-send`.
