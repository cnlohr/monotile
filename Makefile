all : app/authback.cgi app/logout.cgi

app/authback.cgi : src/authback.c
	gcc -o $@ $^ -Os -s -lcurl -lsqlite3

app/logout.cgi : src/logout.c
	gcc -o $@ $^ -Os -s -lsqlite3

install : app/logout.cgi app/authback.cgi
	sudo mkdir /usr/local/cgi-bin/monotile/
	sudo cp $^ /usr/local/cgi-bin/monotile/
	sudo chown -R apache:apache /usr/local/cgi-bin/monotile
	sudo cp monotile.conf /etc/apache2/conf-available/
	sudo a2enconf monotile
	sudo service apache2 reload

createdb :
	echo "CREATE TABLE IF NOT EXISTS logins( key TEXT PRIMARY KEY, name TEXT NOT NULL, logintime DATE )" | sqlite3 monotile.db

clean :
	rm -rf app/authback.cgi app/logout.cgi
