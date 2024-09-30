all : app/authback.cgi app/logout.cgi app/test.cgi

app/authback.cgi : src/authback.c
	gcc -o $@ $^ -Og -g  -lcurl -lsqlite3

app/test.cgi : src/test.c
	gcc -o $@ $^ -Og -g

app/logout.cgi : src/logout.c
	gcc -o $@ $^ -Og -g -lsqlite3

install : app/logout.cgi app/authback.cgi app/index.html
	test -f data/.privateappauth || echo "Error: must set a private key in privateappauth, in the form client_id=XXXXXXXXXXXXX&client_secret=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	sudo cp monotile.conf /etc/apache2/conf-available/
	sudo a2enconf monotile
	sudo service apache2 reload
	chmod 775 data
	touch data/log.txt
	chmod 666 data/log.txt

	mkdir -p data/database
	chmod 777 -R data/database
	echo "CREATE TABLE IF NOT EXISTS logins( key TEXT PRIMARY KEY, name TEXT NOT NULL, userurl TEXT, logintime DATE )" | sqlite3 data/database/monotile.db
	chmod 666 data/database/monotile.db

clean :
	rm -rf app/authback.cgi app/logout.cgi
