all : app/authback.cgi app/logout.cgi

app/authback.cgi : src/authback.c
	gcc -o $@ $^ -Os -s -lcurl -lsqlite3

app/logout.cgi : src/logout.c
	gcc -o $@ $^ -Os -s -lsqlite3

install : app/logout.cgi app/authback.cgi app/index.html
	sudo mkdir -p /var/www/html/monotile/
	sudo cp app/* /var/www/html/monotile/
	sudo chown -R www-data:www-data /var/www/html/monotile
	sudo cp monotile.conf /etc/apache2/conf-available/
	sudo a2enconf monotile
	sudo service apache2 reload

createdb :
	echo "CREATE TABLE IF NOT EXISTS logins( key TEXT PRIMARY KEY, name TEXT NOT NULL, logintime DATE )" | sqlite3 monotile.db

clean :
	rm -rf app/authback.cgi app/logout.cgi
