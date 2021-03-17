install:
	sudo install --owner root --group root --mode 755 ply /usr/local/bin/
	sudo install --owner root --group root --mode 755 ply-notify /usr/local/bin/
	sudo install --owner root --group root --mode 755 cp /usr/local/bin/cp
	mkdir -p ~/.config/systemd/user/
	cp ply.service ~/.config/systemd/user/
	systemctl --user enable --now ply.service
run:
	sudo install --owner root --group root --mode 755 ply /usr/local/bin/
	sudo install --owner root --group root --mode 755 ply-notify /usr/local/bin/
	sudo install --owner root --group root --mode 755 cp /usr/local/bin/cp
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
	sudo apt-get update 
	sudo apt-get install youtube-dl mpv jq libdbus-1-dev libx11-dev -y
