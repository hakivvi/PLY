install:
	sudo install --owner root --group root --mode 755 ply /usr/local/bin/
	sudo install --owner root --group root --mode 755 ply-notify /usr/local/bin/
	mkdir -p ~/.config/systemd/user/
	cp ply.service ~/.config/systemd/user/
	systemctl --user enable --now ply.service
run:
	sudo install --owner root --group root --mode 755 ply /usr/local/bin/
	sudo install --owner root --group root --mode 755 ply-notify /usr/local/bin/
	mkdir -p ~/.config/systemd/user/
	cp ply.service ~/.config/systemd/user/
	echo "alias ply-start=\"systemctl --user start --now ply.service\"" >> ~/.bash_aliases
	echo "alias ply-stop=\"systemctl --user stop --now ply.service\"" >> ~/.bash_aliases
	. ~/.bash_aliases
uninstall:
	systemctl --user disable --now ply.service
	rm ~/.config/systemd/user/ply.service
	sudo rm /usr/local/bin/ply
	sudo rm /usr/local/bin/ply-notify	
install-deps:
	#if [ -f /etc/debian_version ] ; then sudo apt-get update ; sudo apt-get install xclip youtube-dl mpv jq libnotify-bin libdbus-1-dev -y ; fi # uncomment this line if you want to install notify-send also
	if [ -f /etc/debian_version ] ; then sudo apt-get update ; sudo apt-get install xclip youtube-dl mpv jq libdbus-1-dev -y ; fi
	if [ -f /etc/arch-release ] ; then sudo pacman -Syu ; sudo pacman -S libnotify-bin mpv youtube-dl xclip jq libdbus-1-dev; fi
